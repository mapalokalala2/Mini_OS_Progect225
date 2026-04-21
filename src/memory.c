#include <stdio.h>
#include "../include/memory.h"
#include "../include/os.h"
#include "../include/logger.h"

static char ram[MEMORY_SIZE];

typedef struct memory_block
{
    int start,
        size,
        pid,
        free;
    struct memory_block *next;
}memory_block;

static memory_block *memory_head = NULL;
static int total_memory = 0;

static void memory_clear_list(void) {
    memory_block *current = memory_head;
    while (current) {
        memory_block *next = current->next;
        free(current);
        current = next;
    }
    memory_head = NULL;
}

void memory_init(void) {
    // this function will clear memory state and initialize memory-management data
    memory_clear_list();
    total_memory = MEMORY_SIZE;
    memory_head = (memory_block *)malloc(sizeof(memory_block));
    memory_head->start = 0;
    memory_head->size = MEMORY_SIZE;
    memory_head->free = 1;
    memory_head->next = NULL;
    log_event("Memory initialized with size %d bytes", MEMORY_SIZE);
 }

int memory_allocation(int pid, int size) {
    // this fuction will locate free block and assign memory to process pid
        if (size <= 0 || size > MEMORY_SIZE) {
            return -1;
        }
        memory_block *current = memory_head;
        while(current){
            if(current->free && current->size >= size){
                if(current->size > size){
                    memory_block *new_block = malloc(sizeof(memory_block));
                    new_block->start = current->start + size;
                    new_block->size = current->size - size;
                    new_block->free = 1;
                    new_block->next = current->next;
                    current->next = new_block;
                }
                current->free = 0;
                current->pid = pid;
                current->size = size;
                log_event("Memory allocated: %d bytes for PID %d at address %d", size, pid, current->start);
                return current->start;
            }
            current = current->next;
        }
    return -1;
}

void memory_free(int pid) {
    // this function will free all memory owned by pid and coalesce adjacent free blocks
    if (memory_head == NULL) return;

    memory_block *current_block = memory_head;
    bool free_mem_found = false;

    // this will Mark all blocks owned by this PID as free
    while (current_block != NULL) {
        if (!current_block->free && current_block->pid == pid) {
            current_block->free = 1;
            current_block->pid = 0;
            free_mem_found = true;
        }
        current_block = current_block->next;
    }

    if (!free_mem_found) return;

    // this deals with the Coalesce adjacent free blocks (handles next and previous merging)
    memory_block *current = memory_head;
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            memory_block *temp = current->next;
            current->size += temp->size;
            current->next = temp->next;
            free(temp);
            // Stay on current block to check if it can merge with the new next block
        } else {
            current = current->next;
        }
    }
    log_event("Memory freed for PID %d and adjacent blocks coalesced", pid);
}

void memory_display(void) {
    // this function will print memory map and allocated blocks
    printf("\n==============================\n");
    printf("    Memory Map\n");
    printf("===============================\n");
    memory_block *current = memory_head;
    int used_memory = 0, free_memory = 0, block_num = 0;
    while (current != NULL) {
        printf("Block %d: Start=%d, Size=%d, PID=%d, Free=%s\n",
               block_num++, current->start, current->size,
               current->pid, current->free ? "YES" : "NO");
        if (current->free) {
            free_memory += current->size;
        } else {
            used_memory += current->size;
        }
        current = current->next;
    } 
    printf("Used Memory: %d bytes\n", used_memory);
    printf("Free Memory: %d bytes\n", free_memory);
    printf("------------------------------\n");

}
