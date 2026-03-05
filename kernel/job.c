#include "job.h"

#include "cap.h"
#include "idt.h"

#define JOB_MAX 8

static void copy_job_name(char* dst, const char* src, int cap) {
  int i = 0;
  if (dst == 0 || src == 0 || cap <= 0) {
    return;
  }

  while (src[i] != '\0' && i < cap - 1) {
    dst[i] = src[i];
    ++i;
  }
  dst[i] = '\0';
}

static job_record_t g_jobs[JOB_MAX];
static int g_jobs_used = 0;
static uint64_t g_next_id = 1;
static int g_initialized = 0;

void job_state_init(void) {
  if (g_initialized) {
    return;
  }
  g_jobs_used = 0;
  g_next_id = 1;
  g_initialized = 1;
}

const char* job_state_dir(void) {
  return "/state/jobs";
}

int job_start(const char* name, job_profile_t profile, job_record_t* out) {
  job_state_init();
  if (name == 0 || *name == '\0' || g_jobs_used >= JOB_MAX) {
    return 0;
  }

  job_record_t rec;
  rec.id = g_next_id++;
  copy_job_name(rec.name, name, (int)sizeof(rec.name));
  rec.handle = 1000u + rec.id;
  rec.status = JOB_STATUS_RUNNING;
  rec.start_ticks = timer_ticks_get();
  rec.last_output = "started";
  rec.profile = profile;

  g_jobs[g_jobs_used++] = rec;
  if (out != 0) {
    *out = rec;
  }
  return 1;
}

int job_count(void) {
  job_state_init();
  return g_jobs_used;
}

int job_at(int index, job_record_t* out) {
  job_state_init();
  if (index < 0 || index >= g_jobs_used || out == 0) {
    return 0;
  }

  *out = g_jobs[index];
  return 1;
}

int job_by_id(uint64_t id, job_record_t* out) {
  job_state_init();
  for (int i = 0; i < g_jobs_used; ++i) {
    if (g_jobs[i].id == id) {
      if (out != 0) {
        *out = g_jobs[i];
      }
      return 1;
    }
  }
  return 0;
}

int job_stop(uint64_t id, job_record_t* out) {
  job_state_init();
  for (int i = 0; i < g_jobs_used; ++i) {
    if (g_jobs[i].id == id) {
      if (g_jobs[i].status == JOB_STATUS_STOPPED) {
        if (out != 0) {
          *out = g_jobs[i];
        }
        return -1;
      }

      g_jobs[i].status = JOB_STATUS_STOPPED;
      g_jobs[i].last_output = "stopped";
      if (out != 0) {
        *out = g_jobs[i];
      }
      return 1;
    }
  }
  return 0;
}


const char* job_profile_name(job_profile_t profile) {
  if (profile == JOB_PROFILE_ISOLATED) {
    return "isolated";
  }
  return "default";
}

uint32_t job_profile_granted_rights(job_profile_t profile) {
  if (profile == JOB_PROFILE_ISOLATED) {
    return CAP_R_READ;
  }
  return CAP_R_READ | CAP_R_WRITE;
}

uint32_t job_profile_blocked_rights(job_profile_t profile) {
  return (CAP_R_READ | CAP_R_WRITE | CAP_R_EXEC) & ~job_profile_granted_rights(profile);
}

const char* job_profile_reason(job_profile_t profile) {
  if (profile == JOB_PROFILE_ISOLATED) {
    return "isolated profile limits job capabilities to reduce lateral movement";
  }
  return "default profile is baseline restricted but allows state updates";
}

const char* job_profile_next_step(job_profile_t profile, uint32_t required_rights) {
  if ((job_profile_granted_rights(profile) & required_rights) == required_rights) {
    return "retry command";
  }

  if (profile == JOB_PROFILE_ISOLATED) {
    return "restart with --profile=default if write access is required";
  }

  return "restart with --profile=isolated only for read-only workloads";
}

int job_profile_from_flag(const char* profile_flag, job_profile_t* profile_out) {
  if (profile_out == 0) {
    return 0;
  }

  if (profile_flag == 0) {
    *profile_out = JOB_PROFILE_ISOLATED;
    return 1;
  }

  if (profile_flag[0] == 'd' && profile_flag[1] == 'e' && profile_flag[2] == 'f' && profile_flag[3] == 'a' &&
      profile_flag[4] == 'u' && profile_flag[5] == 'l' && profile_flag[6] == 't' && profile_flag[7] == '\0') {
    *profile_out = JOB_PROFILE_DEFAULT;
    return 1;
  }

  if (profile_flag[0] == 'i' && profile_flag[1] == 's' && profile_flag[2] == 'o' && profile_flag[3] == 'l' &&
      profile_flag[4] == 'a' && profile_flag[5] == 't' && profile_flag[6] == 'e' && profile_flag[7] == 'd' && profile_flag[8] == '\0') {
    *profile_out = JOB_PROFILE_ISOLATED;
    return 1;
  }

  return 0;
}
