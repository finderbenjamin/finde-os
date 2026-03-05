#include "job.h"

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

int job_start(const char* name, job_record_t* out) {
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
