#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#include "timing.h"
#include "common.h"

#define DEFAULT_ITERATIONS 100000
#define DEFAULT_INTERVAL_US 1000

typedef struct {
    uint64_t expected_ns;
    uint64_t actual_ns;
    uint64_t latency_ns;
} sample_t;

static void pin_cpu(int cpu) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    if (sched_setaffinity(0, sizeof(set), &set) != 0) {
        DIE("sched_setaffinity");
    }
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-i iterations] [-u interval_us] [-c cpu]\n", prog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

    int iterations = DEFAULT_ITERATIONS;
    int interval_us = DEFAULT_INTERVAL_US;
    int cpu = -1;

    int opt;
    while ((opt = getopt(argc, argv, "i:u:c:")) != -1) {
        switch (opt) {
            case 'i': iterations = atoi(optarg); break;
            case 'u': interval_us = atoi(optarg); break;
            case 'c': cpu = atoi(optarg); break;
            default: usage(argv[0]);
        }
    }

    if (cpu >= 0) {
        pin_cpu(cpu);
    }

    sample_t *samples = malloc(sizeof(sample_t) * iterations);
    if (!samples) DIE("malloc");

    struct timespec next_ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &next_ts);

    uint64_t interval_ns = (uint64_t)interval_us * 1000ULL;
    uint64_t next_ns = timespec_to_ns(&next_ts);

    for (int i = 0; i < iterations; i++) {

        next_ns += interval_ns;
        ns_to_timespec(next_ns, &next_ts);

        if (clock_nanosleep(CLOCK_MONOTONIC_RAW, TIMER_ABSTIME, &next_ts, NULL) != 0) {
            DIE("clock_nanosleep");
        }

        uint64_t actual_ns = now_ns();
        uint64_t latency = actual_ns > next_ns ? actual_ns - next_ns : 0;

        samples[i].expected_ns = next_ns;
        samples[i].actual_ns = actual_ns;
        samples[i].latency_ns = latency;

        for (volatile int j = 0; j < 1000; j++);
    }

    printf("iteration,expected_ns,actual_ns,latency_ns\n");
    for (int i = 0; i < iterations; i++) {
        printf("%d,%lu,%lu,%lu\n",
               i,
               samples[i].expected_ns,
               samples[i].actual_ns,
               samples[i].latency_ns);
    }

    free(samples);
    return 0;
}
