#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <stdint.h>

typedef int16_t pageentry;

/**
 * base page table implementation that can be extended according to the algorithm
 * @level: if level is 1, internally implements single-level paging, else if level is 2 implemets two-level paging
 */
typedef struct pagetable
{
  int level;
} basepagetable;

#endif