# Milestone Playbook

## Recipe: add one milestone
1. Add a new Make flag in `Makefile` as `XYZ_TEST?=0`.
2. Wire compile-time gating (`-DXYZ_TEST`) for that flag.
3. Implement the milestone test path in kernel code.
4. Ensure success emits a single serial marker: `XYZ_OK`.
5. Add exactly one new numbered test step in `scripts/test.sh`:
   - `make clean`
   - `make XYZ_TEST=1`
   - headless QEMU run to a dedicated log file
   - `grep` for `XYZ_OK`, fail otherwise
6. Update `ROADMAP.md` with the milestone and marker.

## Definition of done
- Deterministic execution (no flaky/timing-dependent pass condition).
- Headless QEMU invocation.
- Marker-based assertion via `grep`.
- `./scripts/test.sh` prints `PASS`.
