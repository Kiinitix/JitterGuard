# JitterGuard

JitterGuard is an experimental `sched_ext`-based Linux scheduling policy designed to reduce **tail latency jitter** for interactive workloads under mixed CPU pressure.

Modern schedulers are highly optimized for fairness and overall throughput. Over long time horizons, they behave efficiently and predictably. Humans, however, experience systems in micro-moments. A delayed keystroke, a brief UI freeze during compilation, or an audio glitch under CPU load can dominate perceived responsiveness.

JitterGuard focuses on those moments.

Under heavy background load such as compilation, stress testing, or batch processing, interactive tasks can suffer short wakeup delays. Even when average latency remains low, p95 and p99 tail latency can spike. JitterGuard introduces micro-isolation windows and workload-aware dispatch to reduce those spikes while preserving overall system throughput.

---

## Why This Exists

The Linux Completely Fair Scheduler (CFS) is efficient, robust, and battle-tested. However, it optimizes fairness over time rather than explicitly targeting tail latency. It does not enforce jitter budgets, nor does it actively protect latency-sensitive tasks during short contention windows.

JitterGuard explores a focused idea:

> When an interactive task wakes up, briefly prioritize it within a bounded isolation window to reduce wakeup jitter.

Instead of permanently boosting priorities or starving background tasks, JitterGuard applies short-lived isolation. Once the isolation window expires, normal scheduling behavior resumes.

The objective is not to replace CFS globally.  
The objective is to evaluate whether small, targeted isolation can measurably reduce p99 wakeup latency under mixed workloads without significantly harming throughput.

---

## Core Design

JitterGuard implements a dual-queue dispatch model. Tasks are classified as interactive or background using lightweight runtime heuristics. In the current MVP, classification is based on short execution bursts and limited cumulative runtime, approximating common interactive behavior.

When an interactive task wakes up, a short isolation window (default: 3 ms) is activated. During this window, dispatch decisions prefer interactive tasks. Background tasks are temporarily deprioritized but not starved.

This mechanism aims to prevent CPU-heavy background workloads from delaying interactive execution during critical bursts.

---

## Benchmarking Methodology

JitterGuard includes a reproducible latency measurement harness.

The foreground probe uses `clock_nanosleep` with absolute timing to avoid drift and records wakeup-to-run latency in nanoseconds. Results are exported as raw CSV data and analyzed for mean, p50, p95, p99, and maximum latency.

Background pressure can be introduced using tools such as:

- `stress-ng --cpu N`
- kernel compilation
- synthetic CPU burners

The primary evaluation metric is:

> Reduction in p99 wakeup latency under mixed load compared to CFS.

The focus is on tail reduction rather than average latency alone.

---

## Current Status

The current MVP includes:

- Dual-queue dispatch
- Isolation window activation
- Runtime-based interactive classification
- Latency measurement harness with percentile analysis

The following are not yet implemented:

- Starvation protection guarantees
- Per-CPU isolation windows
- NUMA awareness
- Adaptive isolation scaling
- Feedback-driven latency budgeting

---

## Design Tradeoffs

JitterGuard deliberately trades a small amount of background throughput for improved tail predictability. Heuristic classification may occasionally misidentify tasks, and the current isolation model is global rather than per-CPU. These tradeoffs are intentional in the MVP to keep the design minimal and experimentally transparent.

---

## Future Work

Planned improvements include per-CPU isolation windows, adaptive aggressiveness based on observed latency percentiles, improved task classification, and formal evaluation across different hardware configurations.




