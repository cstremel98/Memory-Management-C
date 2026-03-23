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
	if (_free_list == NULL) {
		return NULL;
	}	

	node_t* new_chunk = _free_list;	
	
	if(_allocation_strategy == BEST_FIT) {
		node_t* small = NULL;

		while(new_chunk != NULL) {
			if(new_chunk->is_free && new_chunk->size >= size) {
				if(small == NULL || new_chunk->size < small->size) {
					small = new_chunk;
				}
			}
			new_chunk = new_chunk->fwd;
		}
		
		return small;
	}
	
	if(_allocation_strategy == WORST_FIT) {
		node_t* big = NULL;

		while(new_chunk != NULL) {
			if(new_chunk->is_free && new_chunk->size >= size) {
				if(big == NULL || new_chunk->size > big->size) {
					big = new_chunk;
				}
			}
			new_chunk = new_chunk->fwd;
		}

		return big;
	}

	while (new_chunk != NULL) {
		
		if (new_chunk->is_free && new_chunk->size >= size) {
				return new_chunk;
		}

		new_chunk = new_chunk->fwd;
	}

	return NULL;
}

node_t* split_node(node_t* node_to_allocate, size_t size) {
	if(node_to_allocate == NULL) {
		return NULL;
	}

	//Same size
	if(node_to_allocate->size == size) {
		node_to_allocate->is_free = false;
		return node_to_allocate;
	} 

	//Big enough
	if(node_to_allocate->size >= size + sizeof(node_t)) {
		node_t* new_node = (node_t*)((char*)node_to_allocate
                          + sizeof(node_t) + size);

		new_node->size = node_to_allocate->size - size - sizeof(node_t);
		new_node->fwd = node_to_allocate->fwd;
		new_node->bwd = node_to_allocate->bwd;
		new_node->is_free = true;

		node_to_allocate->size = node_to_allocate->size - size - sizeof(node_t);
		node_to_allocate->is_free = false;
		node_to_allocate->fwd = NULL;
		node_to_allocate->bwd = NULL;

		
		return new_node;
		
		//Too small
	} else {
		node_to_allocate->is_free = false;
		return node_to_allocate;
	}

}

node_t* get_header(void* ptr) {
	return (void*)ptr - sizeof(node_t);
}

void coalesce_nodes(node_t* front, node_t* back) {
	if ( (front == NULL || back == NULL) || (back != front->fwd) ) {
		statusno = ERR_BAD_ARGUMENTS;
		return;
	} else if (front->is_free == false || back->is_free == false) {
		statusno = ERR_CALL_FAILED;
		return;
	}

	front->size += sizeof(node_t);
	front->fwd = back->fwd;

	if(back->fwd != NULL) {
		back->fwd->bwd = front;
	}
}


void* mem_alloc(size_t size){
  // todo
  
  return NULL; // placeholder value
}

void mem_free(void *ptr){
  
  // todo

}

void add_to_free_list(node_t* newly_freed_node) {
	if(newly_freed_node == NULL) {
		return;
	}

	newly_freed_node->is_free = true;

	node_t *curr = _free_list;

	if(_free_list_order == ORDER_BY_ADDRESS) {

		
		if(_free_list == NULL) {
			newly_freed_node->bwd = NULL;
			newly_freed_node->fwd = NULL;
			_free_list = newly_freed_node;
			return;
		}
		
		//First node
		if(newly_freed_node < _free_list) {
			newly_freed_node->fwd = _free_list;
			_free_list->bwd = newly_freed_node;
			_free_list = newly_freed_node;
		}
		
		//Navigate list
		while(curr->fwd != NULL && curr->fwd < newly_freed_node) {
			curr = curr->fwd;
		}

		newly_freed_node->fwd = curr->fwd;
		newly_freed_node->bwd = curr;

		if (curr->fwd != NULL) {
				curr->fwd->bwd = newly_freed_node;
		}
	
		curr->fwd = newly_freed_node;
	
		return;
	}
	
	if(_free_list_order == ADD_TO_FRONT) {
		newly_freed_node->bwd = NULL;

		if (_free_list == NULL) {
        newly_freed_node->fwd = NULL;
        _free_list = newly_freed_node;
        return;
    }
		
		newly_freed_node->fwd = _free_list;
		_free_list->bwd = newly_freed_node;
		_free_list = newly_freed_node;
	
		return;
	}
	
	if(_free_list_order == ADD_TO_BACK) {
		newly_freed_node->fwd = NULL;

		if (_free_list == NULL) {
        newly_freed_node->bwd = NULL;
        _free_list = newly_freed_node;
        return;
    }

		while(curr->fwd != NULL) {
			curr = curr->fwd;
		}

		curr->fwd = newly_freed_node;
		newly_freed_node->bwd = curr;
	
		return;
	}

}

void coalesce_free_chunks(node_t* node) {
  // todo
}
