# 0001 — Language is C

**Status:** accepted

**Context:** Crush OS is a from-scratch teaching kernel. David’s day-to-day languages are PHP, Python, and Go. The repo already stated C.

**Decision:** Phase 1 is written in C (freestanding in the kernel) plus a small amount of assembly for boot, traps, and context switch.

**Why:** C is the language OS courses and manuals still speak. It maps to the machine without a runtime we would have to explain first. Assembly stays at the edges we cannot express in C.

**Consequences:** We accept manual memory and undefined-behavior discipline. We do not introduce C++, Rust, Zig, or Go in the kernel to feel modern. Userspace language is a Phase 2 question.
