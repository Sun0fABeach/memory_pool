#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "mem_management.h"

typedef uint8_t tag;
typedef unsigned char byte;

#define BOUNDARIES_SIZE (2 * (sizeof(tag)))

static inline tag *find_usable_segment(tag size);
static inline void allocateExactFit(tag *segment_tag);
static inline void allocateWithSplit(tag *segment_tag, tag requested_size);
static inline void mergePrecedingSegment(tag *start_tag);
static inline void mergeFollowingSegment(tag *end_tag);
static inline tag *locateEndTag(tag *start_tag);
static inline tag *locateStartTag(tag *end_tag);
static inline bool segmentFree(tag *segment_tag);
static inline bool segmentFits(tag *segment_tag, tag requested_size);
static inline void setFree(tag *segment_tag);
static inline void setUsed(tag *segment_tag);
static inline void destroyPool();

static byte *memory_pool;
static tag *pool_first_tag,
           *pool_final_tag,
            pool_size,
            pool_content_size;



/*********** INITIALISATION / DESTRUCTION ***********/

void init_pool() {
  pool_size = pow(2, sizeof(tag) * 8) / 2;   //calc pool size based on tag size
  //pool_size = 128;
  pool_content_size = pool_size - BOUNDARIES_SIZE;

  memory_pool = calloc(1, pool_size);                   //init pool
  pool_first_tag = (tag*)memory_pool;
  pool_final_tag = (tag*)(memory_pool + pool_size - sizeof(tag));
  *pool_first_tag = *pool_final_tag = pool_content_size;

  atexit(destroyPool);
}

static inline void destroyPool() {
  free(memory_pool);
}



/******************** ALLOCATION ********************/

/* public: allocate memory of requested size. works like malloc.
 * returns pointer to allocated memory if successful, otherwise NULL. */
void *allocate(size_t requested_size) {
  tag *segment_tag;

  if(!requested_size || requested_size > pool_content_size)
    return NULL;

  if(requested_size % 2)  //only multiples of 2 allocations allowed
    requested_size += 1;  //because lsb is flag

  if(!(segment_tag = find_usable_segment(requested_size)))
    return NULL;

  //will allocate exact fit if segment can't hold requested size + bounds size
  //or if segment has a size equal to the requested size + bounds size.
  //for the second case, splitting would only create a fragment
  //only consisting of bounds, without any content size.
  if(requested_size + BOUNDARIES_SIZE < *segment_tag)
    allocateWithSplit(segment_tag, requested_size);
  else
    allocateExactFit(segment_tag);

  return segment_tag + 1;    //return pointer to first byte after tag
}


/* checks whether a fitting free segment second can be found.
 * returns pointer to that segment or NULL, if none is found. */
static inline tag *find_usable_segment(tag size) {
  tag *probe = (tag*)memory_pool;

  do {
    if(segmentFree(probe) && segmentFits(probe, size))
      return probe;
    else
      probe = locateEndTag(probe) + 1;  //set to next start tag
  } while(probe < pool_final_tag);      

  return NULL;
}


/* allocates memory without a new fragmentation split. */
static inline void allocateExactFit(tag *segment_tag) {
  setUsed(segment_tag);
  segment_tag = locateEndTag(segment_tag);
  setUsed(segment_tag);
}


/* allocates memory with a new fragmentation split. */
static inline void allocateWithSplit(tag *segment_tag, tag requested_size) {
  tag *start_tag = segment_tag;
  tag rest_size = *start_tag - requested_size - BOUNDARIES_SIZE;

  //set start/end tags of used segment with size and flag: 1
  *start_tag = requested_size | 1;
  segment_tag = locateEndTag(start_tag);
  *segment_tag = requested_size | 1;

  //set start/end tags of remaining segment with size and flag: 0
  *++segment_tag = rest_size;
  *locateEndTag(segment_tag) = rest_size;
}



/******************** DEALLOCATION ********************/

/* public: deallocate segment. merge adjacent segments, if unused. */
void deallocate(void *segment) {
  tag *start_tag = (tag*)segment - 1;  //segment points to content
  tag *end_tag = locateEndTag(start_tag);

  setFree(start_tag);
  setFree(end_tag);

  if(start_tag != pool_first_tag && segmentFree(start_tag - 1))
    mergePrecedingSegment(start_tag);
  if(end_tag != pool_final_tag && segmentFree(end_tag + 1))
    mergeFollowingSegment(end_tag);
}


/* frees the preceding segment and merges it with the free segment 
 * given via start_tag. */
static inline void mergePrecedingSegment(tag *start_tag) {
  tag new_size = *start_tag + *(start_tag - 1) + BOUNDARIES_SIZE;
  *locateStartTag(start_tag - 1) = new_size;
  *locateEndTag(start_tag) = new_size;
}


/* frees the following segment and merges it with the free segment
 * given via end_tag. */
static inline void mergeFollowingSegment(tag *end_tag) {
  tag new_size = *end_tag + *(end_tag + 1) + BOUNDARIES_SIZE;
  *locateStartTag(end_tag) = new_size;
  *locateEndTag(end_tag + 1) = new_size;
}



/********************** HELPERS ************************/

/* returns corresponding end tag of given start tag. */
static inline tag *locateEndTag(tag *start_tag) {
  return (tag*)((byte*)start_tag + (*start_tag & ~1) + sizeof(tag));
}                         //& ~1 ignores used flag

/* returns corresponding start tag of given end tag. */
static inline tag *locateStartTag(tag *end_tag) {
  return (tag*)((byte*)end_tag - (*end_tag & ~1) - sizeof(tag));
}

/* checks whether the given segment is free. */
static inline bool segmentFree(tag *segment_tag) {
  return !(*segment_tag & 1);  //free if lsb == 0
}

/* checks whether the given segment holds enough
 * space for the requested size. */
static inline bool segmentFits(tag *segment_tag, tag requested_size) {
  return requested_size <= *segment_tag;
}

/* marks this segment as free. */
static inline void setFree(tag *segment_tag) {
  *segment_tag &= ~1;
}

/* marks this segment as used. */
static inline void setUsed(tag *segment_tag) {
  *segment_tag |= 1;
}

/* debugging function */
void debugInfo() {
  for(size_t i = 0; i < pool_size; i++)
    printf("%hhu%c", memory_pool[i], (i+1)%8 ? '\t' : '\n');
  putchar('\n');
}
