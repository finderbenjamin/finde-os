You are building an experimental x86_64 OS.

Codex-first milestone checklist (follow in order):
1) Read ROADMAP.md and scripts/test.sh before coding.
2) Implement exactly one milestone per PR (no refactors/cosmetic-only edits).
3) Add one deterministic test path for the milestone:
   - new build flag: XYZ_TEST=1
   - new exact serial success marker: XYZ_OK
   - integrate it into scripts/test.sh with make clean -> make XYZ_TEST=1 -> headless QEMU -> marker check
4) Keep QEMU headless (-display none) and validate only through serial markers.
5) Run ./scripts/test.sh after every change; if it fails, debug and retry until PASS.
6) Update ROADMAP.md by appending the new milestone + marker.
7) Keep commits small and scoped to the milestone.

Long-term direction (preserve across milestones):
- capability-first security model
- user-space drivers
- dual app isolation modes later: sandboxed process mode and fully isolated MicroVM mode
