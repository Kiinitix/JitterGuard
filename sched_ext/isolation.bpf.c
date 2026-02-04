// SPDX-License-Identifier: GPL
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <sched_ext.h>
#include "isolation.h"

char LICENSE[] SEC("license") = "GPL";

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 10240);
    __type(key, u32);
    __type(value, u64);
} wakeup_map SEC(".maps");

/* Global isolation window */
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, u32);
    __type(value, u64);
} isolation_deadline SEC(".maps");

/* Simple FIFO dispatch queues */
struct {
    __uint(type, BPF_MAP_TYPE_QUEUE);
    __uint(max_entries, 10240);
    __type(value, u32);
} interactive_q SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_QUEUE);
    __uint(max_entries, 10240);
    __type(value, u32);
} background_q SEC(".maps");

static bool is_interactive(struct task_struct *p)
{
    /* MVP: simple heuristic */
    if (p->se.sum_exec_runtime < 5 * 1000 * 1000ULL) // <5ms runtime
        return true;

    return false;
}

SEC("sched_ext.enqueue")
void BPF_PROG(isolation_enqueue, struct task_struct *p, u64 enq_flags)
{
    u32 pid = p->pid;

    if (is_interactive(p)) {
        bpf_map_push_elem(&interactive_q, &pid, 0);

        u32 key = 0;
        u64 now = bpf_ktime_get_ns();
        u64 deadline = now + ISOLATION_NS;
        bpf_map_update_elem(&isolation_deadline, &key, &deadline, BPF_ANY);
    } else {
        bpf_map_push_elem(&background_q, &pid, 0);
    }
}

SEC("sched_ext.dispatch")
void BPF_PROG(isolation_dispatch, s32 cpu, struct task_struct *prev)
{
    u32 key = 0;
    u64 *deadline = bpf_map_lookup_elem(&isolation_deadline, &key);
    u64 now = bpf_ktime_get_ns();

    u32 pid;

    if (deadline && now < *deadline) {
        if (bpf_map_pop_elem(&interactive_q, &pid) == 0) {
            scx_bpf_dispatch(pid, SLICE_NS, 0);
            return;
        }
    }

    if (bpf_map_pop_elem(&interactive_q, &pid) == 0) {
        scx_bpf_dispatch(pid, SLICE_NS, 0);
        return;
    }

    if (bpf_map_pop_elem(&background_q, &pid) == 0) {
        scx_bpf_dispatch(pid, SLICE_NS, 0);
        return;
    }
}

SEC("sched_ext.init")
s32 BPF_PROG(isolation_init)
{
    return 0;
}
