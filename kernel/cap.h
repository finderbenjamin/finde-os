#ifndef FINDE_OS_KERNEL_CAP_H
#define FINDE_OS_KERNEL_CAP_H

#include <stdint.h>

#define CAP_R_READ  (1u << 0)
#define CAP_R_WRITE (1u << 1)
#define CAP_R_EXEC  (1u << 2)

enum {
  CAP_REQUIRE_OK = 0u,
  CAP_REQUIRE_DENIED = 1u,
};

typedef struct {
  uint64_t handle;
  uint32_t type;
  uint32_t rights;
  uint32_t generation;
} cap_snapshot_t;

void cap_init(void);
uint64_t cap_create(uint32_t type, uint32_t rights);
uint64_t cap_delegate(uint64_t source_handle, uint32_t delegated_rights);
int cap_check(uint64_t handle, uint32_t expected_type, uint32_t required_rights);
int cap_audit(uint64_t handle, uint32_t* type_out, uint32_t* rights_out, uint32_t* generation_out);
int cap_destroy(uint64_t handle);
int cap_require(uint64_t handle, uint32_t expected_type, uint32_t required_rights, const char** deny_reason_out);
const char* cap_deny_reason(void);
uint32_t cap_capacity(void);
int cap_snapshot_at(uint32_t slot_index, cap_snapshot_t* snapshot_out);

#endif
