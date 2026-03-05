#ifndef KERNEL_JOB_H
#define KERNEL_JOB_H

#include <stdint.h>

typedef enum {
  JOB_STATUS_RUNNING = 0,
  JOB_STATUS_STOPPED = 1,
} job_status_t;

typedef enum {
  JOB_PROFILE_DEFAULT = 0,
  JOB_PROFILE_ISOLATED = 1,
} job_profile_t;

typedef struct {
  uint64_t id;
  char name[24];
  uint64_t handle;
  job_status_t status;
  uint64_t start_ticks;
  const char* last_output;
  job_profile_t profile;
} job_record_t;

void job_state_init(void);
const char* job_state_dir(void);
int job_start(const char* name, job_profile_t profile, job_record_t* out);
int job_count(void);
int job_at(int index, job_record_t* out);
int job_by_id(uint64_t id, job_record_t* out);
int job_stop(uint64_t id, job_record_t* out);
const char* job_profile_name(job_profile_t profile);
uint32_t job_profile_granted_rights(job_profile_t profile);
uint32_t job_profile_blocked_rights(job_profile_t profile);
const char* job_profile_reason(job_profile_t profile);
const char* job_profile_next_step(job_profile_t profile, uint32_t required_rights);
int job_profile_from_flag(const char* profile_flag, job_profile_t* profile_out);

#endif
