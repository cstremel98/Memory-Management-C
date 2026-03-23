#ifndef __student_code_h__
#define __student_code_h__

#include <stdbool.h>
#include <stddef.h>

#define ERR_OUT_OF_MEMORY  (-1)
#define ERR_BAD_ARGUMENTS  (-2)
#define ERR_SYSCALL_FAILED (-3)
#define ERR_CALL_FAILED    (-4)
#define ERR_UNINITIALIZED   (-5)

// Question: How many bytes is this?
#define MAX_ARENA_SIZE (0x7FFFFFFF)

// Allocation strategy types
typedef enum {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
} allocation_strategy_t;

// Free list ordering strategies
typedef enum {
    ORDER_BY_ADDRESS,  // Maintain address-sorted order (enables coalescing)
    ADD_TO_FRONT,      // Always add freed nodes at the beginning
    ADD_TO_BACK        // Always add freed nodes at the end
} free_list_order_t;

//Note: size represents the number of bytes available for allocation and does
//not include the header bytes.
typedef struct __node_t {
  size_t size;
  bool is_free;
  struct __node_t *fwd;
  struct __node_t *bwd;
} node_t;

extern int statusno;
extern node_t *_free_list;
extern allocation_strategy_t _allocation_strategy;
extern free_list_order_t _free_list_order;

// Provided functions
extern int init(size_t size);
extern int destroy();
void* get_arena_start();

// Configuration functions
void set_allocation_strategy(allocation_strategy_t strategy);
void set_free_list_order(free_list_order_t order);
void print_header(node_t *header);

// Functions to write
/**
 * Find a suitable chunk using the configured allocation strategy
 * Students only need to implement the strategy logic, not linked list management
 * @param size - size of requested memory
 * @return node_t*: best node according to the configured strategy, or NULL if none found
 */
node_t* find_free_chunk(size_t size);

/**
 * Prepare node_to_allocate for allocation by splitting, if necessary, or setting metadata.
 * @param node_to_allocate - the memory chunk to prepare
 * @param size - the size of the requested memory
 * @return node_t*: the new free node created by splitting, or NULL if no split occurred
 */
node_t* split_node(node_t* node_to_allocate, size_t size);

/**
 * Given a pointer to an object to free, returns the associated header containing size
 * @param ptr: pointer to allocated memory
 * @return node_t*: given a pointer to allocated memory, return the address of the header
 */
node_t* get_header(void* ptr);

/**
 * Coalesce two adjacent free nodes into one larger node
 * @param front - the first node (must come before back in memory)
 * @param back - the second node (must come after front in memory)
 */
void coalesce_nodes(node_t* front, node_t* back);

/**
 * Add a freed node back to the free list using the configured ordering strategy
 * This function handles all the complex pointer manipulation
 * @param newly_freed_node - the node to add to the free list
 */
void add_to_free_list(node_t* newly_freed_node);

/**
 * Coalesce adjacent free chunks automatically
 * Students call this after freeing - it handles all the complexity
 * Note: Only works when free list is ordered by address
 * @param node - the node that was just freed
 */
void coalesce_free_chunks(node_t* node);

// Full functions
/**
 * Allocates a block of memory of the given size
 * @param size: size of block to allocate
 * @return
 */
extern void* mem_alloc(size_t size);

/**
 * Frees a block of memory pointed to by ptr
 * @param ptr: void* to the memory we want to free
 */
extern void mem_free(void* ptr);



#endif
