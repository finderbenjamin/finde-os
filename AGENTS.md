# Codex Operating Instructions (finde-os)

## Start of every task
- Read `ROADMAP.md` and `scripts/test.sh` first.
- Confirm the next unchecked roadmap milestone and its expected serial marker.
- Keep changes deterministic and milestone-scoped.

## Delivery model
- One milestone per PR.
- Deterministic behavior only (no timing-sensitive or flaky checks).
- Tiny diffs, no broad rewrites, no unrelated refactors.

## Test contract (mandatory)
- Every new milestone adds exactly one new test run in `scripts/test.sh`.
- Each test run must build with a dedicated Make flag in the form `XYZ_TEST=1`.
- Kernel success path must print exactly one serial success marker: `XYZ_OK`.
- `scripts/test.sh` must run QEMU headless and grep the marker to produce PASS/FAIL.
- If tests fail: debug and retry until green.
- Never skip tests.

## Build contract
- Add milestone flags in `Makefile` as `CAP_TEST?=0` style defaults.
- Gate compile-time paths via these flags only.
- Keep QEMU checks deterministic (`-display none`, `-serial stdio`, `timeout`, `-no-reboot`; no GUI dependency).

## Output contract for each PR
At end of work, provide:
- PR title
- PR description (what changed + why)
- How to run in WSL:
  - apt packages only if newly required
  - build and test commands
  - interactive QEMU command
- Test proof: `./scripts/test.sh PASS`

## Scope and quality guardrails
- Keep commits small and focused.
- Do not implement multiple milestones in one PR.
- Do not change existing test semantics unless the active milestone requires it.

## North Star (USP guidance; not immediate implementation)
- Capability-first kernel security model.
- Drivers in user-space whenever feasible.
- Dual app-mode concept:
  - App can run as a normal sandboxed process, or
  - App can run as a fully isolated MicroVM.
- Security level should be selectable per app, targeting both cloud and desktop use cases.
