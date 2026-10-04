#include "internal.h"

static free_node_t* head=NULL;

block_header_t* init_header(void * ptr, size_t size, int mmapmapped){
     if(!ptr) return NULL;
     block_header_t* header=(block_header_t*)ptr;
     header->free=1;
     header->size=size;
     header->mmapmapped=mmapmapped;
     return header;
}

void insert_node(block_header_t * header){
     if(!header) return;
     free_node_t* node=header_to_free_node(header);
     node->next=head;
     node->prev=NULL;
     if(head){ head->prev=node; }
     head=node;
}

void remove_node(block_header_t * header){
     if(!header) return;
     free_node_t* node=header_to_free_node(header);
     if(node!=head){
     node->prev->next=node->next;}
     else head=node->next;
     if(node->next){
     node->next->prev=node->prev;}

     node->next = NULL;
     node->prev = NULL;
}

block_header_t* find_free_node(size_t size){
    if(!head){return NULL;}
     free_node_t* curr_node=head;
     while(curr_node){
        block_header_t* header=free_node_to_header(curr_node);
        if(header->size>=(size)){
            remove_node(header);
            header=split_node(header,size);
            return header;
        }
        curr_node=curr_node->next;
     }
     return NULL;
}

block_header_t* split_node(block_header_t* header, size_t size){
    if(header->size-size>=(sizeof(block_header_t)+sizeof(free_node_t))){
         void *ptr=(void*)((uint8_t*)header+size);
         init_header(ptr,header->size-size,header->mmapmapped);
         insert_node((block_header_t*)ptr);
         header->size=size;
    }
    return header;
}

block_header_t* merge_node(block_header_t* header, void* heap_end){
     block_header_t* next_header=(block_header_t*)((uint8_t*)header+header->size);
     if((void*)next_header<heap_end&&next_header->free==1){
        header->size+=next_header->size;
        remove_node(next_header);
     }
     return header;
}