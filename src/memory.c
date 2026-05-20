#include <stdio.h>
#include "../include/memory.h"
#include "../include/os.h"
#include "../include/logger.h"

/*
 * Memory Management Implementation
 * Strategy: First-Fit Allocation
 * Data Structure: Doubly-linked list (simulated via singly-linked list for simplicity)
 * 
 * This module simulates RAM by managing a linked list where each node represents 
 * a contiguous block of memory that is either 'Free' or 'Allocated'.
*/
static char ram[MEMORY_SIZE];

typedef struct memory_block
{
    int start;              // Starting address offset in RAM
    int size;               // Size of the block in bytes
    int pid;                // PID of the owner (0 if free)
    int free;               // Boolean flag: 1 if free, 0 if allocated
    struct memory_block *next; // Pointer to the next adjacent memory block
}memory_block;


static memory_block *memory_head = NULL; // memory_head serves as the entry point to the linked list representing the entire RAM space.
static int total_memory = 0;

// Traverses the list and frees all metadata structures (memory_block structs)
static void memory_clear_list(void) {

    memory_block *current = memory_head;// the algorithm starts at the very beginning of the list, which is the head pointer. It uses a temporary pointer (current) to traverse the list without losing track of the head.
    while (current) {
        memory_block *next = current->next;
        free(current);
        current = next;
    }
    memory_head = NULL;
}

// Sets up the initial state: one large contiguous free block representing the whole RAM
void memory_init(void) {
    memory_clear_list();
    total_memory = MEMORY_SIZE;

    // Create the initial root block
    memory_head = (memory_block *)malloc(sizeof(memory_block));
    if (memory_head == NULL) {
        log_event("FATAL ERROR: Could not allocate memory for system metadata.");
        return;
    }

    memory_head->start = 0;
    memory_head->size = MEMORY_SIZE;
    memory_head->pid = 0;
    memory_head->free = 1;
    memory_head->next = NULL;
    log_event("Memory initialized with size %d bytes", MEMORY_SIZE);
 }

// Implements First-Fit: returns the first block found that is large enough
int memory_allocation(int pid, int size) {
        if (size <= 0 || size > MEMORY_SIZE) {
            return -1;
        }

        memory_block *current = memory_head;
        while(current){
            // Check if block is available and has sufficient space
            if(current->free && current->size >= size){
                
                // BLOCK SPLITTING
                // If the block found is larger than the requested size, we split it.
                // This ensures we only allocate exactly what is needed, leaving the 
                // remainder as a new free block for other processes.
                if(current->size > size){
                    // Allocate a new metadata structure for the leftover space
                    memory_block *new_block = malloc(sizeof(memory_block)); //note to self: malloc( memory allocation) is a built in library function used to allocate a block on the heap during execution of a program (at runtime). It returns a pointer to the beginning of the block of memory allocated. The size of the block is specified in bytes as an argument to malloc.
                    if (new_block == NULL) {
                        log_event("Error: Failed to allocate metadata for block split.");
                        return -1;
                    }
                    
                    // The new block starts immediately after the end of the allocated portion.
                    // Address = [Current Start] + [Requested Size]
                    new_block->start = current->start + size;
                    // The size of the leftover block is the original size minus what we used.
                    new_block->size = current->size - size;
                    new_block->free = 1;
                    
                    // Update pointers to insert the new block into the linked list.
                    // new_block now points to what followed current, and current now points to new_block.
                    new_block->next = current->next;
                    current->next = new_block;
                }

                // Mark the current block as taken
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

// Releases memory and prevents fragmentation via coalescing
void memory_free(int pid) {
    if (memory_head == NULL) return;

    memory_block *current_block = memory_head;
    bool free_mem_found = false;

    while (current_block != NULL) {
        if (!current_block->free && current_block->pid == pid) {
            current_block->free = 1;
            current_block->pid = 0;
            free_mem_found = true;
        }
        current_block = current_block->next;
    }

    if (!free_mem_found) return;

    // Coalescing logic: Merge adjacent free blocks into one large block
    // This prevents external fragmentation.
    memory_block *current = memory_head;
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            // COALESCING ACTION
            // Both neighbors are free. Merge 'next' into 'current'.
            memory_block *temp = current->next;
            current->size += temp->size; // Combine sizes
            current->next = temp->next; // Link to the block after the merged one
            temp->next = NULL;          // Safety: decouple before freeing
            free(temp);                 // Delete the leftover metadata structure
            
            // IMPORTANT: We do not move 'current' forward yet. 
            // We check the same 'current' again to see if its NEW neighbor is also free.
        } else {
            current = current->next;
        }
    }
    log_event("Memory freed for PID %d and adjacent blocks coalesced", pid);
}

// Traverses the list to display the current state of RAM and calculate usage stats
void memory_display(void) {
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
