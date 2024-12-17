#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 256
#define PAGE_SIZE 4096
#define TOTAL_TESTS 100000

uint16_t threshold;

// Calibrate threshold for cache hits and misses
uint16_t calibrate_threshold() {
    uint8_t *temp = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
    if (!temp) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Measure cache hit time
    *(volatile uint8_t *)temp;
    uint64_t start = __rdtsc();
    *(volatile uint8_t *)temp;
    uint64_t end = __rdtsc();
    uint64_t hit_time = end - start;

    // Measure cache miss time
    _mm_clflush(temp);
    start = __rdtsc();
    *(volatile uint8_t *)temp;
    end = __rdtsc();
    uint64_t miss_time = end - start;

    free(temp);

    uint16_t calculated_threshold = (hit_time + miss_time) / 2;
    printf("Calibrated threshold: %u (hit: %lu, miss: %lu)\n", calculated_threshold, hit_time, miss_time);
    return calculated_threshold;
}

int main() {
    uint8_t *UserArray = aligned_alloc(PAGE_SIZE, ARRAY_SIZE * PAGE_SIZE);
    if (!UserArray) {
        perror("Memory allocation failed");
        return EXIT_FAILURE;
    }
    memset(UserArray, 0, ARRAY_SIZE * PAGE_SIZE);

    srand(time(NULL)); // Seed RNG
    threshold = calibrate_threshold(); // Set threshold based on calibration

    int correct = 0;

    for (int test = 0; test < TOTAL_TESTS; test++) {
        // Flush all cache lines
        for (int i = 0; i < ARRAY_SIZE; i++) {
            _mm_clflush(&UserArray[i * PAGE_SIZE]);
        }

        // Transmit secret data
        uint8_t secret = rand() % ARRAY_SIZE;
        *(volatile uint8_t *)&UserArray[secret * PAGE_SIZE];

        // Decode secret data
        uint8_t received = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            uint64_t start, end;
            _mm_lfence();
            start = __rdtsc();
            *(volatile uint8_t *)&UserArray[i * PAGE_SIZE];
            _mm_lfence();
            end = __rdtsc();

            uint32_t time = (uint32_t)(end - start);
            if (time < threshold) { // Cache hit
                received = i;
                break;
            }
        }

        if (received == secret) {
            correct++;
        }
    }

    // Print results
    double accuracy = (double)correct / TOTAL_TESTS * 100.0;
    printf("Accuracy: %.2f%% (%d/%d)\n", accuracy, correct, TOTAL_TESTS);

    free(UserArray);
    return 0;
}
