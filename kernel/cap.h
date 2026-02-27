#ifndef CAP_H
#define CAP_H

#include <stdint.h>

#define CAP_R_READ  (1u << 0)
#define CAP_R_WRITE (1u << 1)
#define CAP_R_EXEC  (1u << 2)

void cap_init(void);
uint64_t cap_create(uint32_t type, uint32_t rights);
int cap_check(uint64_t handle, uint32_t expected_type, uint32_t required_rights);
int cap_destroy(uint64_t handle);

#endif
