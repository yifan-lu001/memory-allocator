#include "interface.h"
#include "my_memory.h"

// Interface implementation
// Implement APIs here...

void my_setup(enum malloc_type type, int mem_size, void *start_of_memory)
{
    global_mem_size = mem_size;
    malloc_type = type;
    global_mem_offset = (long) start_of_memory;
    buddy_binary_tree = init_node(FREE, 0, NULL);
}

void *my_malloc(int size)
{
    // If we are using buddy allocator
    if (malloc_type == MALLOC_BUDDY)
    {
        return (buddy_allocator(size));
    }

    // If we are using slab allocator
    else if (malloc_type == MALLOC_SLAB)
    {
        return (slab_allocator(size));
    }
}

void my_free(void *ptr)
{
    // If we are using buddy allocator
    if (malloc_type == MALLOC_BUDDY)
    {
        buddy_free(ptr);
    }

    // If we are using slab allocator
    else if (malloc_type == MALLOC_SLAB)
    {
        slab_free(ptr);
    }
}
