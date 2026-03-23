#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#include <common.h>
#include "student_code.h"

int statusno, _initialized;

node_t *_free_list;
allocation_strategy_t _allocation_strategy = FIRST_FIT;
free_list_order_t _free_list_order = ORDER_BY_ADDRESS;
size_t num_free_chunks = 0;
void *_arena_start, *_arena_end;


void print_header(node_t *header){
  //Note: These printf statements may produce a segmentation fault if the buff
  //pointer is incorrect, e.g., if buff points to the start of the arena.
  printf("Header->size: %lu\n", header->size);
  printf("Header->fwd: %p\n", header->fwd);
  printf("Header->bwd: %p\n", header->bwd);
  printf("Header->is_free: %d\n", header->is_free);
}

size_t get_freelist_size() {
  size_t size = 0;
  node_t* curr_node = _free_list;
  while(curr_node != NULL) {
    size++;
    curr_node = curr_node->fwd;
  }
  return size;
}


int init(size_t size) {
  
  if(size > (size_t) MAX_ARENA_SIZE) {
    return ERR_BAD_ARGUMENTS;
  }


  // Find pagesize and increase allocation to match some multiple a page size
  // Question: Why is it good to match our allocation to the size of the page?
  int pagesize = getpagesize();

  if (pagesize <= 0)
    return ERR_CALL_FAILED;

  //Align to page size only if we're already at least one page size
  if( size >= pagesize && size % pagesize != 0 ) {
    // Calculate how much we need to increase to match the size of a page
    size -= size % pagesize;
    size += pagesize;
  }

  // Open up /dev/zero to zero-init our memory.
  int fd=open("/dev/zero",O_RDWR);
  if (fd == -1) {
    return ERR_SYSCALL_FAILED;
  }
  // Map memory from /dev/zero using mmap()
  _arena_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  if (_arena_start == MAP_FAILED) {
    return ERR_SYSCALL_FAILED;
  }

  _arena_end = _arena_start + size;
  _initialized = 1;

  _free_list = _arena_start;
  _free_list->size = size - sizeof(node_t);

  // Set up the initial free element
  _free_list->fwd = NULL;
  _free_list->bwd = NULL;
  _free_list -> is_free = true;

  return size;
}

int destroy() {

  if (_initialized == 0) {
    return ERR_UNINITIALIZED;
  }

  // Remove arena with munmap()
  if(munmap(_arena_start, _arena_end - _arena_start) == -1) {
    return ERR_SYSCALL_FAILED;
  }

  // Question: Are there memory leaks?

  // Clean up variables
  _arena_start = NULL;
  _arena_end = NULL;
  _free_list = NULL;
  _initialized = 0;

  return 0;
}

void set_allocation_strategy(allocation_strategy_t strategy) {
  _allocation_strategy = strategy;
}

void set_free_list_order(free_list_order_t order) {
  _free_list_order = order;
}

void* get_arena_start() {
  return _arena_start;
}


node_t* find_free_chunk(size_t size) {
  // todo
  return NULL; // placeholder value
}

node_t* split_node(node_t* node_to_allocate, size_t size) {
  
  // todo
  return NULL; // placeholder value
}

node_t* get_header(void* ptr) {
  // todo 
  return NULL; // placeholder value
}

void coalesce_nodes(node_t* front, node_t* back) {
  
  // todo
  
}


void* mem_alloc(size_t size){
  // todo
  
  return NULL; // placeholder value
}

void mem_free(void *ptr){
  
  // todo

}

void add_to_free_list(node_t* newly_freed_node) {
  
  // todo
  
}

void coalesce_free_chunks(node_t* node) {
  // todo
}