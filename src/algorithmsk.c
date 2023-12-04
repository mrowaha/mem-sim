#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "algorithmsk.h"
#include "swapspace.h"
#include "pagetable.h"

#define SS_PAGEIDX(va) (va & 0xffc0) >> 6

#define MEMSIM_ASSERTIONS

struct pagereplacement
{
  algonode *evictednode;
  uint16_t inmemoryoffset;
};

/**
 * Below are small utilty functions forward declarations
 */

// onloadpage: sets the corresponding metabits for a page table entry when performing page in
void onloadpage(
    void *vpt,
    PAGETABLE type,
    uint16_t virtualaddr,
    bool ismodified);
// writetopage: writes the value to the offset of the corresponding in-memory frame
void writetopage(
    page *frame,
    uint16_t virtualaddr,
    uint8_t value);

// onunloadpage: call this function on page out
void onunloadpage(
    ALGO algo,
    void *pagereplacer,
    uint16_t virtualaddr,
    page *frame,
    swapspace *ss);

/**
 * Below are forward declarations for each page replacement algorithm
 */
struct pagereplacement run_pagereplacement(ALGO algo, void *pagereplacer, uint16_t virtualaddr, pagetableentry *pte_ref);
struct pagereplacement run_pagereplacement_fifo(fifo *fifolist, uint16_t virtualaddr, pagetableentry *pte_ref);
struct pagereplacement run_pagereplacement_sclock(sclock *sclocklist, uint16_t virtualaddr, pagetableentry *pte_ref);
struct pagereplacement run_pagereplacement_eclock(eclock *eclocklist, uint16_t virtualaddr, pagetableentry *pte_ref);
struct pagereplacement run_pagereplacement_lru(lru *lrulist, uint16_t virtualaddr, pagetableentry *pte_ref);

struct pagefaultresult handlepagefault(
    ALGO algo,
    void *pagereplacer,
    uint16_t virtualaddr,
    page *memory,
    int *currmemorysize,
    swapspace *ss,
    bool ismodified,
    uint8_t writevalue)
{
  // handle page fault using the fifo page replacement algorithm

  // extract the underlying page from swapspace
  uint16_t pageidx = SS_PAGEIDX(virtualaddr);
#ifdef MEMSIM_ASSERTIONS
  assert(pageidx <= (uint16_t)1023);
#endif
  page *pagedin_page = get_pagecpy(ss, pageidx);
  pagetableentry *pte_ref = NULL;
  switch (algo)
  {
  case FIFO:
  case CLOCK:
  case ECLOCK:
    basealgo *list = (basealgo *)pagereplacer;
    pte_ref = get_pte_reference(list->type, list->vpt, virtualaddr);
    break;
  case LRU:
    lru *lrulist = (lru *)pagereplacer;
    pte_ref = get_pte_reference(lrulist->type, lrulist->vpt, virtualaddr);
    break;
  }

  struct pagereplacement result = run_pagereplacement(algo, pagereplacer, virtualaddr, pte_ref);
#ifdef MEMSIM_ASSERTIONS
  switch (algo)
  {
  case FIFO:
  case CLOCK:
  case ECLOCK:
    basealgo *list = (basealgo *)pagereplacer;
    if ((*currmemorysize) < list->maxsize)
    {
      // there should be no eviction performed
      assert(result.evictednode == NULL);
    }
    break;
  case LRU:
    lru *lrulist = (lru *)pagereplacer;
    if ((*currmemorysize) < lrulist->maxsize)
    {
      // there should be no eviction performed
      assert(result.evictednode == NULL);
    }
  }
#endif
  if (result.evictednode == NULL)
  {
    // there will be no eviction if there is space in memory,
    // increment currmemorysize
    *currmemorysize = (*currmemorysize) + 1;
  }
  else
  {
    // there was a page eviction
    // call the onunloadpage listener
    page *evictedframe = memory + result.inmemoryoffset;
    onunloadpage(
        algo,
        pagereplacer,
        result.evictednode->msb_virtualaddr,
        evictedframe,
        ss);
#ifdef MEMSIM_ASSERTIONS
    assert(result.evictednode->next == NULL);
#endif
    free_algonode(result.evictednode);
  }

#ifdef MEMSIM_ASSERTIONS
  switch (algo)
  {
  case FIFO:
  case CLOCK:
  case ECLOCK:
    basealgo *list = (basealgo *)pagereplacer;
    assert((*currmemorysize) <= list->maxsize);
    break;
  case LRU:
    lru *lrulist = (lru *)pagereplacer;
    assert((*currmemorysize) <= lrulist->maxsize);
    break;
  }

#endif

  // the page replacement result will hold the underlying in-memory offsets
  memcpy(memory + result.inmemoryoffset, pagedin_page, sizeof(page));
  // update the frame number for the paged-in page entry
  switch (algo)
  {
  case FIFO:
  case CLOCK:
  case ECLOCK:
    basealgo *list = (basealgo *)pagereplacer;
    update_framenumber(
        list->type,
        list->vpt,
        virtualaddr,
        result.inmemoryoffset);
    // set the corresponding metabits in the pte
    onloadpage(
        list->vpt,
        list->type,
        virtualaddr,
        ismodified);
    break;
  case LRU:
    lru *lrulist = (lru *)pagereplacer;
    update_framenumber(
        lrulist->type,
        lrulist->vpt,
        virtualaddr,
        result.inmemoryoffset);
    // set the corresponding metabits in the pte
    onloadpage(
        lrulist->vpt,
        lrulist->type,
        virtualaddr,
        ismodified);
    break;
  }

  if (ismodified)
  {
    writetopage(memory + result.inmemoryoffset, virtualaddr, writevalue);
  }

  free(pagedin_page);
  struct pagefaultresult updateresult = {
      .VA = virtualaddr,
      .PFN = result.inmemoryoffset};
  return updateresult;
}

/**
 * Algorithm-wise eviction selection
 */
struct pagereplacement run_pagereplacement(ALGO algo, void *pagereplacer, uint16_t virtualaddr, pagetableentry *pte_ref)
{
  switch (algo)
  {
  case FIFO:
    fifo *fifolist = (fifo *)pagereplacer;
    return run_pagereplacement_fifo(fifolist, virtualaddr, pte_ref);
  case CLOCK:
    sclock *sclocklist = (sclock *)pagereplacer;
    return run_pagereplacement_sclock(sclocklist, virtualaddr, pte_ref);
  case ECLOCK:
    eclock *eclocklist = (eclock *)pagereplacer;
    return run_pagereplacement_eclock(eclocklist, virtualaddr, pte_ref);
  case LRU:
    lru *lrulist = (lru *)pagereplacer;
    return run_pagereplacement_lru(lrulist, virtualaddr, pte_ref);
  }
}

struct pagereplacement run_pagereplacement_fifo(fifo *fifolist, uint16_t virtualaddr, pagetableentry *pte_ref)
{
  struct pagereplacement returnedstruct = {.evictednode = NULL, .inmemoryoffset = -1};
  if (fifolist->head == NULL)
  {
#ifdef MEMSIM_ASSERTIONS
    assert(fifolist->currsize == 0);
#endif
    fifolist->head = (algonode *)malloc(sizeof(algonode));
    init_algonode(&(fifolist->head), pte_ref, (virtualaddr & 0xffc0), fifolist->currsize, NULL);
    returnedstruct.inmemoryoffset = fifolist->currsize;
  }
  else
  {
    algonode *tobe_pagedin = (algonode *)malloc(sizeof(algonode));
    // perform an append first
    algonode *curr = fifolist->head, *prev = NULL;
    while (curr != NULL)
    {
      prev = curr;
      curr = curr->next;
    }
    prev->next = tobe_pagedin;
    // requires a page eviction if the memory is already full
    // maintain reference to the underlying eviction entry
    algonode *tobe_evicted = NULL;
    bool evict = fifolist->currsize >= fifolist->maxsize;
    if (evict)
    {
      tobe_evicted = fifolist->head;
#ifdef MEMSIM_ASSERTIONS
      assert(tobe_evicted != NULL);
#endif
      init_algonode(
          &tobe_pagedin,
          pte_ref,
          (virtualaddr & 0xffc0),
          tobe_evicted->inmemoryoffset,
          NULL);

      // advance the list head
      fifolist->head = fifolist->head->next;
      // cut of the evicted node
      tobe_evicted->next = NULL;
      returnedstruct.inmemoryoffset = tobe_evicted->inmemoryoffset;
      returnedstruct.evictednode = tobe_evicted;
    }
    else
    {
      init_algonode(
          &tobe_pagedin,
          pte_ref,
          (virtualaddr & 0xffc0),
          fifolist->currsize,
          NULL);
      returnedstruct.inmemoryoffset = fifolist->currsize;
      returnedstruct.evictednode = NULL;
    }
  }

  if (fifolist->currsize != fifolist->maxsize)
    fifolist->currsize++;

#ifdef MEMSIM_ASSERTIONS
  assert(returnedstruct.inmemoryoffset != -1);
  if (returnedstruct.evictednode == NULL)
    assert(returnedstruct.inmemoryoffset < fifolist->maxsize);
#endif
  return returnedstruct;
}

struct pagereplacement run_pagereplacement_sclock(sclock *sclocklist, uint16_t virtualaddr, pagetableentry *pte_ref)
{
  struct pagereplacement returnedstruct = {.evictednode = NULL, .inmemoryoffset = -1};
  if (sclocklist->head == NULL)
  {
#ifdef MEMSIM_ASSERTIONS
    assert(sclocklist->currsize == 0);
#endif
    sclocklist->head = (algonode *)malloc(sizeof(algonode));
    init_algonode(&(sclocklist->head), pte_ref, (virtualaddr & 0xffc0), sclocklist->currsize, NULL);
    returnedstruct.inmemoryoffset = sclocklist->currsize;
  }
  else
  {
    algonode *tobe_pagedin = (algonode *)malloc(sizeof(algonode));
    // perform an append first
    algonode *curr = sclocklist->head, *prev = NULL;
    while (curr != NULL)
    {
      prev = curr;
      curr = curr->next;
    }
    prev->next = tobe_pagedin;
    // requires a page eviction if the memory is already full
    // maintain reference to the underlying eviction entry
    algonode *tobe_evicted = NULL;
    bool evict = sclocklist->currsize >= sclocklist->maxsize;
    if (evict)
    {
      algonode *innercurr, *innerprev;
      innercurr = sclocklist->head;
      innerprev = NULL;
      while (isreferenced_pte(sclocklist->type, sclocklist->vpt, innercurr->msb_virtualaddr))
      {
        // give each a second chance
        unset_referencedpte(sclocklist->type, sclocklist->vpt, innercurr->msb_virtualaddr);
        innerprev = innercurr;
        innercurr = innercurr->next;
        if (innercurr == NULL)
        {
          // cycle to head
          innercurr = sclocklist->head;
          innerprev = NULL;
        }
      }
      // remove the curr
      if (innerprev == NULL)
      {
        sclocklist->head = innercurr->next;
      }
      else
      {
        innerprev->next = innercurr->next;
      }

      tobe_evicted = innercurr;

#ifdef MEMSIM_ASSERTIONS
      assert(tobe_evicted != NULL);
#endif
      init_algonode(
          &tobe_pagedin,
          pte_ref,
          (virtualaddr & 0xffc0),
          tobe_evicted->inmemoryoffset,
          NULL);

      // cut of the evicted node
      tobe_evicted->next = NULL;
      returnedstruct.inmemoryoffset = tobe_evicted->inmemoryoffset;
      returnedstruct.evictednode = tobe_evicted;
    }
    else
    {
      init_algonode(
          &tobe_pagedin,
          pte_ref,
          (virtualaddr & 0xffc0),
          sclocklist->currsize,
          NULL);
      returnedstruct.inmemoryoffset = sclocklist->currsize;
      returnedstruct.evictednode = NULL;
    }
  }

  if (sclocklist->currsize != sclocklist->maxsize)
    sclocklist->currsize++;

#ifdef MEMSIM_ASSERTIONS
  assert(returnedstruct.inmemoryoffset != -1);
  if (returnedstruct.evictednode == NULL)
    assert(returnedstruct.inmemoryoffset < sclocklist->maxsize);
#endif
  return returnedstruct;
}

struct pagereplacement run_pagereplacement_eclock(eclock *eclocklist, uint16_t virtualaddr, pagetableentry *pte_ref)
{
  struct pagereplacement returnedstruct = {.evictednode = NULL, .inmemoryoffset = -1};
  if (eclocklist->head == NULL)
  {
#ifdef MEMSIM_ASSERTIONS
    assert(eclocklist->currsize == 0);
#endif
    eclocklist->head = (algonode *)malloc(sizeof(algonode));
    init_algonode(&(eclocklist->head), pte_ref, (virtualaddr & 0xffc0), eclocklist->currsize, NULL);
    returnedstruct.inmemoryoffset = eclocklist->currsize;
  }
  else
  {
    algonode *tobe_pagedin = (algonode *)malloc(sizeof(algonode));
    // perform an append first
    algonode *curr = eclocklist->head, *prev = NULL;
    while (curr != NULL)
    {
      prev = curr;
      curr = curr->next;
    }
    prev->next = tobe_pagedin;
    // requires a page eviction if the memory is already full
    // maintain reference to the underlying eviction entry
    algonode *tobe_evicted = NULL;
    bool evict = eclocklist->currsize >= eclocklist->maxsize;
    if (evict)
    {
      algonode *innercurr, *innerprev;
      int step = 1;
      bool found = false;
    cycle:
      innercurr = eclocklist->head;
      innerprev = NULL;
      while (innercurr != NULL)
      {
        bool isreferenced = isreferenced_pte(eclocklist->type, eclocklist->vpt, innercurr->msb_virtualaddr);
        bool ismodified = ismodified_pte(eclocklist->type, eclocklist->vpt, innercurr->msb_virtualaddr);
        if (step == 1 || step == 3)
        {
          if (!isreferenced && !ismodified)
          {
            found = true;
            break;
          }
        }
        else if (step == 2 || step == 4)
        {
          if (!isreferenced && ismodified)
          {
            found = true;
            break;
          }
          if (step == 2)
          {
            unset_referencedpte(eclocklist->type, eclocklist->head, innercurr->msb_virtualaddr);
          }
        }
        innerprev = innercurr;
        innercurr = innercurr->next;
      }
      step++;
      if (step != 5 && !found)
      {
        goto cycle;
      }

      // remove the curr
      if (innerprev == NULL)
      {
        eclocklist->head = innercurr->next;
      }
      else
      {
        innerprev->next = innercurr->next;
      }

      tobe_evicted = innercurr;

#ifdef MEMSIM_ASSERTIONS
      assert(tobe_evicted != NULL);
#endif
      init_algonode(
          &tobe_pagedin,
          pte_ref,
          (virtualaddr & 0xffc0),
          tobe_evicted->inmemoryoffset,
          NULL);

      // cut of the evicted node
      tobe_evicted->next = NULL;
      returnedstruct.inmemoryoffset = tobe_evicted->inmemoryoffset;
      returnedstruct.evictednode = tobe_evicted;
    }
    else
    {
      init_algonode(
          &tobe_pagedin,
          pte_ref,
          (virtualaddr & 0xffc0),
          eclocklist->currsize,
          NULL);
      returnedstruct.inmemoryoffset = eclocklist->currsize;
      returnedstruct.evictednode = NULL;
    }
  }

  if (eclocklist->currsize != eclocklist->maxsize)
    eclocklist->currsize++;

#ifdef MEMSIM_ASSERTIONS
  assert(returnedstruct.inmemoryoffset != -1);
  if (returnedstruct.evictednode == NULL)
    assert(returnedstruct.inmemoryoffset < eclocklist->maxsize);
#endif
  return returnedstruct;
}

struct pagereplacement run_pagereplacement_lru(lru *lrulist, uint16_t virtualaddr, pagetableentry *pte_ref)
{
  struct pagereplacement returnedstruct = {.evictednode = NULL, .inmemoryoffset = -1};
  if (lrulist->head == NULL)
  {
#ifdef MEMSIM_ASSERTIONS
    assert(lrulist->currsize == 0);
#endif
    lrulist->head = (algonode *)malloc(sizeof(algonode));
    init_algonode(&(lrulist->head), pte_ref, (virtualaddr & 0xffc0), lrulist->currsize, NULL);
    returnedstruct.inmemoryoffset = lrulist->currsize;
  }
  else
  {
    algonode *tobe_pagedin = (algonode *)malloc(sizeof(algonode));
    // perform an append first
    algonode *curr = lrulist->head, *prev = NULL;
    while (curr != NULL)
    {
      prev = curr;
      curr = curr->next;
    }
    prev->next = tobe_pagedin;
    // requires a page eviction if the memory is already full
    // maintain reference to the underlying eviction entry
    algonode *tobe_evicted = NULL;
    bool evict = lrulist->currsize >= lrulist->maxsize;
    if (evict)
    {
      algonode *innercurr, *innerprev;
      int step = 1;
      bool found = false;
    cycle:
      innercurr = lrulist->head;
      innerprev = NULL;
      while (innercurr != NULL)
      {
        bool isreferenced = isreferenced_pte(lrulist->type, lrulist->vpt, innercurr->msb_virtualaddr);
        bool ismodified = ismodified_pte(lrulist->type, lrulist->vpt, innercurr->msb_virtualaddr);
        if (step == 1 || step == 3)
        {
          if (!isreferenced && !ismodified)
          {
            found = true;
            break;
          }
        }
        else if (step == 2 || step == 4)
        {
          if (!isreferenced && ismodified)
          {
            found = true;
            break;
          }
          if (step == 2)
          {
            unset_referencedpte(lrulist->type, lrulist->head, innercurr->msb_virtualaddr);
          }
        }
        innerprev = innercurr;
        innercurr = innercurr->next;
      }
      step++;
      if (step != 5 && !found)
      {
        goto cycle;
      }

      // remove the curr
      if (innerprev == NULL)
      {
        lrulist->head = innercurr->next;
      }
      else
      {
        innerprev->next = innercurr->next;
      }

      tobe_evicted = innercurr;

#ifdef MEMSIM_ASSERTIONS
      assert(tobe_evicted != NULL);
#endif
      init_algonode(
          &tobe_pagedin,
          pte_ref,
          (virtualaddr & 0xffc0),
          tobe_evicted->inmemoryoffset,
          NULL);

      // cut of the evicted node
      tobe_evicted->next = NULL;
      returnedstruct.inmemoryoffset = tobe_evicted->inmemoryoffset;
      returnedstruct.evictednode = tobe_evicted;
    }
    else
    {
      init_algonode(
          &tobe_pagedin,
          pte_ref,
          (virtualaddr & 0xffc0),
          lrulist->currsize,
          NULL);
      returnedstruct.inmemoryoffset = lrulist->currsize;
      returnedstruct.evictednode = NULL;
    }
  }

  if (lrulist->currsize != lrulist->maxsize)
    lrulist->currsize++;

#ifdef MEMSIM_ASSERTIONS
  assert(returnedstruct.inmemoryoffset != -1);
  if (returnedstruct.evictednode == NULL)
    assert(returnedstruct.inmemoryoffset < lrulist->maxsize);
#endif
  return returnedstruct;
}

void onloadpage(
    void *vpt,
    PAGETABLE type,
    uint16_t virtualaddr,
    bool ismodified)
{
  // this utility function sets the metabits
  // for the loaded page
  // it sets the valid bit to one
  // it sets the referenced bit to one
  // if the page was for write, sets the modified bit to one
  set_validpte(type, vpt, virtualaddr);
  set_referencedpte(type, vpt, virtualaddr);
  if (ismodified)
  {
    set_modifiedpte(type, vpt, virtualaddr);
  }
}

void writetopage(
    page *frame,
    uint16_t virtualaddr,
    uint8_t value)
{
  uint16_t offset = virtualaddr & 0x003f;
  frame->content[offset] = value;
}

void onunloadpage(
    ALGO algo,
    void *pagereplacer,
    uint16_t virtualaddr,
    page *frame,
    swapspace *ss)
{
  PAGETABLE type;
  void *vpt;
  switch (algo)
  {
  case FIFO:
  case CLOCK:
  case ECLOCK:
    basealgo *list = (basealgo *)pagereplacer;
    type = list->type;
    vpt = list->vpt;
    break;
  case LRU:
    lru *lrulist = (lru *)pagereplacer;
    type = lrulist->type;
    vpt = lrulist->vpt;
  }

  uint16_t pageidx = SS_PAGEIDX(virtualaddr);

  if (ismodified_pte(type, vpt, virtualaddr))
  {
    write_page(ss, pageidx, frame);
  }

  // clear all the metabits
  unset_validpte(type, vpt, virtualaddr);
  unset_referencedpte(type, vpt, virtualaddr);
  unset_modifiedpte(type, vpt, virtualaddr);
}

/**
 * Algorithm Specific Constructors and Destructors
 */
fifo *new_fifo(
    PAGETABLE type,
    void *vpt,
    int fcount)
{
  fifo *newfifo = (fifo *)malloc(sizeof(fifo));
  newfifo->head = NULL;
  newfifo->currsize = 0;
  newfifo->maxsize = fcount;
  newfifo->type = type;
  newfifo->vpt = vpt;
  return newfifo;
}

void free_fifo(fifo *fifolist)
{
  if (fifolist)
  {
    free_algonode(fifolist->head);
    free(fifolist);
  }
}

sclock *new_sclock(
    PAGETABLE type,
    void *vpt,
    int fcount)
{
  sclock *newsclock = (sclock *)malloc(sizeof(sclock));
  newsclock->head = NULL;
  newsclock->currsize = 0;
  newsclock->maxsize = fcount;
  newsclock->type = type;
  newsclock->vpt = vpt;
  return newsclock;
}

void free_sclock(sclock *sclocklist)
{
  if (sclocklist)
  {
    free_algonode(sclocklist->head);
    free(sclocklist);
  }
}

eclock *new_eclock(
    PAGETABLE type,
    void *vpt,
    int fcount)
{
  eclock *neweclock = (eclock *)malloc(sizeof(eclock));
  neweclock->head = NULL;
  neweclock->currsize = 0;
  neweclock->maxsize = fcount;
  neweclock->type = type;
  neweclock->vpt = vpt;
  return neweclock;
}

void free_eclock(eclock *eclocklist)
{
  if (eclocklist)
  {
    free_algonode(eclocklist->head);
    free(eclocklist);
  }
}

lru *new_lru(
    PAGETABLE type,
    void *vpt,
    int fcount)
{
  lru *newlru = (lru *)malloc(sizeof(lru));
  newlru->start = clock();
  newlru->head = NULL;
  newlru->currsize = 0;
  newlru->maxsize = fcount;
  newlru->type = type;
  newlru->vpt = vpt;
  return newlru;
}

void free_lru(lru *lrulist)
{
  if (lrulist)
  {
    free_algonode(lrulist->head);
    free(lrulist);
  }
}

void update_referencedtime(lru *list, uint16_t virtualaddr)
{
  algonode *curr = list->head;
  uint16_t msb_virtualaddr = (virtualaddr & 0xffc0);
  while (curr != NULL)
  {
    if (curr->msb_virtualaddr == msb_virtualaddr)
    {
      curr->lastreferenced = (double)(clock() - list->start);
      return;
    }
    curr = curr->next;
  }
}

void init_algonode(
    algonode **node,
    pagetableentry *pte,
    uint16_t msb_va,
    uint16_t mem_offset,
    algonode *next)
{
#ifdef MEMSIM_ASSERTIONS
  assert(node != NULL);
#endif
  (*node)->pte_ref = pte;
  (*node)->msb_virtualaddr = msb_va;
  (*node)->inmemoryoffset = mem_offset;
  (*node)->next = next;
}

void free_algonode(algonode *node)
{
  if (node)
  {
    if (node->next)
    {
      free_algonode(node);
    }
    free(node);
  }
}