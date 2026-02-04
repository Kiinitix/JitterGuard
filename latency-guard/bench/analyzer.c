#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "common.h"

static int cmp_u64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a;
    uint64_t y = *(const uint64_t *)b;
    return (x > y) - (x < y);
}

int main(int argc, char *argv[]) {

    FILE *fp = stdin;
    if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (!fp) DIE("fopen");
    }

    char line[256];
    size_t cap = 1024;
    size_t count = 0;

    uint64_t *latencies = malloc(cap * sizeof(uint64_t));
    if (!latencies) DIE("malloc");

    /* skip header */
    if (!fgets(line, sizeof(line), fp)) {
        fprintf(stderr, "Empty input\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), fp)) {
        char *token;
        int col = 0;
        uint64_t latency = 0;

        token = strtok(line, ",");
        while (token) {
            if (col == 3) {
                latency = strtoull(token, NULL, 10);
                break;
            }
            token = strtok(NULL, ",");
            col++;
        }

        if (count == cap) {
            cap *= 2;
            latencies = realloc(latencies, cap * sizeof(uint64_t));
            if (!latencies) DIE("realloc");
        }

        latencies[count++] = latency;
    }

    qsort(latencies, count, sizeof(uint64_t), cmp_u64);

    double mean = 0.0;
    for (size_t i = 0; i < count; i++) {
        mean += latencies[i];
    }
    mean /= count;

    size_t p50 = (size_t)(0.50 * count);
    size_t p95 = (size_t)(0.95 * count);
    size_t p99 = (size_t)(0.99 * count);

    printf("Samples: %zu\n", count);
    printf("Mean latency: %.2f ns\n", mean);
    printf("p50 latency:  %lu ns\n", latencies[p50]);
    printf("p95 latency:  %lu ns\n", latencies[p95]);
    printf("p99 latency:  %lu ns\n", latencies[p99]);
    printf("Max latency:  %lu ns\n", latencies[count - 1]);

    free(latencies);
    if (fp != stdin) fclose(fp);
    return 0;
}
