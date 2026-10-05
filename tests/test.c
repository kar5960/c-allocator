#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "allocator.h"
#include "internal.h"

#define IS_16_ALIGNED(p) (((uintptr_t)(p) & 15) == 0)

// Helper to execute tests in an isolated context (fork)
static void run_isolated_test(void (*test_func)(void), const char *test_name) {
    printf("[TEST] %-48s ", test_name);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        test_func();
        _exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("PASSED\n");
        } else {
            printf("FAILED (Crash or Assertion Failure)\n");
            exit(1);
        }
    }
}

void test_alignment_and_payload(void) {
    void *p1 = my_malloc(1);
    void *p2 = my_malloc(17);
    void *p3 = my_malloc(129);

    assert(p1 != NULL && IS_16_ALIGNED(p1));
    assert(p2 != NULL && IS_16_ALIGNED(p2));
    assert(p3 != NULL && IS_16_ALIGNED(p3));

    memset(p1, 0xAA, 1);
    memset(p2, 0xBB, 17);
    memset(p3, 0xCC, 129);

    my_free(p1);
    my_free(p2);
    my_free(p3);
}

void test_zero_byte_alloc(void) {
    void *ptr = my_malloc(0);
    if (ptr != NULL) {
        assert(IS_16_ALIGNED(ptr));
        my_free(ptr);
    }
}

void test_block_splitting(void) {
    void *ptr1 = my_malloc(512);
    my_free(ptr1);

    void *ptr2 = my_malloc(64);
    assert(ptr2 == ptr1);

    void *ptr3 = my_malloc(64);
    assert(ptr3 != NULL && ptr3 > ptr2);

    my_free(ptr2);
    my_free(ptr3);
}

void test_exact_fit_no_split(void) {
    void *p1 = my_malloc(128);
    my_free(p1);

    void *p2 = my_malloc(128);
    assert(p2 == p1);

    my_free(p2);
}


void test_forward_coalescing(void) {
    void *a = my_malloc(128);
    void *b = my_malloc(128);
    void *c = my_malloc(128);

    my_free(b);
    my_free(a);

    void *merged = my_malloc(220);
    assert(merged == a && "Forward coalescing failed to merge adjacent free blocks");

    my_free(c);
    my_free(merged);
}

void test_multi_block_coalesce_chain(void) {
    void *a = my_malloc(64);
    void *b = my_malloc(64);
    void *c = my_malloc(64);

    my_free(c);
    my_free(b);
    my_free(a);

    void *merged = my_malloc(180);
    assert(merged == a && "Chain coalescing failed across 3 blocks");

    my_free(merged);
}

void test_mmap_flag_and_munmap(void) {
    size_t large_size = 200 * 1024; // 200KB > 128KB threshold
    void *ptr = my_malloc(large_size);
    assert(ptr != NULL && IS_16_ALIGNED(ptr));

    block_header_t *header = free_node_to_header((free_node_t *)ptr);
    assert(header->mmapmapped == 1 && "Header mmapmapped flag was not set to 1");
    assert(header->size >= large_size);

    memset(ptr, 0xFE, large_size);

    my_free(ptr);
}

void test_mixed_sbrk_and_mmap(void) {
    void *sbrk_ptr = my_malloc(64);
    void *mmap_ptr = my_malloc(200 * 1024);

    block_header_t *h1 = free_node_to_header((free_node_t *)sbrk_ptr);
    block_header_t *h2 = free_node_to_header((free_node_t *)mmap_ptr);

    assert(h1->mmapmapped == 0);
    assert(h2->mmapmapped == 1);

    my_free(sbrk_ptr);
    my_free(mmap_ptr);
}


void test_free_list_traversal_stress(void) {
    #define SLOTS 32
    void *ptrs[SLOTS];

    for (int i = 0; i < SLOTS; i++) {
        ptrs[i] = my_malloc(32 + (i * 8));
        assert(ptrs[i] != NULL && IS_16_ALIGNED(ptrs[i]));
    }

    for (int i = 1; i < SLOTS; i += 2) {
        my_free(ptrs[i]);
        ptrs[i] = NULL;
    }

    for (int i = 1; i < SLOTS; i += 2) {
        ptrs[i] = my_malloc(32);
        assert(ptrs[i] != NULL);
    }

    for (int i = 0; i < SLOTS; i++) {
        if (ptrs[i]) {
            my_free(ptrs[i]);
        }
    }
}

int main(void) {
    printf("=====================================================\n");
    printf("     CUSTOM ALLOCATOR ISOLATED TEST SUITE\n");
    printf("=====================================================\n\n");

    run_isolated_test(test_alignment_and_payload, "16-Byte Alignment & Payload Writing");
    run_isolated_test(test_zero_byte_alloc, "Zero-Byte Allocation Policy");
    run_isolated_test(test_block_splitting, "Block Splitting (split_node)");
    run_isolated_test(test_exact_fit_no_split, "Exact Fit Reuse Without Splitting");
    run_isolated_test(test_forward_coalescing, "Forward Coalescing (merge_node)");
    run_isolated_test(test_multi_block_coalesce_chain, "Multi-Block Sequential Coalescing");
    run_isolated_test(test_mmap_flag_and_munmap, "mmap Threshold Check & Header Flag");
    run_isolated_test(test_mixed_sbrk_and_mmap, "Interleaved sbrk & mmap Allocations");
    run_isolated_test(test_free_list_traversal_stress, "Free List Traversal & Fragmentation Stress");

    printf("\nALL TESTS PASSED SUCCESSFULLY! 🎉\n");
    return 0;
}