#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf buf;


static void segfault_handler(int signum) {
    printf("Segmentation fault caught and suppressed.\n");
    longjmp(buf, 1); 
}

int main() {
    // Set up signal handler for segmentation faults
    if (signal(SIGSEGV, segfault_handler) == SIG_ERR) {
        fprintf(stderr, "Failed to set up signal handler\n");
        return EXIT_FAILURE;
    }

    printf("Testing segmentation fault suppression...\n");

    
    if (!setjmp(buf)) {
        int *invalid_ptr = NULL; 
        *invalid_ptr = 42;       
    } else {
        // Recovery after segmentation fault
        printf("Program recovered from segmentation fault.\n");
    }

    printf("Program execution continued after suppressing the fault.\n");

    return EXIT_SUCCESS;
}
