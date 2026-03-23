void* get_three_nodes() {
  void *memory = malloc(3 * sizeof(node_t));
  node_t* node0;
  node_t* node1;
  node_t* node2;

  node0 = memory;
  node1 = memory + sizeof(node_t);
  node2 = memory + 2 * sizeof(node_t);

  node0->size = 0;
  node0->is_free = true;
  node0->fwd = node1;
  node0->bwd = NULL;

  node1->size = 0;
  node1->is_free = true;
  node1->fwd = node2;
  node1->bwd = node0;

  node2->size = 0;
  node2->is_free = true;
  node2->fwd = NULL;
  node2->bwd = node1;

  return memory;
}


Test(Functions, get_header) {
  char* base = malloc(1);
  cr_assert(get_header(base) == (void*)base - sizeof(node_t));
  free(base);
}

Test(Functions, coalesce_nodes__coalesce_first) {
  void* memory = get_three_nodes();
  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + sizeof(node_t));
  node_t* node2 = (node_t*)(memory + 2 * sizeof(node_t));

  // Run the test
  size_t orig_size = node0->size;
  coalesce_nodes(node0, node1);
  cr_assert(node0->size == orig_size + sizeof(node_t));
  cr_assert(node0->fwd == node2);
  cr_assert(node0->bwd == NULL);

  // Release memory after we're done
  free(memory);

  // Question:  Why don't I need to free the individual nodes?
}

Test(Functions, coalesce_nodes__coalesce_last) {
  void* memory = get_three_nodes();
  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + sizeof(node_t));
  node_t* node2 = (node_t*)(memory + 2 * sizeof(node_t));

  // Run the test
  size_t orig_size = node1->size;
  coalesce_nodes(node1, node2);
  cr_assert(node1->size == orig_size + sizeof(node_t));
  cr_assert(node1->fwd == NULL);
  cr_assert(node1->bwd == node0);

  // Release memory after we're done
  free(memory);
}

Test(Functions, coalesce_nodes__check_null) {
  void* memory = get_three_nodes();
  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + sizeof(node_t));
  node_t* node2 = (node_t*)(memory + 2 * sizeof(node_t));

  // Check whether inputting null does nothing but sets statusno
  coalesce_nodes(node1, NULL);
  cr_assert(statusno == ERR_BAD_ARGUMENTS); // Use the errors from student_code.h, not the raw numbers!!
  cr_assert(node1->size == 0);
  cr_assert(node1->is_free == true);
  cr_assert(node1->fwd == node2);
  cr_assert(node1->bwd == node0);

  coalesce_nodes(NULL, node1);
  cr_assert(statusno == ERR_BAD_ARGUMENTS); // Use the errors from student_code.h, not the raw numbers!!
  cr_assert(node1->size == 0);
  cr_assert(node1->is_free == true);
  cr_assert(node1->fwd == node2);
  cr_assert(node1->bwd == node0);

  // Release memory after we're done
  free(memory);
}

Test(Functions, coalesce_nodes__check_order) {
  void* memory = get_three_nodes();
  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + sizeof(node_t));
  node_t* node2 = (node_t*)(memory + 2 * sizeof(node_t));

  // Check whether inputting in wrong order does nothing but sets statusno
  coalesce_nodes(node1, node0);
  cr_assert(statusno == ERR_BAD_ARGUMENTS); // Use the errors from student_code.h, not the raw numbers!!

  cr_assert(node0->size == 0);
  cr_assert(node0->is_free == true);
  cr_assert(node0->fwd == node1);
  cr_assert(node0->bwd == NULL);

  cr_assert(node1->size == 0);
  cr_assert(node1->is_free == true);
  cr_assert(node1->fwd == node2);
  cr_assert(node1->bwd == node0);

  // Release memory after we're done
  free(memory);
}

Test(Functions, coalesce_nodes__check_free) {
  void* memory = get_three_nodes();
  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + sizeof(node_t));
  node_t* node2 = (node_t*)(memory + 2 * sizeof(node_t));

  node1->is_free = false;

  // Check whether inputting is_free is checked and sets statusno
  coalesce_nodes(node0, node1);
  cr_assert(statusno == ERR_CALL_FAILED); // Use the errors from student_code.h, not the raw numbers!!

  cr_assert(node0->size == 0);
  cr_assert(node0->is_free == true);
  cr_assert(node0->fwd == node1);
  cr_assert(node0->bwd == NULL);

  cr_assert(node1->size == 0);
  cr_assert(node1->is_free == false);
  cr_assert(node1->fwd == node2);
  cr_assert(node1->bwd == node0);

  statusno = 0;
  coalesce_nodes(node1, node2);
  cr_assert(statusno == ERR_CALL_FAILED); // Use the errors from student_code.h, not the raw numbers!!

  cr_assert(node1->size == 0);
  cr_assert(node1->is_free == false);
  cr_assert(node1->fwd == node2);
  cr_assert(node1->bwd == node0);

  cr_assert(node2->size == 0);
  cr_assert(node2->is_free == true);
  cr_assert(node2->fwd == NULL);
  cr_assert(node2->bwd == node1);

  // Release memory after we're done
  free(memory);
}

Test(Functions, find_free_chunk__first_fit_0bytes) {
  // Initialize with minimal arena - just enough for header + 0 bytes
  init(sizeof(node_t));
  set_allocation_strategy(FIRST_FIT);
  
  // The free list should have one node with size 0 (arena size - header size)
  node_t* chunk = find_free_chunk(0);
  cr_assert(chunk != NULL);
  cr_assert(chunk == (node_t*)get_arena_start());  // Should be at arena start
  cr_assert(chunk->size == 0);                     // sizeof(node_t) - sizeof(node_t) = 0
  cr_assert(chunk->is_free == true);
  
  destroy();
}

Test(Functions, find_free_chunk__first_fit_1bytes) {
  // Initialize with arena big enough for header + 1 byte
  init(sizeof(node_t) + 1);
  set_allocation_strategy(FIRST_FIT);
  
  // The free list should have one node with size 1
  node_t* chunk = find_free_chunk(1);
  cr_assert(chunk != NULL);
  cr_assert(chunk == (node_t*)get_arena_start());  // Should be at arena start
  cr_assert(chunk->size == 1);                     // (sizeof(node_t) + 1) - sizeof(node_t) = 1
  cr_assert(chunk->is_free == true);
  
  destroy();
}

Test(Functions, find_free_chunk__first_fit_1bytes_notfree) {
  // Initialize with larger arena to create multiple chunks
  init(sizeof(node_t) * 3 + 10);  // Space for multiple headers and data
  set_allocation_strategy(FIRST_FIT);
  
  // Manually mark the first chunk as not free to test skipping behavior
  node_t* first_chunk = (node_t*)get_arena_start();
  first_chunk->is_free = false;  // Make first chunk unavailable
  
  // Should return NULL since we only have one chunk and it's not free
  node_t* chunk = find_free_chunk(1);
  cr_assert(chunk == NULL);  // No free chunks available
  
  destroy();
}

Test(Functions, find_free_chunk__first_fit_2bytes) {
  // Initialize with arena big enough for header + 2+ bytes  
  init(sizeof(node_t) + 10);  // Extra space for testing
  set_allocation_strategy(FIRST_FIT);
  
  // The free list should have one node with size 10
  node_t* chunk = find_free_chunk(2);
  cr_assert(chunk != NULL);
  cr_assert(chunk == (node_t*)get_arena_start());  // Should be at arena start
  cr_assert(chunk->size >= 2);                     // Should have at least 2 bytes
  cr_assert(chunk->is_free == true);
  
  destroy();
}

// Best-fit strategy tests
Test(Functions, find_free_chunk__best_fit_finds_smallest) {
  // Manually create a free list with multiple chunks of different sizes
  void* memory = malloc(5 * sizeof(node_t));
  node_t* node_10 = (node_t*)memory;                    // 10-byte chunk
  node_t* node_30 = (node_t*)(memory + sizeof(node_t)); // 30-byte chunk  
  node_t* node_20 = (node_t*)(memory + 2 * sizeof(node_t)); // 20-byte chunk
  
  // Set up 10-byte chunk
  node_10->size = 10;
  node_10->is_free = true;
  node_10->fwd = node_30;
  node_10->bwd = NULL;
  
  // Set up 30-byte chunk (largest)
  node_30->size = 30;
  node_30->is_free = true;
  node_30->fwd = node_20;
  node_30->bwd = node_10;
  
  // Set up 20-byte chunk
  node_20->size = 20;
  node_20->is_free = true;
  node_20->fwd = NULL;
  node_20->bwd = node_30;
  
  // Temporarily set the global free list
  node_t* original_free_list = _free_list;
  _free_list = node_10;
  
  set_allocation_strategy(BEST_FIT);
  node_t* chunk = find_free_chunk(15);  // Request 15 bytes
  
  cr_assert(chunk != NULL);
  cr_assert(chunk->is_free == true);
  cr_assert(chunk->size >= 15);  // Must fit the request
  cr_assert(chunk->size == 20);  // Should choose 20-byte chunk (smallest that fits 15)
  
  // Restore original state
  _free_list = original_free_list;
  free(memory);
}

Test(Functions, find_free_chunk__best_fit_exact_match) {
  // Test that best-fit finds exact matches when available
  void* memory = malloc(3 * sizeof(node_t));
  node_t* node_10 = (node_t*)memory;
  node_t* node_25 = (node_t*)(memory + sizeof(node_t));
  node_t* node_30 = (node_t*)(memory + 2 * sizeof(node_t));
  
  // Set up linked list: 10 -> 25 -> 30
  node_10->size = 10;
  node_10->is_free = true;
  node_10->fwd = node_25;
  node_10->bwd = NULL;
  
  node_25->size = 25;  // Exact match for our test
  node_25->is_free = true;
  node_25->fwd = node_30;
  node_25->bwd = node_10;
  
  node_30->size = 30;
  node_30->is_free = true;
  node_30->fwd = NULL;
  node_30->bwd = node_25;
  
  node_t* original_free_list = _free_list;
  _free_list = node_10;
  
  set_allocation_strategy(BEST_FIT);
  node_t* chunk = find_free_chunk(25);  // Request exact size
  
  cr_assert(chunk != NULL);
  cr_assert(chunk->size == 25);  // Should find exact match
  
  _free_list = original_free_list;
  free(memory);
}

// Worst-fit strategy tests  
Test(Functions, find_free_chunk__worst_fit_finds_largest) {
  // Create scenario with multiple free chunks, worst-fit should pick largest
  void* memory = malloc(3 * sizeof(node_t));
  node_t* node_10 = (node_t*)memory;
  node_t* node_40 = (node_t*)(memory + sizeof(node_t));
  node_t* node_20 = (node_t*)(memory + 2 * sizeof(node_t));
  
  // Set up linked list: 10 -> 40 -> 20
  node_10->size = 10;
  node_10->is_free = true;
  node_10->fwd = node_40;
  node_10->bwd = NULL;
  
  node_40->size = 40;  // Largest
  node_40->is_free = true;
  node_40->fwd = node_20;
  node_40->bwd = node_10;
  
  node_20->size = 20;
  node_20->is_free = true;
  node_20->fwd = NULL;
  node_20->bwd = node_40;
  
  node_t* original_free_list = _free_list;
  _free_list = node_10;
  
  set_allocation_strategy(WORST_FIT);
  node_t* chunk = find_free_chunk(5);   // Small request
  
  cr_assert(chunk != NULL);
  cr_assert(chunk->is_free == true);
  cr_assert(chunk->size >= 5);   // Must fit request
  cr_assert(chunk->size == 40);  // Should choose largest available
  
  _free_list = original_free_list;
  free(memory);
}

Test(Functions, find_free_chunk__worst_fit_no_suitable_chunk) {
  // Test worst-fit when no chunk is large enough
  void* memory = malloc(2 * sizeof(node_t));
  node_t* node_10 = (node_t*)memory;
  node_t* node_15 = (node_t*)(memory + sizeof(node_t));
  
  // Set up linked list: 10 -> 15
  node_10->size = 10;
  node_10->is_free = true;
  node_10->fwd = node_15;
  node_10->bwd = NULL;
  
  node_15->size = 15;
  node_15->is_free = true;
  node_15->fwd = NULL;
  node_15->bwd = node_10;
  
  node_t* original_free_list = _free_list;
  _free_list = node_10;
  
  set_allocation_strategy(WORST_FIT);
  node_t* chunk = find_free_chunk(20);  // Request larger than any available
  
  cr_assert(chunk == NULL);  // No suitable chunk
  
  _free_list = original_free_list;
  free(memory);
}

// Strategy comparison test
Test(Functions, find_free_chunk__strategy_comparison) {
  // Test BEST_FIT vs WORST_FIT behavior with same free list
  void* memory = malloc(3 * sizeof(node_t));
  node_t* node_15 = (node_t*)memory;
  node_t* node_25 = (node_t*)(memory + sizeof(node_t));
  node_t* node_35 = (node_t*)(memory + 2 * sizeof(node_t));
  
  // Set up linked list: 15 -> 25 -> 35
  node_15->size = 15;  // Too small for 20-byte request
  node_15->is_free = true;
  node_15->fwd = node_25;
  node_15->bwd = NULL;
  
  node_25->size = 25;  // Best fit for 20 bytes
  node_25->is_free = true;
  node_25->fwd = node_35;
  node_25->bwd = node_15;
  
  node_35->size = 35;  // Worst fit choice
  node_35->is_free = true;
  node_35->fwd = NULL;
  node_35->bwd = node_25;
  
  node_t* original_free_list = _free_list;
  _free_list = node_15;
  
  // Test BEST_FIT
  set_allocation_strategy(BEST_FIT);
  node_t* best_chunk = find_free_chunk(20);  // Request 20 bytes
  cr_assert(best_chunk != NULL);
  cr_assert(best_chunk->size == 25);  // Should choose 25 (smallest that fits)
  
  // Test WORST_FIT  
  set_allocation_strategy(WORST_FIT);
  node_t* worst_chunk = find_free_chunk(20);  // Same 20-byte request
  cr_assert(worst_chunk != NULL);
  cr_assert(worst_chunk->size == 35);  // Should choose 35 (largest available)
  
  _free_list = original_free_list;
  free(memory);
}

// Free list ordering strategy tests
Test(Functions, add_to_free_list__order_by_address_basic) {
  // Test address-ordered insertion in correct positions
  void* memory = malloc(4 * sizeof(node_t));
  node_t* node_low = (node_t*)memory;                      // Lowest address
  node_t* node_mid = (node_t*)(memory + sizeof(node_t));   // Middle address
  node_t* node_high = (node_t*)(memory + 2 * sizeof(node_t)); // Highest address
  node_t* node_new = (node_t*)(memory + 3 * sizeof(node_t));  // To be inserted
  
  // Set up existing list: low -> high (missing middle)
  node_low->size = 10;
  node_low->is_free = true;
  node_low->fwd = node_high;
  node_low->bwd = NULL;
  
  node_high->size = 30;
  node_high->is_free = true;
  node_high->fwd = NULL;
  node_high->bwd = node_low;
  
  // Set up node to insert (should go between low and high)
  node_mid->size = 20;
  node_mid->is_free = true;
  node_mid->fwd = NULL;
  node_mid->bwd = NULL;
  
  // Since node_mid is between node_low and node_high in memory address,
  // we'll simulate this by using node_new as the middle position
  node_new->size = 20;
  node_new->is_free = true;
  node_new->fwd = NULL;
  node_new->bwd = NULL;
  
  node_t* original_free_list = _free_list;
  _free_list = node_low;
  
  set_free_list_order(ORDER_BY_ADDRESS);
  
  // Insert node_new - since it's higher address than node_high, should go at end
  add_to_free_list(node_new);
  
  // Verify address ordering: low -> high -> new
  cr_assert(_free_list == node_low);
  cr_assert(node_low->fwd == node_high);
  cr_assert(node_high->fwd == node_new);
  cr_assert(node_new->fwd == NULL);
  cr_assert(node_new->bwd == node_high);
  
  _free_list = original_free_list;
  free(memory);
}

Test(Functions, add_to_free_list__add_to_front) {
  // Test LIFO behavior - newest additions go to front
  void* memory = malloc(3 * sizeof(node_t));
  node_t* node_1 = (node_t*)memory;
  node_t* node_2 = (node_t*)(memory + sizeof(node_t));
  node_t* node_3 = (node_t*)(memory + 2 * sizeof(node_t));
  
  // Set up existing list: node_1 -> node_2
  node_1->size = 10;
  node_1->is_free = true;
  node_1->fwd = node_2;
  node_1->bwd = NULL;
  
  node_2->size = 20;
  node_2->is_free = true;
  node_2->fwd = NULL;
  node_2->bwd = node_1;
  
  // Set up node to add
  node_3->size = 30;
  node_3->is_free = true;
  node_3->fwd = NULL;
  node_3->bwd = NULL;
  
  node_t* original_free_list = _free_list;
  _free_list = node_1;
  
  set_free_list_order(ADD_TO_FRONT);
  add_to_free_list(node_3);
  
  // Should be: node_3 -> node_1 -> node_2 (node_3 added to front)
  cr_assert(_free_list == node_3);
  cr_assert(node_3->fwd == node_1);
  cr_assert(node_3->bwd == NULL);
  cr_assert(node_1->bwd == node_3);
  cr_assert(node_1->fwd == node_2);
  
  _free_list = original_free_list;
  free(memory);
}

Test(Functions, add_to_free_list__add_to_back) {
  // Test FIFO behavior - newest additions go to back
  void* memory = malloc(3 * sizeof(node_t));
  node_t* node_1 = (node_t*)memory;
  node_t* node_2 = (node_t*)(memory + sizeof(node_t));
  node_t* node_3 = (node_t*)(memory + 2 * sizeof(node_t));
  
  // Set up existing list: node_1 -> node_2
  node_1->size = 10;
  node_1->is_free = true;
  node_1->fwd = node_2;
  node_1->bwd = NULL;
  
  node_2->size = 20;
  node_2->is_free = true;
  node_2->fwd = NULL;
  node_2->bwd = node_1;
  
  // Set up node to add
  node_3->size = 30;
  node_3->is_free = true;
  node_3->fwd = NULL;
  node_3->bwd = NULL;
  
  node_t* original_free_list = _free_list;
  _free_list = node_1;
  
  set_free_list_order(ADD_TO_BACK);
  add_to_free_list(node_3);
  
  // Should be: node_1 -> node_2 -> node_3 (node_3 added to back)
  cr_assert(_free_list == node_1);
  cr_assert(node_1->fwd == node_2);
  cr_assert(node_2->fwd == node_3);
  cr_assert(node_3->fwd == NULL);
  cr_assert(node_3->bwd == node_2);
  
  _free_list = original_free_list;
  free(memory);
}

Test(Functions, add_to_free_list__empty_list) {
  // Test adding to empty list works with all strategies
  void* memory = malloc(sizeof(node_t));
  node_t* node_1 = (node_t*)memory;
  
  node_1->size = 10;
  node_1->is_free = true;
  node_1->fwd = NULL;
  node_1->bwd = NULL;
  
  node_t* original_free_list = _free_list;
  
  // Test all strategies with empty list
  free_list_order_t strategies[] = {ORDER_BY_ADDRESS, ADD_TO_FRONT, ADD_TO_BACK};
  
  for (int i = 0; i < 3; i++) {
    _free_list = NULL;  // Empty list
    set_free_list_order(strategies[i]);
    add_to_free_list(node_1);
    
    // Should become the only node regardless of strategy
    cr_assert(_free_list == node_1);
    cr_assert(node_1->fwd == NULL);
    cr_assert(node_1->bwd == NULL);
  }
  
  _free_list = original_free_list;
  free(memory);
}

Test(Functions, add_to_free_list__strategy_comparison) {
  // Compare how same sequence of additions creates different orderings
  void* memory = malloc(3 * sizeof(node_t));
  node_t* node_1 = (node_t*)memory;
  node_t* node_2 = (node_t*)(memory + sizeof(node_t));
  node_t* node_3 = (node_t*)(memory + 2 * sizeof(node_t));
  
  node_t* original_free_list = _free_list;
  
  // Test ADD_TO_FRONT: should be reverse order of insertion
  _free_list = NULL;
  set_free_list_order(ADD_TO_FRONT);
  
  // Reset nodes for clean test
  node_1->size = 10; node_1->is_free = true; node_1->fwd = NULL; node_1->bwd = NULL;
  node_2->size = 20; node_2->is_free = true; node_2->fwd = NULL; node_2->bwd = NULL;
  node_3->size = 30; node_3->is_free = true; node_3->fwd = NULL; node_3->bwd = NULL;
  
  add_to_free_list(node_1);  // 1
  add_to_free_list(node_2);  // 2 -> 1  
  add_to_free_list(node_3);  // 3 -> 2 -> 1
  
  cr_assert(_free_list == node_3);
  cr_assert(node_3->fwd == node_2);
  cr_assert(node_2->fwd == node_1);
  
  // Test ADD_TO_BACK: should be same order as insertion
  _free_list = NULL;
  set_free_list_order(ADD_TO_BACK);
  
  // Reset nodes
  node_1->fwd = NULL; node_1->bwd = NULL;
  node_2->fwd = NULL; node_2->bwd = NULL;
  node_3->fwd = NULL; node_3->bwd = NULL;
  
  add_to_free_list(node_1);  // 1
  add_to_free_list(node_2);  // 1 -> 2
  add_to_free_list(node_3);  // 1 -> 2 -> 3
  
  cr_assert(_free_list == node_1);
  cr_assert(node_1->fwd == node_2);
  cr_assert(node_2->fwd == node_3);
  
  _free_list = original_free_list;
  free(memory);
}

Test(Functions, split_node__same_size) {
  void* memory = malloc(3 * sizeof(node_t));

  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + 2*sizeof(node_t));

  node0->size = 0;
  node0->is_free = true;
  node0->fwd = node1;
  node0->bwd = NULL;

  node1->size = 0;
  node1->is_free = true;
  node1->fwd = NULL;
  node1->bwd = node0;

  split_node(node0, 0);
  cr_assert(node0->size == 0);
  cr_assert(node0->is_free == false);
  cr_assert(node0->fwd == node1);
  cr_assert(node0->bwd == NULL);

  free(memory);
}

Test(Functions, split_node__too_small_to_split) {
  void* memory = malloc(3 * sizeof(node_t));

  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + 2*sizeof(node_t));

  node0->size = sizeof(node_t)-1;
  node0->is_free = true;
  node0->fwd = node1;
  node0->bwd = NULL;

  node1->size = 0;
  node1->is_free = true;
  node1->fwd = NULL;
  node1->bwd = node0;

  split_node(node0, 0);
  cr_assert(node0->size == sizeof(node_t)-1);
  cr_assert(node0->is_free == false);
  cr_assert(node0->fwd == node1);
  cr_assert(node0->bwd == NULL);

  free(memory);
}


Test(Functions, split_node__big_enough_to_split1) {
  void* memory = malloc(3 * sizeof(node_t));

  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + 2*sizeof(node_t));

  node0->size = sizeof(node_t);
  node0->is_free = true;
  node0->fwd = node1;
  node0->bwd = NULL;

  node1->size = 0;
  node1->is_free = true;
  node1->fwd = NULL;
  node1->bwd = node0;

  log_debug("split_node: pre\n");
  node_t* new_node = split_node(node0, 0);
  log_debug("split_node: post\n");

  log_debug("split_node: testing\n");
  cr_assert(new_node->size == 0);
  log_debug("split_node: testing 1\n");
  cr_assert(new_node->is_free == true);
  cr_assert(new_node->fwd == node1);
  cr_assert(new_node->bwd == NULL);

  cr_assert(node0->size == 0);
  cr_assert(node0->is_free == false);
  cr_assert(node0->fwd == NULL);
  cr_assert(node0->bwd == NULL);

  free(memory);
}


Test(Functions, split_node__big_enough_to_split2) {
  void* memory = malloc(3 * sizeof(node_t) + 1);

  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)(memory + 2*sizeof(node_t)) + 1;


  node0->size = sizeof(node_t) + 1;
  node0->is_free = true;
  node0->fwd = node1;
  node0->bwd = NULL;

  node1->size = 0;
  node1->is_free = true;
  node1->fwd = NULL;
  node1->bwd = node0;

  node_t* new_node = split_node(node0, 1);
  // node_t* new_node = node0->fwd;

  cr_assert(new_node->size == 0);
  cr_assert(new_node->is_free == true);
  cr_assert(new_node->fwd == node1);
  cr_assert(new_node->bwd == NULL);

  cr_assert(node0->size == 1);
  cr_assert(node0->is_free == false);
  cr_assert(node0->fwd == NULL);
  cr_assert(node0->bwd == NULL);

  free(memory);
}

Test(Functions, add_to_free_list__no_free_list) {
  // Note: Although we init, this is a bit of a hack since we aren't actually putting things in the arena
  init(sizeof(node_t));
  void* memory = malloc(sizeof(node_t));
  _free_list = NULL;

  node_t* node0 = (node_t*)memory;
  node0->size = 0;
  node0->fwd = NULL;
  node0->bwd = NULL;
  node0->is_free = false;

  add_to_free_list(node0);

  cr_assert(node0->size == 0); // This shouldn't change
  cr_assert_null(node0->fwd); // This should be updated if it were set to something before
  cr_assert_null(node0->bwd); // This should be updated if it were set to something before
  cr_assert_not(node0->is_free); // This isn't touched here

  cr_assert(_free_list == node0);

}

Test(Functions, add_to_free_list__free_list_head) {
  // Note: Although we init, this is a bit of a hack since we aren't actually putting things in the arena
  init(2*sizeof(node_t));
  void* memory = malloc(2*sizeof(node_t));
  _free_list = NULL;

  // This node is allocated
  node_t* node0 = (node_t*)memory;
  node0->size = 0;
  node0->fwd = NULL;
  node0->bwd = NULL;
  node0->is_free = false;

  // This node is unallocated
  node_t* node1 = (node_t*)memory + 1;
  node1->size = 0;
  node1->fwd = NULL;
  node1->bwd = NULL;
  node1->is_free = true;
  // Set the freelist to this node
  _free_list = node1;

  add_to_free_list(node0);

  cr_assert(node0->size == 0); // This shouldn't change
  cr_assert(node0->fwd == node1);
  cr_assert(node0->bwd == NULL);
  cr_assert( ! node0->is_free); // This isn't touched here

  cr_assert(node1->size == 0); // This shouldn't change
  cr_assert(node1->fwd == NULL);
  cr_assert(node1->bwd == node0);
  cr_assert(node1->is_free); // This isn't touched here

  cr_assert(_free_list == node0);
}

Test(Functions, add_to_free_list__free_list_tail) {
  // Note: Although we init, this is a bit of a hack since we aren't actually putting things in the arena
  init(2*sizeof(node_t));
  void* memory = malloc(2*sizeof(node_t));
  _free_list = NULL;

  // This node is unallocated
  node_t* node0 = (node_t*)memory;
  node0->size = 0;
  node0->fwd = NULL;
  node0->bwd = NULL;
  node0->is_free = true;
  // Set the freelist to this node
  _free_list = node0;

  // This node isnallocated
  node_t* node1 = (node_t*)memory + 1;
  node1->size = 0;
  node1->fwd = NULL;
  node1->bwd = NULL;
  node1->is_free = false;

  add_to_free_list(node1);

  cr_assert(node0->size == 0); // This shouldn't change
  cr_assert(node0->fwd == node1);
  cr_assert(node0->bwd == NULL);
  cr_assert(node0->is_free); // This isn't touched here

  cr_assert(node1->size == 0); // This shouldn't change
  cr_assert(node1->fwd == NULL);
  cr_assert(node1->bwd == node0);
  cr_assert( ! node1->is_free); // This isn't touched here

  cr_assert(_free_list == node0);
}


Test(Functions, add_to_free_list__free_list_middle) {
  // Note: Although we init, this is a bit of a hack since we aren't actually putting things in the arena
  init(3*sizeof(node_t));
  void* memory = malloc(3*sizeof(node_t));
  _free_list = NULL;

  node_t* node0 = (node_t*)memory;
  node_t* node1 = (node_t*)memory + 1;
  node_t* node2 = (node_t*)memory + 2;

  // This node is unallocated
  node0->size = 0;
  node0->fwd = node2;
  node0->bwd = NULL;
  node0->is_free = true;
  // Set the freelist to this node
  _free_list = node0;

  // This node isnallocated
  node1->size = 0;
  node1->fwd = NULL;
  node1->bwd = NULL;
  node1->is_free = false;

  // This node is unallocated
  node2->size = 0;
  node2->fwd = NULL;
  node2->bwd = node0;
  node2->is_free = true;

  add_to_free_list(node1);

  cr_assert(node0->size == 0); // This shouldn't change
  cr_assert(node0->fwd == node1);
  cr_assert(node0->bwd == NULL);
  cr_assert(node0->is_free); // This isn't touched here

  cr_assert(node1->size == 0); // This shouldn't change
  cr_assert(node1->fwd == node2);
  cr_assert(node1->bwd == node0);
  cr_assert( ! node1->is_free); // This isn't touched here

  cr_assert(node2->size == 0); // This shouldn't change
  cr_assert(node2->fwd == NULL);
  cr_assert(node2->bwd == node1);
  cr_assert(node2->is_free); // This isn't touched here

  cr_assert(_free_list == node0);
}
