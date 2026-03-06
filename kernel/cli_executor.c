#include "cli_executor.h"

#include <stddef.h>

#include "cap.h"
#include "console.h"
#include "heap.h"
#include "idt.h"
#include "job.h"

static int streq(const char* a, const char* b) {
  while (*a && *b) {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static void write_u64(uint64_t value) {
  char buf[21];
  size_t i = 0;

  if (value == 0) {
    console_write("0");
    return;
  }

  while (value > 0 && i < sizeof(buf)) {
    buf[i++] = (char)('0' + (value % 10));
    value /= 10;
  }

  while (i > 0) {
    console_write_char(buf[--i]);
  }
}

static void write_hex_u64(uint64_t value) {
  static const char* digits = "0123456789ABCDEF";

  console_write("0x");
  for (int shift = 60; shift >= 0; shift -= 4) {
    console_write_char(digits[(value >> shift) & 0xF]);
  }
}

static void write_rights(uint32_t rights) {
  if (rights == 0u) {
    console_write("none");
    return;
  }

  int wrote = 0;
  if ((rights & CAP_R_READ) != 0u) {
    console_write("read");
    wrote = 1;
  }
  if ((rights & CAP_R_WRITE) != 0u) {
    if (wrote) {
      console_write("|");
    }
    console_write("write");
    wrote = 1;
  }
  if ((rights & CAP_R_EXEC) != 0u) {
    if (wrote) {
      console_write("|");
    }
    console_write("exec");
  }
}

static uint32_t op_to_rights(const char* operation) {
  if (operation == 0) {
    return 0u;
  }

  if (operation[0] == 'r' && operation[1] == 'e' && operation[2] == 'a' && operation[3] == 'd' && operation[4] == '\0') {
    return CAP_R_READ;
  }

  if (operation[0] == 'w' && operation[1] == 'r' && operation[2] == 'i' && operation[3] == 't' && operation[4] == 'e' && operation[5] == '\0') {
    return CAP_R_WRITE;
  }

  if (operation[0] == 'e' && operation[1] == 'x' && operation[2] == 'e' && operation[3] == 'c' && operation[4] == '\0') {
    return CAP_R_EXEC;
  }

  return 0u;
}

static void print_help_topic(const char* topic) {
  if (topic == 0 || streq(topic, "help")) {
    console_write("finde-os command map\n");
    console_write("  Schnellstart: welcome\n");
    console_write("  Hilfe: help <command> oder <command> --help\n");
    console_write("  Verfuegbare Commands:\n");
    console_write("    help                 Hilfe-Index und Details\n");
    console_write("    welcome              Onboarding mit 7 Schritten\n");
    console_write("    status               Aktiven Isolationsmodus zeigen\n");
    console_write("    ticks                Timer-Ticks seit Boot anzeigen\n");
    console_write("    malloc               Heap-Test (nur sandbox)\n");
    console_write("    cap                  Capabilities listen/pruefen/erklaeren\n");
    console_write("    job                  Jobs starten und verwalten\n");
    console_write("    hub|home             TUI-Hub mit Panels\n");
    console_write("  Hub-Shortcuts (vom Prompt): j=jobs, l=logs, r=retry, q=quit\n");
    console_write("  Beispiele:\n");
    console_write("    help job\n");
    console_write("    job --help\n");
    console_write("    hub\n");
    console_write("  Exit-Codes: 0 ok, 2 unbekanntes Thema\n");
    return;
  }

  if (streq(topic, "status")) {
    console_write("status\n");
    console_write("  Zweck: Zeigt den aktiven Isolationsmodus.\n");
    console_write("  Syntax: status\n");
    console_write("  Beispiele:\n");
    console_write("    status\n");
    console_write("    help status\n");
    console_write("  Exit-Codes: 0 ok\n");
    return;
  }

  if (streq(topic, "ticks")) {
    console_write("ticks\n");
    console_write("  Zweck: Zeigt aktuelle Timer-Ticks seit Boot.\n");
    console_write("  Syntax: ticks\n");
    console_write("  Beispiele:\n");
    console_write("    ticks\n");
    console_write("    ticks --help\n");
    console_write("  Exit-Codes: 0 ok\n");
    return;
  }

  if (streq(topic, "malloc")) {
    console_write("malloc\n");
    console_write("  Zweck: Testet Heap-Allokation und zeigt Adressen.\n");
    console_write("  Syntax: malloc\n");
    console_write("  Beispiele:\n");
    console_write("    malloc\n");
    console_write("    malloc --help\n");
    console_write("  Exit-Codes: 0 ok, 3 in microvm verweigert\n");
    return;
  }

  if (streq(topic, "cap")) {
    console_write("cap\n");
    console_write("  Zweck: Capability-Status anzeigen und pruefen.\n");
    console_write("  Syntax: cap list|show <id>|check <read|write|exec>|explain <profile>\n");
    console_write("  Beispiele:\n");
    console_write("    cap list\n");
    console_write("    cap show 5\n");
    console_write("    cap check write\n");
    console_write("    cap explain isolated\n");
    console_write("  Exit-Codes: 0 ok, 2 Syntax, 3 Zugriff verweigert\n");
    return;
  }

  if (streq(topic, "job")) {
    console_write("job\n");
    console_write("  Zweck: Einfache Hintergrund-Jobs verwalten.\n");
    console_write("  Syntax: job start <cmd> [--profile=default|isolated]|list|status <id>|logs <id> [--follow]|stop <id>\n");
    console_write("  Beispiele:\n");
    console_write("    job start worker\n");
    console_write("    job list\n");
    console_write("    job status 1\n");
    console_write("    job logs 1 --follow\n");
    console_write("    job stop 1\n");
    console_write("  Exit-Codes: 0 ok, 1 Laufzeitfehler, 2 Syntax\n");
    return;
  }

  if (streq(topic, "hub") || streq(topic, "home")) {
    console_write("hub|home\n");
    console_write("  Zweck: Keyboard-navigierbares TUI Hub im Terminal.\n");
    console_write("  Syntax: hub [jobs|commands|errors|logs|status|retry|quit]\n");
    console_write("  Beispiele:\n");
    console_write("    hub\n");
    console_write("    hub jobs\n");
    console_write("    hub logs\n");
    console_write("  Exit-Codes: 0 ok, 2 Syntax\n");
    return;
  }

  if (streq(topic, "welcome") || streq(topic, "onboarding")) {
    console_write("welcome|onboarding\n");
    console_write("  Zweck: Einstieg mit 5 wichtigsten Workflows.\n");
    console_write("  Syntax: welcome\n");
    console_write("  Beispiele:\n");
    console_write("    welcome\n");
    console_write("    onboarding\n");
    console_write("  Exit-Codes: 0 ok\n");
    return;
  }

  console_write("help\n");
  console_write("  Zweck: Zeigt Hilfe fuer alle Befehle oder ein Thema.\n");
  console_write("  Syntax: help [command]\n");
  console_write("  Beispiele:\n");
  console_write("    help\n");
  console_write("    help status\n");
  console_write("  Exit-Codes: 0 ok, 2 unbekanntes Thema\n");
}

static void print_onboarding(void) {
  console_write("Willkommen bei finde-os (shell-first).\n");
  console_write("1) Hilfe entdecken: help\n");
  console_write("2) Modus pruefen: status\n");
  console_write("3) Zeit pruefen: ticks\n");
  console_write("4) Rechte ansehen: cap list, cap show <id>\n");
  console_write("5) Rechte testen: cap check read|write|exec\n");
  console_write("6) Jobs steuern: job start/list/status/logs/stop\n");
  console_write("7) Hub oeffnen: hub (oder home)\n");
}

static void write_job_status(job_status_t status) {
  if (status == JOB_STATUS_RUNNING) {
    console_write("running");
  } else {
    console_write("stopped");
  }
}

static void write_security_summary(job_profile_t profile) {
  console_write("JOB_SECURITY profile=");
  console_write(job_profile_name(profile));
  console_write(" granted=");
  write_rights(job_profile_granted_rights(profile));
  console_write(" blocked=");
  write_rights(job_profile_blocked_rights(profile));
  console_write(" why=");
  console_write(job_profile_reason(profile));
  console_write("\n");
}


static void hub_footer(void) {
  console_write("Shortcuts: [j] Jobs [l] Logs [r] Retry [q] Quit\n");
}

static void hub_panel_jobs(void) {
  job_record_t rec;
  console_write("+---------------- laufende Jobs ----------------+\n");
  if (job_count() == 0) {
    console_write("(keine laufenden Jobs)\n");
    return;
  }

  for (int i = 0; i < job_count(); ++i) {
    if (job_at(i, &rec) == 0) {
      continue;
    }
    console_write("- job ");
    write_u64(rec.id);
    console_write(": ");
    console_write(rec.name);
    console_write(" [");
    write_job_status(rec.status);
    console_write("]\n");
  }
}

static void hub_panel_commands(void) {
  console_write("+-------------- haeufige Befehle ---------------+\n");
  console_write("- job start worker\n");
  console_write("- job list\n");
  console_write("- job logs 1 --follow\n");
  console_write("- status\n");
}

static void hub_panel_errors(void) {
  console_write("+------------ letzte Fehlermeldungen -----------+\n");
  console_write("- JOB_ERROR code=1 message=job not found\n");
  console_write("- CLI_DENY reason=DENY: capability security check failed\n");
}

static void hub_panel_status(void) {
  console_write("+---------------- Systemstatus -----------------+\n");
  console_write("CPU=unavailable\n");
  console_write("Mem=unavailable\n");
  console_write("UptimeTicks=");
  write_u64(timer_ticks_get());
  console_write("\n");
}

static void hub_render(void) {
  console_write("==== finde-os hub ====\n");
  hub_panel_jobs();
  hub_panel_commands();
  hub_panel_errors();
  hub_panel_status();
  hub_footer();
}

void cli_execute_validated(const cli_validated_command_t* cmd) {
  if (cmd == 0 || cmd->status != CLI_VALIDATE_OK) {
    return;
  }

  if (cmd->ast.kind == CLI_AST_EMPTY) {
    return;
  }

  if (cmd->ast.kind == CLI_AST_HELP) {
    print_help_topic(cmd->ast.arg0);
    return;
  }

  if (cmd->ast.kind == CLI_AST_ONBOARDING) {
    print_onboarding();
    return;
  }

  if (cmd->ast.kind == CLI_AST_STATUS) {
    console_write("STATUS mode=");
    if (cmd->mode == CLI_MODE_MICROVM) {
      console_write("microvm");
    } else {
      console_write("sandbox");
    }
    console_write("\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_TICKS) {
    console_write("ticks: ");
    write_u64(timer_ticks_get());
    console_write("\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_MALLOC) {
    void* a = kmalloc(16);
    void* b = kmalloc(32);
    void* c = kmalloc(64);

    console_write("malloc: ");
    write_hex_u64((uint64_t)(uintptr_t)a);
    console_write(" ");
    write_hex_u64((uint64_t)(uintptr_t)b);
    console_write(" ");
    write_hex_u64((uint64_t)(uintptr_t)c);
    console_write("\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_JOB) {
    job_record_t rec;

    if (streq(cmd->ast.arg0, "start")) {
      if (job_start(cmd->ast.arg1, cmd->profile, &rec) == 0) {
        console_write("JOB_ERROR code=1 message=job start failed\n");
        console_write("JOB_EXIT code=1\n");
        return;
      }
      console_write("JOB_START id=");
      write_u64(rec.id);
      console_write(" name=");
      console_write(rec.name);
      console_write(" handle=");
      write_u64(rec.handle);
      console_write(" status=");
      write_job_status(rec.status);
      console_write(" profile=");
      console_write(job_profile_name(rec.profile));
      console_write(" profile=");
      console_write(job_profile_name(rec.profile));
      console_write(" start=");
      write_u64(rec.start_ticks);
      console_write(" state_dir=");
      console_write(job_state_dir());
      console_write("\n");
      write_security_summary(rec.profile);
      console_write("JOB_EXIT code=0\n");
      return;
    }

    if (streq(cmd->ast.arg0, "list")) {
      console_write("JOB_LIST_HEADER id|name|handle|status|profile|start|last_output|state_dir\n");
      for (int i = 0; i < job_count(); ++i) {
        if (job_at(i, &rec) == 0) {
          continue;
        }
        write_u64(rec.id);
        console_write("|");
        console_write(rec.name);
        console_write("|");
        write_u64(rec.handle);
        console_write("|");
        write_job_status(rec.status);
        console_write("|");
        console_write(job_profile_name(rec.profile));
        console_write("|");
        write_u64(rec.start_ticks);
        console_write("|");
        console_write(rec.last_output);
        console_write("|");
        console_write(job_state_dir());
        console_write("\n");
      }
      console_write("JOB_EXIT code=0\n");
      return;
    }

    if (job_by_id(cmd->job_id, &rec) == 0) {
      console_write("JOB_ERROR code=1 message=job not found\n");
      console_write("JOB_EXIT code=1\n");
      return;
    }

    if (streq(cmd->ast.arg0, "status")) {
      console_write("JOB_STATUS id=");
      write_u64(rec.id);
      console_write(" name=");
      console_write(rec.name);
      console_write(" handle=");
      write_u64(rec.handle);
      console_write(" status=");
      write_job_status(rec.status);
      console_write(" profile=");
      console_write(job_profile_name(rec.profile));
      console_write(" profile=");
      console_write(job_profile_name(rec.profile));
      console_write(" start=");
      write_u64(rec.start_ticks);
      console_write(" last_output=");
      console_write(rec.last_output);
      console_write("\nJOB_EXIT code=0\n");
      return;
    }

    if (streq(cmd->ast.arg0, "logs")) {
      console_write("JOB_LOG id=");
      write_u64(rec.id);
      console_write(" line=");
      console_write(rec.last_output);
      console_write("\n");
      if (cmd->follow) {
        console_write("JOB_LOG_FOLLOW id=");
        write_u64(rec.id);
        console_write(" line=");
        console_write(rec.last_output);
        console_write("\n");
      }
      console_write("JOB_EXIT code=0\n");
      return;
    }

    if (streq(cmd->ast.arg0, "stop")) {
      int stop_status = job_stop(cmd->job_id, &rec);
      if (stop_status == -1) {
        console_write("JOB_ERROR code=1 message=already stopped\n");
        console_write("JOB_EXIT code=1\n");
        return;
      }

      if (stop_status == 0) {
        console_write("JOB_ERROR code=1 message=job not found\n");
        console_write("JOB_EXIT code=1\n");
        return;
      }

      console_write("JOB_STOP id=");
      write_u64(rec.id);
      console_write(" status=");
      write_job_status(rec.status);
      console_write(" profile=");
      console_write(job_profile_name(rec.profile));
      console_write("\nJOB_EXIT code=0\n");
      return;
    }
  }

  if (cmd->ast.kind == CLI_AST_HUB) {
    if (cmd->ast.arg0 == 0) {
      hub_render();
      return;
    }

    if (streq(cmd->ast.arg0, "jobs")) {
      hub_panel_jobs();
      hub_footer();
      console_write("HUB_ACTION action=jobs cli=job list\n");
      return;
    }

    if (streq(cmd->ast.arg0, "commands")) {
      hub_panel_commands();
      hub_footer();
      console_write("HUB_ACTION action=commands cli=help\n");
      return;
    }

    if (streq(cmd->ast.arg0, "errors") || streq(cmd->ast.arg0, "logs")) {
      hub_panel_errors();
      hub_footer();
      console_write("HUB_ACTION action=logs cli=job logs <id> --follow\n");
      return;
    }

    if (streq(cmd->ast.arg0, "status")) {
      hub_panel_status();
      hub_footer();
      console_write("HUB_ACTION action=status cli=status\n");
      return;
    }

    if (streq(cmd->ast.arg0, "retry")) {
      console_write("HUB_ACTION action=retry cli=job start worker\n");
      hub_footer();
      return;
    }

    if (streq(cmd->ast.arg0, "quit")) {
      console_write("HUB_ACTION action=quit cli=help\n");
      hub_footer();
      return;
    }
  }

  if (cmd->ast.kind == CLI_AST_CAP_LIST) {
    cap_snapshot_t snap;
    int found = 0;
    for (uint32_t i = 0; i < cap_capacity(); ++i) {
      if (cap_snapshot_at(i, &snap) == 0) {
        continue;
      }

      found = 1;
      console_write("CAP_LIST id=");
      write_u64(snap.handle);
      console_write(" type=");
      write_u64(snap.type);
      console_write(" rights=");
      write_rights(snap.rights);
      console_write("\n");
    }

    if (!found) {
      console_write("CAP_LIST empty\n");
    }
    return;
  }

  if (cmd->ast.kind == CLI_AST_CAP_EXPLAIN) {
    console_write("CAP_EXPLAIN profile=");
    console_write(job_profile_name(cmd->profile));
    console_write(" granted=");
    write_rights(job_profile_granted_rights(cmd->profile));
    console_write(" blocked=");
    write_rights(job_profile_blocked_rights(cmd->profile));
    console_write(" why=");
    console_write(job_profile_reason(cmd->profile));
    console_write("\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_CAP_SHOW) {
    cap_snapshot_t snap;
    if (cap_audit(cmd->handle, &snap.type, &snap.rights, &snap.generation) == 0) {
      return;
    }

    console_write("CAP_SHOW id=");
    write_u64(cmd->handle);
    console_write(" type=");
    write_u64(snap.type);
    console_write(" rights=");
    write_rights(snap.rights);
    console_write(" generation=");
    write_u64(snap.generation);
    console_write("\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_CAP_CHECK) {
    uint32_t rights = op_to_rights(cmd->ast.arg0);
    console_write("CAP_CHECK op=");
    console_write(cmd->ast.arg0);
    if (rights == 0u || cmd->handle == 0) {
      console_write(" result=DENY reason=");
      console_write(cap_deny_reason());
      console_write(" next=");
      console_write(job_profile_next_step(JOB_PROFILE_ISOLATED, rights));
      console_write("\n");
      return;
    }

    console_write(" result=ALLOW id=");
    write_u64(cmd->handle);
    console_write("\n");
  }
}
