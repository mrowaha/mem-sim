#ifndef FIFO_H
#define FIFO_H

#include <time.h>

#include "pagetable.h"
#include "memsimarg.h"

typedef struct node
{
  pagetableentry *pte;
  page *frame;
  uint16_t virtualaddr;
  struct node *next;
  double lastreferenced;
} node;

typedef struct fifo
{
  int size;
  int framecount;
  node *head;
} fifo;

fifo *new_fifo(int fcount);

void free_fifo(fifo *);

typedef struct sclock
{
  int size;
  int framecount;
  node *head;
} sclock;

sclock *new_sclock(int fcount);

void free_sclock(sclock *);

typedef struct eclock
{
  int size;
  int framecount;
  node *head;
} eclock;

eclock *new_eclock(int fcount);

void free_eclock(eclock *);

typedef struct lru
{
  int size;
  int framecount;
  node *head;
  clock_t start;
} lru;

lru *new_lru(int fcount);

void free_lru(lru *);

void update_referencedtime(lru *, uint16_t virtualaddr);

void insert_pte(ALGO algo, void *structure, pagetableentry *, page *, uint16_t va);

node *evict_node(ALGO algo, void *structure);

#endif