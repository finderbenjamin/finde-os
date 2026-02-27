#include "cap.h"

#define CAP_TABLE_SIZE 64u

typedef struct {
  uint32_t used;
  uint32_t type;
  uint32_t rights;
  uint32_t generation;
} cap_entry_t;

static cap_entry_t cap_table[CAP_TABLE_SIZE];

static uint64_t cap_make_handle(uint32_t index, uint32_t generation) {
  return ((uint64_t)generation << 32) | (uint64_t)(index + 1u);
}

static int cap_decode_handle(uint64_t handle, uint32_t* index, uint32_t* generation) {
  const uint32_t slot = (uint32_t)(handle & 0xFFFFFFFFu);
  if (slot == 0u || slot > CAP_TABLE_SIZE) {
    return 0;
  }

  *index = slot - 1u;
  *generation = (uint32_t)(handle >> 32);
  return 1;
}

void cap_init(void) {
  for (uint32_t i = 0; i < CAP_TABLE_SIZE; ++i) {
    cap_table[i].used = 0u;
    cap_table[i].type = 0u;
    cap_table[i].rights = 0u;
    cap_table[i].generation += 1u;
    if (cap_table[i].generation == 0u) {
      cap_table[i].generation = 1u;
    }
  }
}

uint64_t cap_create(uint32_t type, uint32_t rights) {
  for (uint32_t i = 0; i < CAP_TABLE_SIZE; ++i) {
    if (cap_table[i].used == 0u) {
      cap_table[i].used = 1u;
      cap_table[i].type = type;
      cap_table[i].rights = rights;
      return cap_make_handle(i, cap_table[i].generation);
    }
  }

  return 0u;
}

int cap_check(uint64_t handle, uint32_t expected_type, uint32_t required_rights) {
  uint32_t index;
  uint32_t generation;
  if (!cap_decode_handle(handle, &index, &generation)) {
    return 0;
  }

  cap_entry_t* entry = &cap_table[index];
  if (entry->used == 0u) {
    return 0;
  }
  if (entry->generation != generation) {
    return 0;
  }
  if (entry->type != expected_type) {
    return 0;
  }
  if ((entry->rights & required_rights) != required_rights) {
    return 0;
  }

  return 1;
}

int cap_destroy(uint64_t handle) {
  uint32_t index;
  uint32_t generation;
  if (!cap_decode_handle(handle, &index, &generation)) {
    return 0;
  }

  cap_entry_t* entry = &cap_table[index];
  if (entry->used == 0u || entry->generation != generation) {
    return 0;
  }

  entry->used = 0u;
  entry->type = 0u;
  entry->rights = 0u;
  entry->generation += 1u;
  if (entry->generation == 0u) {
    entry->generation = 1u;
  }

  return 1;
}
