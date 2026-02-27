# Codex Operating Checklist (finde-os)

## 0) Start here on every task
- Read `ROADMAP.md` and `scripts/test.sh` first.
- Identify the next unchecked milestone and its serial success marker (`XYZ_OK`).
- Keep all changes scoped to that single milestone.

## 1) PR scope and delivery
- Exactly **one milestone per PR**.
- No unrelated refactors, formatting sweeps, or behavior changes.
- Keep diffs small and deterministic.

## 2) Milestone test contract (mandatory)
For every new milestone:
- Add exactly one test stage to `scripts/test.sh`.
- Build that stage with a dedicated Make flag: `XYZ_TEST=1`.
- Kernel success path prints exactly one marker: `XYZ_OK`.
- `scripts/test.sh` must run headless QEMU and validate with serial log grep.
- If tests fail, fix and rerun until `./scripts/test.sh` prints `PASS`.
- Never skip tests.

## 3) Build contract
- Declare milestone flags in `Makefile` as `XYZ_TEST?=0` defaults.
- Gate test-only kernel paths with compile-time flags only.
- Keep QEMU deterministic/headless (`-display none`, `-serial stdio`, `timeout`, `-no-reboot`, `-no-shutdown`).

## 4) Roadmap upkeep
- After implementing a milestone, append/update it in `ROADMAP.md` with its marker.
- Keep roadmap entries ordered and unambiguous.

## 5) Required PR output
Include all of the following in the final response:
- PR title
- PR description (what changed + why)
- WSL run instructions:
  - apt packages only if newly required
  - build command(s)
  - test command(s)
  - interactive QEMU command
- Test proof line: `./scripts/test.sh PASS`

## 6) Long-term direction (north star)
- Capability-first kernel security model.
- Prefer user-space drivers where feasible.
- Evolve toward dual app isolation modes:
  - normal sandboxed process mode
  - fully isolated MicroVM mode
