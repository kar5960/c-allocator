#ifndef INTERNAL_H
#define INTERNAL_H

#include<stddef.h>
#include<stdint.h>

typedef struct block_header{
    size_t size;
    int free;
    int mmapmapped;
}block_header_t;

typedef struct free_node{
    struct free_node* next;
    struct free_node* prev;
}free_node_t;

static inline block_header_t* free_node_to_header(void *ptr){
       return (block_header_t *)((uint8_t*)ptr-sizeof(block_header_t));
}

static inline free_node_t* header_to_free_node(block_header_t *header){
       return (free_node_t*)((uint8_t *)header+sizeof(block_header_t));
}

block_header_t* find_free_node(size_t size);
void remove_node(block_header_t* header);
void insert_node(block_header_t* header);

block_header_t* init_header(void* ptr, size_t size, int mmapmapped);
block_header_t* split_node(block_header_t* header, size_t size);
block_header_t* merge_node(block_header_t* header, void* heap_end);

#endif