#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include "allocator.h"
#include "internal.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

void *heap_end;
void *my_malloc(size_t size){
    size += sizeof(block_header_t);
    block_header_t *header = find_free_node(size);
    int mmapmapped=0;
    if (header == NULL){
        void *ptr;
        size_t alloc_size = ((size + 4095) / 4096) * 4096;
        if (alloc_size <= 128 * 1024){
            ptr = sbrk(max((sizeof(block_header_t) + sizeof(free_node_t)), alloc_size));
            if (ptr == (void *)-1)
            {
                perror("sbrk failed");
                return NULL;
            }
            heap_end = (void *)((uint8_t *)ptr + alloc_size);
        }
        else{
            ptr=mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if(ptr==MAP_FAILED){
                perror("mmap failed");
                return NULL;
            }
            mmapmapped=1;
        }
        header = init_header(ptr, alloc_size, mmapmapped);
        if(mmapmapped==0){
        header = split_node(header, size);}
    }
    header->free = 0;
    free_node_t *node = header_to_free_node(header);
    return (void *)node;
}

void my_free(void *ptr){
    block_header_t *header = free_node_to_header((free_node_t *)(ptr));
    header->free = 1;
    if(header->mmapmapped==0){
        merge_node(header, heap_end);
        insert_node(header);
    }
    else{
        if (munmap((void*)header, header->size) == -1) {
            perror("munmap failed");
        }
        return;
    }
}