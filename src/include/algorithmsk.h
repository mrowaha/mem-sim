#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <time.h>

#include "pagetable.h"
#include "memsimarg.h"

typedef struct algonode
{
  pagetableentry *pte_ref;
  uint16_t msb_virtualaddr; // base virtual addr with the offset zeroed out
  uint16_t inmemoryoffset;
  double lastreferenced;
  struct algonode *next;
} algonode;

void init_algonode(
    algonode **node,
    pagetableentry *pte,
    uint16_t msb_va,
    uint16_t mem_offset,
    algonode *next);

void free_algonode(algonode *node);

typedef struct basealgo
{
  PAGETABLE type;
  void *vpt;
  int currsize;
  int maxsize;
  algonode *head;
} basealgo;

typedef basealgo fifo;

typedef basealgo sclock;

typedef basealgo eclock;

fifo *new_fifo(
    PAGETABLE type,
    void *vpt,
    int fcount);

void free_fifo(fifo *);

sclock *new_sclock(
    PAGETABLE type,
    void *vpt,
    int fcount);

void free_sclock(sclock *);

eclock *new_eclock(
    PAGETABLE type,
    void *vpt,
    int fcount);

void free_eclock(eclock *);

typedef struct lru
{
  PAGETABLE type;
  void *vpt;
  int currsize;
  int maxsize;
  algonode *head;
  clock_t start;
} lru;

lru *new_lru(
    PAGETABLE type,
    void *vpt,
    int fcount);

void free_lru(lru *);

void update_referencedtime(lru *, uint16_t virtualaddr);

struct pagefaultresult
{
  uint16_t VA;
  uint16_t PFN;
};

/**
 * PAGE FAULT HANDLER
 */
struct pagefaultresult handlepagefault(
    ALGO algo,
    void *pagereplacer,
    uint16_t virtualaddr,
    page *memory,
    int *currmemorysize,
    swapspace *ss,
    bool ismodified,
    uint8_t writevalue);

void writetopage(
    page *frame,
    uint16_t virtualaddr,
    uint8_t value);

#endif