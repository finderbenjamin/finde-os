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

static int cap_resolve_live_entry(uint64_t handle, cap_entry_t** entry_out) {
  uint32_t index;
  uint32_t generation;
  if (!cap_decode(handle, &index, &generation)) {
    return 0;
  }

  cap_entry_t* entry = &cap_table[index];
  if (entry->used == 0 || entry->generation != generation) {
    return 0;
  }

  *entry_out = entry;
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

uint64_t cap_delegate(uint64_t source_handle, uint32_t delegated_rights) {
  cap_entry_t* source;
  if (!cap_resolve_live_entry(source_handle, &source)) {
    return 0;
  }

  if ((source->rights & delegated_rights) != delegated_rights) {
    return 0;
  }

  return cap_create(source->type, delegated_rights);
}

int cap_check(uint64_t handle, uint32_t expected_type, uint32_t required_rights) {
  cap_entry_t* entry;
  if (!cap_resolve_live_entry(handle, &entry)) {
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

int cap_audit(uint64_t handle, uint32_t* type_out, uint32_t* rights_out, uint32_t* generation_out) {
  uint32_t index;
  uint32_t generation;
  if (!cap_decode(handle, &index, &generation)) {
    return 0;
  }

  cap_entry_t* entry = &cap_table[index];
  if (entry->used == 0 || entry->generation != generation) {
    return 0;
  }

  if (type_out != 0) {
    *type_out = entry->type;
  }
  if (rights_out != 0) {
    *rights_out = entry->rights;
  }
  if (generation_out != 0) {
    *generation_out = entry->generation;
  }

  return 1;
}

int cap_destroy(uint64_t handle) {
  cap_entry_t* entry;
  if (!cap_resolve_live_entry(handle, &entry)) {
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

int cap_require(uint64_t handle, uint32_t expected_type, uint32_t required_rights, const char** deny_reason_out) {
  if (cap_check(handle, expected_type, required_rights) == 1) {
    if (deny_reason_out != 0) {
      *deny_reason_out = "OK";
    }
    return CAP_REQUIRE_OK;
  }

  if (deny_reason_out != 0) {
    *deny_reason_out = cap_deny_reason();
  }
  return CAP_REQUIRE_DENIED;
}

const char* cap_deny_reason(void) {
  return "DENY: capability security check failed";
}

uint32_t cap_capacity(void) {
  return CAP_TABLE_SIZE;
}

int cap_snapshot_at(uint32_t slot_index, cap_snapshot_t* snapshot_out) {
  if (snapshot_out == 0 || slot_index >= CAP_TABLE_SIZE) {
    return 0;
  }

  cap_entry_t* entry = &cap_table[slot_index];
  if (entry->used == 0) {
    return 0;
  }

  snapshot_out->handle = cap_encode(slot_index, entry->generation);
  snapshot_out->type = entry->type;
  snapshot_out->rights = entry->rights;
  snapshot_out->generation = entry->generation;
  return 1;
}
