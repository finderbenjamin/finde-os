# finde-os Vision

## USP direction
- **Capability-first kernel + user-space drivers** is the core differentiator.
- **Dual app-mode (normal process vs MicroVM)** is the long-term product concept.

## What this means
- The kernel should evolve toward explicit capabilities for authority, minimizing ambient privilege.
- Drivers should move to user space when feasible to reduce kernel blast radius.
- Applications should be deployable in two modes:
  1. normal sandboxed process mode for efficiency
  2. MicroVM isolation mode for stronger boundaries

## Why it matters
- Better fault isolation improves system stability.
- Reduced kernel attack surface improves security.
- Per-app security level selection allows practical tradeoffs for desktop and cloud workloads.
