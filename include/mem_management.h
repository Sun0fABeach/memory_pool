#include <stdio.h>

/* call to setup the memory pool */
void init_pool();

/* allocates memory if the requested size.
 * returns pointer to memory on success, NULL on failure. */
void *allocate(size_t requested_size);

/* frees given memory segment. */
void deallocate(void *segment);

/* print out memory pool */
void debugInfo();
