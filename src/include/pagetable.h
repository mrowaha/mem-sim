#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <stdbool.h>
#include <stdint.h>
#include "swapspace.h"

/**
 * page table enty
 * <fcount> least significant bits are used as frame number
 * one bit is used as reference bit
 * one bit is used as modify bit
 * one bit is used as valid/invalid bit
 */
typedef uint16_t pagetableentry;

/**
 * page table implementation
 * @level: if level is 1, internally implements single-level paging, else if level is 2 implemets two-level paging
 */
typedef struct pagetable
{
  int level;
  int fcount;
  pagetableentry entries[PAGES];
} pagetable;

pagetable *newpagetable(int level, int fcount);

void update_framenumber(pagetable *, const int idx, const uint16_t framenumber);

uint16_t get_framenumber(const pagetable *, const int idx);

void set_validpte(pagetable *, const int idx);

void unset_validpte(pagetable *, const int idx);

void set_referencedpte(pagetable *, const int idx);

void unset_referencedpte(pagetable *, const int idx);

void set_modifiedpte(pagetable *, const int idx);

void unset_modifiedpte(pagetable *, const int idx);

bool isvalid_pte(const pagetable *, const int idx);

bool isreferenced_pte(const pagetable *, const int idx);

bool ismodified_pte(const pagetable *, const int idx);

#endif