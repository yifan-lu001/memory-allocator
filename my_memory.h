#ifndef MY_MEMORY_H
#define MY_MEMORY_H

#include "interface.h"
#include <math.h>

// Declare your own data structures and functions here...

// Status of a memory chunk
enum status
{
    FREE = 0,       // Nothing allocated in the chunk
    ALLOCATED = 1,  // Memory allocated in the chunk
    SPLIT = 2,      // Memory chunk is split into smaller chunks (children)
};

// Struct for a binary tree node, implemented as a linked-list
typedef struct node_t
{
    struct node_t *next;    // Pointer to the next node
    struct node_t *prev;    // Pointer to the previous node
    int mem_size;           // Size of the memory chunk
    int status;             // Status of the memory chunk
    int offset;             // Offset of the memory chunk from the start of the slab
} node_t;

// Status for a slab
enum slab_status
{
    // FREE = 0,                   Nothing allocated in the slab
    PARTIALLY_ALLOCATED = 1,    // Slab is partially allocated
    FULLY_ALLOCATED = 2,        // Slab is full
};

// Struct for a slab
typedef struct slab_t
{
    struct slab_t *next;            // Pointer to the next slab
    int slab_size;                  // Size of the slab
    int slab_status;                // Status of the slab
    int slab_offset;                // Offset of the slab from the start of the memory
    int slab_type;                  // Size of memory held in the slab
    int num_allocated_chunks;       // Number of allocated chunks in the slab
    int slab_chunks[N_OBJS_PER_SLAB];    // Array to store the allocated chunks
    int slab_id;                    // ID of the slab
} slab_t;

int malloc_type;            // Type of memory allocator
int global_mem_size;        // Size of the memory
int global_mem_offset;      // Offset of the memory from the start of the slab
void *start_of_memory;      // Pointer to the start of the memory
int slab_id_counter;    // Counter to keep track of the slab ID
node_t *buddy_binary_tree;  // Binary tree for buddy allocation
slab_t *slab_descriptor_table;  // Slab descriptor table for slab allocation

// Buddy allocation functions
node_t* init_node(int status, int offset, node_t *parent);
int split(node_t *node, int alloc_size, int desired_size);
int combine(node_t *node);
int allocate(node_t *node, int alloc_size);
int get_alloc_size(int size);
void slab_setup(enum malloc_type type, int mem_size, void *start_of_memory);
bool free_helper(node_t *node, int loc);
void* buddy_allocator(int size);
void buddy_free(void *ptr);

// Slab allocation functions
slab_t* init_slab(int slab_type);
void* slab_allocator(int size);
void slab_free(void *ptr);

#endif
