#ifndef KERNEL_JOB_H
#define KERNEL_JOB_H

#include <stdint.h>

typedef enum {
  JOB_STATUS_RUNNING = 0,
  JOB_STATUS_STOPPED = 1,
} job_status_t;

typedef struct {
  uint64_t id;
  char name[24];
  uint64_t handle;
  job_status_t status;
  uint64_t start_ticks;
  const char* last_output;
} job_record_t;

void job_state_init(void);
const char* job_state_dir(void);
int job_start(const char* name, job_record_t* out);
int job_count(void);
int job_at(int index, job_record_t* out);
int job_by_id(uint64_t id, job_record_t* out);
int job_stop(uint64_t id, job_record_t* out);

#endif
