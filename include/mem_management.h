#include <stdio.h>
#include <stdbool.h>

/* call to setup the memory pool.
 * returns true on success, false if needed memory could not be allocated. */
bool initPool();

/* call to deallocate the memory pool. */
void destroyPool();

/* allocates memory if the requested size.
 * returns pointer to memory on success, NULL on failure. */
void *allocate(size_t requested_size);

/* frees given memory segment. */
void deallocate(void *segment);

/* print out memory pool */
void debugInfo();
