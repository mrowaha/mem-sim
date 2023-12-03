#include <stdio.h>
#include "pagetable.h"

// log levels
// #define ERROR
#define INFO

#ifdef ERROR
#define LOG_ERROR(...) fprintf(stderr, ##__VA_ARGS__);
#else
#define LOG_ERROR(...) (void)0;
#endif

#ifdef INFO
#define LOG_INFO(...) fprintf(stdout, ##__VA_ARGS__);
#else
#define LOG_INFO(...) (void)0;
#endif

// retrieve the idx by extracting the most significant 10 bits in the address
// this means max virtual page number is 2^10 = 1024 = PAGES
// takes the virtual address and zero outs the offsets
// then bit shifts to the right by the length of the offset bits
#define TYPE_SINGLE(vpt, va, ret)                                             \
  singlepagetable *pt = (singlepagetable *)vpt;                               \
  uint16_t idx = (va & 0xffc0) >> 6;                                          \
  if (idx >= PAGES)                                                           \
  {                                                                           \
    LOG_ERROR("[ERROR] page table index cannot be greater than PAGE count\n") \
    return ret;                                                               \
  }

#define TYPE_DOUBLE(vpt, va, ret)                                            \
  doublepagetable *pt = (doublepagetable *)vpt;                              \
  uint16_t outeridx = (virtualaddr & 0xf800) >> 11;                          \
  if (outeridx >= 32)                                                        \
  {                                                                          \
    LOG_ERROR("[ERROR] outer page table index cannot be greater than 32\n")  \
    return ret;                                                              \
  }                                                                          \
  uint16_t inneridx = (virtualaddr & 0x07c0) >> 6;                           \
  if (inneridx >= 32)                                                        \
  {                                                                          \
    LOG_ERROR("[ERROR] inner page table index cannot be greater than 32\n"); \
    return ret;                                                              \
  }

// this function initializes the page entries in level one page table to zero
// all metadata bits are unset
void init_pagetable(singlepagetable *);

// this function initializes the page entries in level two page table to zero
// all metadata bits are unset
void init_dblpagetable(doublepagetable *pt);

singlepagetable *newpagetable(void)
{
  singlepagetable *pt = (singlepagetable *)malloc(sizeof(singlepagetable));
  init_pagetable(pt);
  return pt;
}

void init_pagetable(singlepagetable *pt)
{
  for (int i = 0; i < PAGES; i++)
  {
    pt->entries[i] = (pagetableentry)0;
  }
}

doublepagetable *newdblpagetable(void)
{
  doublepagetable *pt = (doublepagetable *)malloc(sizeof(doublepagetable));
  init_dblpagetable(pt);
  return pt;
}

void init_dblpagetable(doublepagetable *pt)
{
  for (int i = 0; i < 32; i++)
  {
    for (int j = 0; j < 32; j++)
    {
      pt->pagetables[i][j] = (pagetableentry)0;
    }
  }
}

bool update_framenumber(
    enum PAGETABLE type,
    void *vpt,
    const uint16_t virtualaddr,
    const uint16_t framenumber)
{

  if (framenumber > MAX_FRAME_NUMBER)
  {
    LOG_ERROR("[ERROR] max frame number can be only 8191\n")
    return false;
  }

  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    // saves the current state of the metadata bits
    uint16_t curr_state = pt->entries[idx] & 0xe000;
    pt->entries[idx] = curr_state | framenumber;
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    uint16_t curr_state = pt->pagetables[outeridx][inneridx] & 0xe000;
    pt->pagetables[outeridx][inneridx] = curr_state | framenumber;
  }

  return true;
}

uint16_t get_framenumber(
    enum PAGETABLE type,
    const void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, 0);
    LOG_INFO("[INFO] page index is %d\n", idx)
    return (uint16_t)(0x1fff & pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, 0);
    LOG_INFO("[INFO] outer page index: %d, inner index: %d\n", outeridx, inneridx);
    return (uint16_t)(0x1fff & pt->pagetables[outeridx][inneridx]);
  }
}

bool set_validpte(
    enum PAGETABLE type,
    void *vpt,
    const uint16_t virtualaddr)
{

  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    pt->entries[idx] = SET_VALID_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    pt->pagetables[outeridx][inneridx] = SET_VALID_BIT(pt->pagetables[outeridx][inneridx]);
  }
  return true;
}

bool unset_validpte(
    enum PAGETABLE type,
    void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    pt->entries[idx] = UNSET_VALID_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    pt->pagetables[outeridx][inneridx] = UNSET_VALID_BIT(pt->pagetables[outeridx][inneridx]);
  }
  return true;
}

bool set_referencedpte(
    enum PAGETABLE type,
    void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    pt->entries[idx] = SET_REFERENCE_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    pt->pagetables[outeridx][inneridx] = SET_REFERENCE_BIT(pt->pagetables[outeridx][inneridx]);
  }
  return true;
}

bool unset_referencedpte(
    enum PAGETABLE type,
    void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    pt->entries[idx] = UNSET_REFERENCE_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    pt->pagetables[outeridx][inneridx] = UNSET_REFERENCE_BIT(pt->pagetables[outeridx][inneridx]);
  }
  return true;
}

bool set_modifiedpte(
    enum PAGETABLE type,
    void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    pt->entries[idx] = SET_MODIFY_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    pt->pagetables[outeridx][inneridx] = SET_MODIFY_BIT(pt->pagetables[outeridx][inneridx]);
  }
  return true;
}

bool unset_modifiedpte(
    enum PAGETABLE type,
    void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    pt->entries[idx] = UNSET_MODIFY_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    pt->pagetables[outeridx][inneridx] = UNSET_MODIFY_BIT(pt->pagetables[outeridx][inneridx]);
  }
  return true;
}

bool isvalid_pte(
    enum PAGETABLE type,
    const void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    return GET_VALID_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    return GET_VALID_BIT(pt->pagetables[outeridx][inneridx]);
  }
}

bool isreferenced_pte(
    enum PAGETABLE type,
    const void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    return GET_REFERENCE_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    return GET_REFERENCE_BIT(pt->pagetables[outeridx][inneridx]);
  }
}

bool ismodified_pte(
    enum PAGETABLE type,
    const void *vpt,
    const uint16_t virtualaddr)
{
  if (type == ONE_LEVEL)
  {
    TYPE_SINGLE(vpt, virtualaddr, false);
    return GET_MODIFY_BIT(pt->entries[idx]);
  }
  else
  {
    TYPE_DOUBLE(vpt, virtualaddr, false);
    return GET_MODIFY_BIT(pt->pagetables[outeridx][inneridx]);
  }
}