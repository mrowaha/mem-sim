#ifndef FIFO_H
#define FIFO_H

#include "pagetable.h"
#include "memsimarg.h"

typedef struct node
{
  pagetableentry *pte;
  page *frame;
  uint16_t virtualaddr;
  struct node *next;
} node;

typedef struct fifo
{
  int size;
  int framecount;
  node *head;
} fifo;

fifo *new_fifo(int fcount);

node *insert_pte(ALGO algo, void *structure, pagetableentry *, page *, uint16_t va);

node *to_be_evicted(ALGO algo, void *structure);

#endif