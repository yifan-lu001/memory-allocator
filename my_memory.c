#include "my_memory.h"

// Memory allocator implementation
// Implement all other functions here...

// Initializes a node
node_t *init_node(int status, int offset, node_t *parent)
{
    // Allocate memory for the node
    node_t *newNode = (node_t *)malloc(sizeof(node_t));

    // Initialize the node
    newNode->status = status;
    newNode->offset = offset;
    newNode->prev = NULL;
    newNode->next = NULL;
    // If the node has a parent
    if (parent != NULL)
    {
        newNode->mem_size = parent->mem_size / 2;
    }
    // If the node is the root
    else
    {
        newNode->mem_size = global_mem_size;
    }
    return newNode;
}

// Allocates memory for the given alloc_size by splitting into chunks if needed
int split(node_t *node, int alloc_size, int desired_size)
{
    // If no space could be found
    if (node == NULL || node->status == ALLOCATED)
    {
        // *offset = -1; // WAS THERE A RESAON OFFSET WASN'T RESET ON THE ALLOCATED CASE
        return -1;
    }
    else if (node->status == FREE) {
        if (alloc_size == node->mem_size) {
            node->status = ALLOCATED;
            return node->offset;
        }
        else if (node->mem_size != MIN_MEM_CHUNK_SIZE && node->mem_size == desired_size)
        {
            node->status = SPLIT;
            node->prev = init_node(FREE, node->offset, node);
            node->next = init_node(FREE, node->offset + node->mem_size / 2, node);
            return split(node->prev, alloc_size, desired_size / 2);
        }
    }

    else if (node->status == SPLIT) {
        // Recursively split the tree
        int offset = split(node->prev, alloc_size, desired_size);
        // If we couldn't allocate from the left, try the right
        if (offset == -1)
        {
            offset = split(node->next, alloc_size, desired_size);
        }
        return offset;
    }

    return -1;
}

int combine(node_t *node)
{
    // If the node has two free children
    if (node->status == SPLIT && node->prev->status == FREE && node->next->status == FREE)
    {
        // Combine the children
        node->status = FREE;
        node->prev = NULL;
        node->next = NULL;
        return 0;
    }
    return -1;
}

int allocate(node_t *node, int alloc_size)
{
    // If no space could be found
    if (node == NULL || node->status == ALLOCATED)
    {
        return -1;
    }

    // If space is found in the current node
    else if (node->status == FREE && alloc_size == node->mem_size)
    {
        node->status = ALLOCATED;
        return node->offset;
    }
    // Recursively try to allocate in the tree
    else
    {
        // Try the left side first
        int offset = allocate(node->prev, alloc_size);
        if (offset == -1)
        {
            offset = allocate(node->next, alloc_size);
        }
        return offset;
    }
}

bool free_helper(node_t *node, int loc)
{
    // If the root is NULL, just return
    if (node == NULL) return false;

    // If this is the node, free it, and return
    else if (node->offset == loc && node->status == ALLOCATED)
    {
        node->status = FREE;
        return true;
    }

    // Otherwise, recursively try to free the node
    else if (node->status == SPLIT)
    {
        // Search the left
        if (free_helper(node->prev, loc)) {
            combine(node);
            return true;
        }
        // Search the right
        else if (free_helper(node->next, loc))
        {
            combine(node);
            return true;
        }
    }
    return false;
}

int get_alloc_size(int size)
{
    // Find the smallest size that works
    int alloc_size = global_mem_size;

    if (size + HEADER_SIZE < MIN_MEM_CHUNK_SIZE)
    {
        alloc_size = MIN_MEM_CHUNK_SIZE;
    }
    else
    {
        // Shrink alloc size to smallest possible
        while (alloc_size > size + HEADER_SIZE && alloc_size > MIN_MEM_CHUNK_SIZE)
        {
            alloc_size /= 2;
        }
        // Round up if needed
        if (ceil(log2(size + HEADER_SIZE)) != floor(log2(size + HEADER_SIZE)))
        {
            alloc_size *= 2;
        } 
    }
    return (alloc_size);
}

void *buddy_allocator(int size)
{
    // If we try to allocate a size greater than the max size
    if (size + HEADER_SIZE > global_mem_size)
    {
        return (void *)(-1);
    }
    // Otherwise, we try to allocate the memory
    int alloc_size = get_alloc_size(size);

    // Try to allocate immediately
    int offset = allocate(buddy_binary_tree, alloc_size);

    // If allocation failed
    if (offset == -1)
    {
        int desired_size = alloc_size;
        // Increase desired size until it works
        while (desired_size <= MEMORY_SIZE && offset == -1)
        {
            desired_size *= 2;
            offset = split(buddy_binary_tree, alloc_size, desired_size);
        }
    }

    // If we were unable to allocate, return -1
    if (offset == -1) {
        return (void *)(-1);
    }
    // Otherwise return the start of the chunk
    else {
        return (void *)((long)global_mem_offset + offset + HEADER_SIZE);
    }
}

void *slab_allocator(int size)
{
    // return ;
    // If we try to allocate a size greater than the max size
    if (size * N_OBJS_PER_SLAB > global_mem_size)
    {
        return (void *)(-1);
    }

    // Make sure we have space to allocate new slab
    slab_t *slab = slab_descriptor_table;
    int current_mem_size = 0;
    int temp;
    while (slab)
    {
        temp = get_alloc_size(size);
        current_mem_size += temp;
        // If there's an empty chunk
        if (slab->slab_status != FULLY_ALLOCATED && slab->slab_type == size) // TODO: Should this be after increasing size?
        {
            break;
        }
        else if (current_mem_size + (size * N_OBJS_PER_SLAB) > global_mem_size)
        {
            return (void *)(-1);
        }
        slab = slab->next;
    }

    // Now we can allocate the memory
    // Reset slab
    slab = slab_descriptor_table;
    void *ptr;

    // If this is the first slab to be allocated
    if (!slab_descriptor_table)
    {
        // Use buddy to allocate memory
        slab_descriptor_table = init_slab(size);
        slab_descriptor_table->num_allocated_chunks = 1;
        ptr = buddy_allocator(slab_descriptor_table->slab_size);

        // If buddy can't allocate
        if (ptr == (void *)(-1))
        {
            slab_descriptor_table = NULL;
            return (void *)(-1);
        }
        ptr += HEADER_SIZE;
        slab_descriptor_table->slab_offset = (long)(ptr - global_mem_offset);
        slab_descriptor_table->slab_chunks[0] = ALLOCATED;
        return ptr;
    }

    // Check if there is a chunk of appropriate size in the table
    while (slab)
    {
        if (slab->slab_status != FULLY_ALLOCATED && slab->slab_type == size)
        {
            // Iterate through available slabs to see if we can find an empty chunk
            for (int i = 0; i < N_OBJS_PER_SLAB; i++)
            {
                if (slab->slab_chunks[i] == FREE)
                {
                    slab->slab_chunks[i] = ALLOCATED;
                    slab->num_allocated_chunks++;

                    slab->slab_status = (slab->num_allocated_chunks == N_OBJS_PER_SLAB) ? FULLY_ALLOCATED : PARTIALLY_ALLOCATED;

                    ptr = (void *)((long)global_mem_offset + slab->slab_offset + i * (size + HEADER_SIZE));
                    return ptr;
                }
            }
        }
        slab = slab->next;
    }

    // If we couldn't find an empty chunk, try to allocate a new chunk
    slab = slab_descriptor_table;
    // Find next available spot in the table
    while (slab->next)
    {
        slab = slab->next;
    }
    // Create a slab of the appropriate size
    slab->next = init_slab(size);
    slab->next->num_allocated_chunks = 1;
    slab->next->slab_chunks[0] = 1;
    ptr = buddy_allocator(slab->next->slab_size);

    // If buddy can't allocate
    if (ptr == (void *)(-1))
    {
        slab->next = NULL;
        return (void *)(-1);
    }
    else
    {
        ptr += HEADER_SIZE;
        slab->next->slab_offset = (long)(ptr - global_mem_offset);
        return ptr;
    }
}

void buddy_free(void *ptr)
{
    // If we try to free a NULL pointer
    if (ptr == NULL)
    {
        return;
    }
    // Else, we try to free the memory
    int pointer_with_offset = (long)(ptr - global_mem_offset) - HEADER_SIZE;
    free_helper(buddy_binary_tree, pointer_with_offset);
}

void slab_free(void *ptr)
{
    slab_t *slab1 = slab_descriptor_table;
    slab_t *slab2 = slab_descriptor_table;
    int offset = (long)(ptr - global_mem_offset);

    // Iterate through the slab descriptor table to find the chunk being freed
    while (slab1)
    {
        // If the chunk is in this slab
        if (offset >= slab1->slab_offset && offset < slab1->slab_offset + slab1->slab_size)
        {
            // Update the slab descriptor table
            for (int i = 0; i < N_OBJS_PER_SLAB; i++)
            {
                if (offset == slab1->slab_offset + i * (slab1->slab_type + HEADER_SIZE))
                {
                    slab1->slab_chunks[i] = FREE;
                    slab1->num_allocated_chunks--;                    
                    slab1->slab_status = PARTIALLY_ALLOCATED;
                    break;
                }
            }

            // If all chunks are free, free the slab
            if (slab1->num_allocated_chunks == 0)
            {
                // Call buddy free to free the buddy chunk
                buddy_free((void *)((long)global_mem_offset + slab1->slab_offset - HEADER_SIZE));
                // If this is the first slab in the table
                if (slab1->next == NULL)
                {
                    slab1 = NULL;
                }
                else
                {
                    // If this is the first slab in the table
                    if (slab2->slab_id == slab1->slab_id)
                    {
                        slab_descriptor_table = slab_descriptor_table->next;
                    }
                    // Find the slab to be freed
                    else
                    {
                        while (slab2->slab_id != slab1->slab_id)
                        {
                            slab2 = slab2->next;
                        }
                        slab2->next = slab1->next;
                    }
                }
            }
            break;
        }
        slab1 = slab1->next;
    }
}

// Initializes a slab
slab_t *init_slab(int slab_type)
{
    // Allocate memory for the slab
    slab_t *newSlab = (slab_t *)malloc(sizeof(slab_t));

    // Initialize the slab
    newSlab->next = NULL;
    newSlab->slab_size = (slab_type + HEADER_SIZE) * N_OBJS_PER_SLAB + HEADER_SIZE;
    newSlab->slab_status = FREE;
    newSlab->slab_offset = 2 * HEADER_SIZE;
    newSlab->slab_type = slab_type;
    newSlab->num_allocated_chunks = 0;
    newSlab->slab_id = slab_id_counter;
    slab_id_counter++;
    for (int i = 0; i < N_OBJS_PER_SLAB; i++)
    {
        newSlab->slab_chunks[i] = FREE;
    }
    return newSlab;
}