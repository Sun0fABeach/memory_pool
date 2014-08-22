#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mem_management.h"

#define TAGSIZE 1

int main() {
  init_pool();
  debugInfo();

#if TAGSIZE==1
  puts("------------- Allocations --------------\n");

  printf("Trying to allocate segment size %d:\n", 6);
  void *p1 = allocate(6);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 2);
  void *p2 = allocate(2);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 4);
  void *p3 = allocate(4);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 8);
  void *p4 = allocate(8);
  debugInfo();

  puts("------------ Deallocations -----------\n");
  
  printf("Deallocating segment size %d:\n", 6);
  deallocate(p1);
  debugInfo();
  printf("Deallocating segment size %d:\n", 4);
  deallocate(p3);
  debugInfo();
  printf("Deallocating segment size %d:\n", 2);
  deallocate(p2);
  debugInfo();
  printf("Deallocating segment size %d:\n", 8);
  deallocate(p4);
  debugInfo();

  puts("------------ Corner cases ------------\n");

  printf("Trying to allocate segment size %d:\n", 123);
  p1 = allocate(123);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 1);
  p2 = allocate(1);
  debugInfo();
  printf("Deallocating segment size %d:\n", 123);
  deallocate(p1);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 62);
  p1 = allocate(62);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 62);
  p2 = allocate(62);
  debugInfo();
  printf("Deallocating first segment size %d:\n", 62);
  deallocate(p1);
  debugInfo();
  printf("Deallocating second segment size %d:\n", 62);
  deallocate(p2);
  debugInfo();

#elif TAGSIZE==4
  /* TESTS FOR TAGS SIZE 4 */

  printf("Trying to allocate segment size %d:\n", 56);
  void *p1 = allocate(56);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 4);
  void *p2 = allocate(4);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 10);
  void *p3 = allocate(10);
  debugInfo();
  printf("Trying to deallocate segment size %d:\n", 10);
  deallocate(p3);
  debugInfo();
  printf("Trying to deallocate segment size %d:\n", 56);
  deallocate(p1);
  debugInfo();
  printf("Trying to deallocate segment size %d:\n", 4);
  deallocate(p2);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 111);
  p1 = allocate(111);
  debugInfo();
  printf("Trying to allocate segment size %d:\n", 2);
  p1 = allocate(2);
  debugInfo();
#endif

  return EXIT_SUCCESS;
}

