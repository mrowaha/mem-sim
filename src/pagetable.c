#include <stdio.h>
#include "pagetable.h"

#define REFERENCE_BIT 0x8000
#define MODIFY_BIT 0x4000
#define VALID_BIT 0x2000

#define MAX_FRAME_NUMBER (uint16_t)8191

// this function initializes the page entries to zero
// all metadata bits are unset
void init_pagetable(pagetable *);

pagetable *newpagetable(int level, int fcount)
{
  if (level != 1 && level != 2)
  {
    fprintf(stderr, "[ERROR] pagetable level can only be 1 or 2\n");
    return false;
  }

  pagetable *pt = (pagetable *)malloc(sizeof(pagetable));
  pt->level = level;
  pt->fcount = fcount;
  init_pagetable(pt);
  return pt;
}

void init_pagetable(pagetable *pt)
{
  for (int i = 0; i < PAGES; i++)
  {
    pt->entries[i] = (uint16_t)0;
  }
}

void update_framenumber(pagetable *pt, const int idx, const uint16_t framenumber)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return;
  }

  if (framenumber > MAX_FRAME_NUMBER)
  {
    fprintf(stderr, "[ERROR] max frame number can be only 8191\n");
    return;
  }
  // saves the current state of the metadata bits
  uint16_t curr_state = pt->entries[idx] & 0xe000;
  pt->entries[idx] = curr_state | framenumber;
}

uint16_t get_framenumber(const pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
  }

  return (uint16_t)(0x1fff & pt->entries[idx]);
}

void set_validpte(pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return;
  }

  pt->entries[idx] = pt->entries[idx] | VALID_BIT;
}

void unset_validpte(pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return;
  }

  pt->entries[idx] = pt->entries[idx] & 0xdfff;
}

void set_referencedpte(pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return;
  }

  pt->entries[idx] = pt->entries[idx] | REFERENCE_BIT;
}

void unset_referencedpte(pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return;
  }

  pt->entries[idx] = pt->entries[idx] & 0x7fff;
}

void set_modifiedpte(pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return;
  }

  pt->entries[idx] = pt->entries[idx] | MODIFY_BIT;
}

void unset_modifiedpte(pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return;
  }

  pt->entries[idx] = pt->entries[idx] & 0xbfff;
}

bool isvalid_pte(const pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return false;
  }

  if (pt->entries[idx] & VALID_BIT)
  {
    return true;
  }
  return false;
}

bool isreferenced_pte(const pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return false;
  }

  if (pt->entries[idx] & REFERENCE_BIT)
  {
    return true;
  }
  return false;
}

bool ismodified_pte(const pagetable *pt, const int idx)
{
  if (idx >= PAGES)
  {
    fprintf(stderr, "[ERROR] page table index cannot be greater than PAGE count\n");
    return false;
  }

  if (pt->entries[idx] & MODIFY_BIT)
  {
    return true;
  }
  return false;
}