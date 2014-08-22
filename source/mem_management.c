#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "mem_management.h"

typedef unsigned char byte;
typedef struct {
  uint8_t size:7;
  uint8_t used:1;
} Tag;

#define BOUNDARIES_SIZE (2 * (sizeof(Tag)))

static inline Tag *find_usable_segment(size_t size);
static inline void allocateExactFit(Tag *segment_tag);
static inline void allocateWithSplit(Tag *segment_tag, size_t requested_size);
static inline void mergePrecedingSegment(Tag *start_tag);
static inline void mergeFollowingSegment(Tag *end_tag);
static inline void mergeSurroundingSegments(Tag *start_tag, Tag *end_tag);
static inline Tag *locateEndTag(Tag *start_tag);
static inline Tag *locateStartTag(Tag *end_tag);
static inline bool segmentFree(Tag *segment_tag);
static inline bool segmentFits(Tag *segment_tag, size_t requested_size);
static inline void destroyPool();

static byte *memory_pool;
static Tag *pool_first_tag,
           *pool_final_tag;
static size_t pool_size,
              pool_content_size;



/*********** INITIALISATION / DESTRUCTION ***********/

void init_pool() {
  pool_size = pow(2, sizeof(Tag) * 8) / 2;   //calc pool size based on tag size
  //pool_size = 128;
  pool_content_size = pool_size - BOUNDARIES_SIZE;

  memory_pool = calloc(1, pool_size);                   //init pool
  pool_first_tag = (Tag*)memory_pool;
  pool_final_tag = (Tag*)(memory_pool + pool_size - sizeof(Tag));
  pool_first_tag->size = pool_content_size;
  pool_final_tag->size = pool_content_size;

  atexit(destroyPool);
}

static inline void destroyPool() {
  free(memory_pool);
}



/******************** ALLOCATION ********************/

/* public: allocate memory of requested size. works like malloc.
 * returns pointer to allocated memory if successful, otherwise NULL. */
void *allocate(size_t requested_size) {
  Tag *segment_tag;

  if(!requested_size || requested_size > pool_content_size)
    return NULL;

  if(requested_size % 2)  //only multiples of 2 allocations allowed
    requested_size += 1;

  if(!(segment_tag = find_usable_segment(requested_size)))
    return NULL;

  //will allocate exact fit if segment can't hold requested size + bounds size
  //or if segment has a size equal to the requested size + bounds size.
  //for the second case, splitting would only create a fragment
  //only consisting of bounds, without any content size.
  if(requested_size + BOUNDARIES_SIZE < segment_tag->size)
    allocateWithSplit(segment_tag, requested_size);
  else
    allocateExactFit(segment_tag);

  return segment_tag + 1;    //return pointer to first byte after tag
}


/* checks whether a fitting free segment second can be found.
 * returns pointer to that segment or NULL, if none is found. */
static inline Tag *find_usable_segment(size_t size) {
  Tag *probe = (Tag*)memory_pool;

  do {
    if(segmentFree(probe) && segmentFits(probe, size))
      return probe;
    else
      probe = locateEndTag(probe) + 1;  //set to next start tag
  } while(probe < pool_final_tag);      

  return NULL;
}


/* allocates memory without a new fragmentation split. */
static inline void allocateExactFit(Tag *segment_tag) {
  segment_tag->used = true;
  locateEndTag(segment_tag)->used = true;
}


/* allocates memory with a new fragmentation split.
 * segment_tag points to start tag of the to-be-used segment. */
static inline void allocateWithSplit(Tag *segment_tag, size_t requested_size) {
  size_t rest_size = segment_tag->size - requested_size - BOUNDARIES_SIZE;

  //set start/end tags of used segment with size and flag: 1
  *segment_tag = (Tag){ .size = requested_size, .used = true };
  segment_tag = locateEndTag(segment_tag);
  *segment_tag = (Tag){ .size = requested_size, .used = true };

  //set start/end tags of remaining segment with size and flag: 0
  *++segment_tag             = (Tag){ .size = rest_size, .used = false };
  *locateEndTag(segment_tag) = (Tag){ .size = rest_size, .used = false };
}



/******************** DEALLOCATION ********************/

/* public: deallocate segment. merge adjacent segments, if unused. */
void deallocate(void *segment) {
  Tag *start_tag = (Tag*)segment - 1;  //segment points to content
  Tag *end_tag = locateEndTag(start_tag);

  start_tag->used = false;
  end_tag->used = false;

  bool prev_free = start_tag != pool_first_tag && segmentFree(start_tag - 1);
  bool next_free = end_tag != pool_final_tag && segmentFree(end_tag + 1);

  if(prev_free)
    if(next_free)
      mergeSurroundingSegments(start_tag, end_tag);
    else
      mergePrecedingSegment(start_tag);
  else if(next_free)
    mergeFollowingSegment(end_tag);
}


/* frees the preceding (lefthand) segment and merges it with the free segment.
 * start_tag is the start tag of the original (righthand) free segment. */
static inline void mergePrecedingSegment(Tag *start_tag) {
  size_t new_size = start_tag->size + (start_tag - 1)->size + BOUNDARIES_SIZE;
  *locateStartTag(start_tag - 1) = (Tag){ .size = new_size, .used = false };
  *locateEndTag(start_tag)       = (Tag){ .size = new_size, .used = false };
}


/* frees the following (righthand) segment and merges it with the free segment
 * end_tag is the end tag of the original (lefthand) free segment. */
static inline void mergeFollowingSegment(Tag *end_tag) {
  size_t new_size = end_tag->size + (end_tag + 1)->size + BOUNDARIES_SIZE;
  *locateStartTag(end_tag)   = (Tag){ .size = new_size, .used = false };
  *locateEndTag(end_tag + 1) = (Tag){ .size = new_size, .used = false };
}


/* frees the surrounding segments and merges them with the free segment.
 * start_tag and end_tag belong to the original free segment. */
static inline void mergeSurroundingSegments(Tag *start_tag, Tag *end_tag) {
  size_t new_size = start_tag->size + (start_tag - 1)->size +
                    (end_tag + 1)->size + 2 * BOUNDARIES_SIZE;
  *locateStartTag(start_tag - 1) = (Tag){ .size = new_size, .used = false };
  *locateEndTag(end_tag + 1)     = (Tag){ .size = new_size, .used = false };
}


/********************** HELPERS ************************/

/* returns corresponding end tag of given start tag. */
static inline Tag *locateEndTag(Tag *start_tag) {
  return (Tag*)((byte*)start_tag + start_tag->size + sizeof(Tag));
}                         //& ~1 ignores used flag

/* returns corresponding start tag of given end tag. */
static inline Tag *locateStartTag(Tag *end_tag) {
  return (Tag*)((byte*)end_tag - end_tag->size - sizeof(Tag));
}

/* checks whether the given segment is free. */
static inline bool segmentFree(Tag *segment_tag) {
  return !segment_tag->used;
}

/* checks whether the given segment holds enough
 * space for the requested size. */
static inline bool segmentFits(Tag *segment_tag, size_t requested_size) {
  return requested_size <= segment_tag->size;
}

/* debugging function */
void debugInfo() {
  Tag *next_tag = (Tag*)memory_pool;
  byte *pool_ending = memory_pool + pool_size;
  bool is_end_tag = false;

  for(byte *p = memory_pool, i = 0; p < pool_ending; p++, i++) {
    if(p == (byte*)next_tag) {
      printf("%d(%d)", next_tag->size, next_tag->used);
      next_tag = is_end_tag ? next_tag + 1 : locateEndTag(next_tag);
      is_end_tag = !is_end_tag;
    } else {
      printf("%hhu", *p);
    }
    putchar((i+1) % 8 ? '\t' : '\n');
  }
  putchar('\n');
}
