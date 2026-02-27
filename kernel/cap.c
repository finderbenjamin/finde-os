#include "cap.h"

#define CAP_TABLE_SIZE 64u

typedef struct {
  uint8_t used;
  uint32_t type;
  uint32_t rights;
  uint32_t generation;
} cap_entry_t;

static cap_entry_t cap_table[CAP_TABLE_SIZE];

static uint64_t cap_encode(uint32_t index, uint32_t generation) {
  return ((uint64_t)generation << 32) | (uint64_t)(index + 1u);
}

static int cap_decode(uint64_t handle, uint32_t* index_out, uint32_t* generation_out) {
  uint32_t slot = (uint32_t)(handle & 0xFFFFFFFFu);
  uint32_t generation = (uint32_t)(handle >> 32);
  if (slot == 0 || slot > CAP_TABLE_SIZE) {
    return 0;
  }

  *index_out = slot - 1u;
  *generation_out = generation;
  return 1;
}

void cap_init(void) {
  for (uint32_t i = 0; i < CAP_TABLE_SIZE; ++i) {
    cap_table[i].used = 0;
    cap_table[i].type = 0;
    cap_table[i].rights = 0;
    cap_table[i].generation = 1;
  }
}

uint64_t cap_create(uint32_t type, uint32_t rights) {
  for (uint32_t i = 0; i < CAP_TABLE_SIZE; ++i) {
    if (cap_table[i].used == 0) {
      cap_table[i].used = 1;
      cap_table[i].type = type;
      cap_table[i].rights = rights;
      return cap_encode(i, cap_table[i].generation);
    }
  }

  return 0;
}

int cap_check(uint64_t handle, uint32_t expected_type, uint32_t required_rights) {
  uint32_t index;
  uint32_t generation;
  if (!cap_decode(handle, &index, &generation)) {
    return 0;
  }

  cap_entry_t* entry = &cap_table[index];
  if (entry->used == 0 || entry->generation != generation) {
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
  if (!cap_decode(handle, &index, &generation)) {
    return 0;
  }

  cap_entry_t* entry = &cap_table[index];
  if (entry->used == 0 || entry->generation != generation) {
    return 0;
  }

  entry->used = 0;
  entry->type = 0;
  entry->rights = 0;
  entry->generation += 1u;
  if (entry->generation == 0) {
    entry->generation = 1;
  }
  return 1;
}
