#ifndef TIMING_H
#define TIMING_H

#include <time.h>
#include <stdint.h>

static inline uint64_t timespec_to_ns(const struct timespec *ts) {
    return (uint64_t)ts->tv_sec * 1000000000ULL + ts->tv_nsec;
}

static inline void ns_to_timespec(uint64_t ns, struct timespec *ts) {
    ts->tv_sec = ns / 1000000000ULL;
    ts->tv_nsec = ns % 1000000000ULL;
}

static inline uint64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return timespec_to_ns(&ts);
}

#endif
