#include <stdio.h>
#include <stdbool.h>
#include "algorithms.h"
#include "pagetable.h"
// include pagetable macros

void free_node(node *n)
{
  if (n->next)
  {
    free_node(n->next);
  }
  free(n);
}

void insert_pte_fifo(fifo *list, node *newnode)
{
  if (list->head == NULL)
  {
    list->head = newnode;
    list->head->next = NULL;
  }
  else
  {
    // if all the frames have been assigned
    // evict the node at the head
    node *curr = list->head, *prev = NULL;
    while (curr != NULL)
    {
      // append to end
      prev = curr;
      curr = curr->next;
    }
    prev->next = newnode;
    newnode->next = NULL;
    // if evict was flagged, return the head node
    list->size++;
  }
}

void insert_pte_sclock(sclock *list, node *newnode)
{
  if (list->head == NULL)
  {
    list->head = newnode;
    list->head->next = NULL;
  }
  else
  {
    // if all the frames have been assigned
    // evict the node at the head
    node *curr = list->head, *prev = NULL;
    while (curr != NULL)
    {
      // append to end
      prev = curr;
      curr = curr->next;
    }
    prev->next = newnode;
    newnode->next = NULL;
    // if evict was flagged, return the head node
    list->size++;
  }
}

void insert_pte_eclock(eclock *list, node *newnode)
{
  if (list->head == NULL)
  {
    list->head = newnode;
    list->head->next = NULL;
  }
  else
  {
    // if all the frames have been assigned
    // evict the node at the head
    node *curr = list->head, *prev = NULL;
    while (curr != NULL)
    {
      // append to end
      prev = curr;
      curr = curr->next;
    }
    prev->next = newnode;
    newnode->next = NULL;
    // if evict was flagged, return the head node
    list->size++;
  }
}

void insert_pte_lru(lru *list, node *newnode)
{
  printf("here 7.1\n");
  fflush(stdout);
  newnode->lastreferenced = (double)(clock() - list->start);
  if (list->head == NULL)
  {
    list->head = newnode;
    list->head->next = NULL;
  }
  else
  {
    node *curr = list->head, *prev = NULL;
    while (curr != NULL)
    {
      printf("{ va: %#x, next: %p\n }", curr->virtualaddr, (void *)(curr->next));
      prev = curr;
      curr = curr->next;
    }
    prev->next = newnode;
    newnode->next = NULL;
    list->size++;
  }
  printf("here 7.2\n");
  fflush(stdout);
}

fifo *new_fifo(int fcount)
{
  fifo *pte_fifo = (fifo *)malloc(sizeof(fifo));
  pte_fifo->framecount = fcount;
  pte_fifo->size = 0;
  pte_fifo->head = NULL;
  return pte_fifo;
}

void free_fifo(fifo *structure)
{
  if (structure)
  {
    if (structure->head)
    {
      free_node(structure->head);
    }
    free(structure);
  }
}

sclock *new_sclock(int fcount)
{
  sclock *pte_clock = (sclock *)malloc(sizeof(sclock));
  pte_clock->framecount = fcount;
  pte_clock->size = 0;
  pte_clock->head = NULL;
  return pte_clock;
}

void free_sclock(sclock *structure)
{
  if (structure)
  {
    if (structure->head)
    {
      free_node(structure->head);
    }
    free(structure);
  }
}

eclock *new_eclock(int fcount)
{
  eclock *pte_eclock = (eclock *)malloc(sizeof(eclock));
  pte_eclock->framecount = fcount;
  pte_eclock->size = 0;
  pte_eclock->head = NULL;
  return pte_eclock;
}

void free_eclock(eclock *structure)
{
  if (structure)
  {
    if (structure->head)
    {
      free_node(structure->head);
    }
    free(structure);
  }
}

lru *new_lru(int fcount)
{
  lru *pte_lru = (lru *)malloc(sizeof(lru));
  pte_lru->framecount = fcount;
  pte_lru->size = 0;
  pte_lru->head = NULL;
  pte_lru->start = clock();
  return pte_lru;
}

void free_lru(lru *structure)
{
  if (structure)
  {
    if (structure->head)
    {
      free_node(structure->head);
    }
    free(structure);
  }
}

void insert_pte(ALGO algo, void *structure, pagetableentry *pte, page *frame, uint16_t va)
{
  node *newnode;
  newnode = (node *)malloc(sizeof(node));
  newnode->frame = frame;
  newnode->pte = pte;
  newnode->virtualaddr = va;
  switch (algo)
  {
  case FIFO:
    insert_pte_fifo((fifo *)structure, newnode);
    return;
  case LRU:
    printf("here 7\n");
    fflush(stdout);
    insert_pte_lru((lru *)structure, newnode);
    printf("here 8\n");
    fflush(stdout);
    return;
  case CLOCK:
    insert_pte_sclock((sclock *)structure, newnode);
    return;
  case ECLOCK:
    insert_pte_eclock((eclock *)structure, newnode);
    return;
  default:
    return;
  }
}

node *evict_node(ALGO algo, void *structure)
{
  node *curr, *prev;
  switch (algo)
  {
  case FIFO:
    fifo *fifolist = (fifo *)structure;
    node *evictednode = fifolist->head;
    fifolist->head = fifolist->head->next;
    evictednode->next = NULL;
    return evictednode;
  case LRU:
    lru *lrulist = (lru *)structure;
    curr = lrulist->head;
    prev = NULL;
    node *leastrecentlyused = curr;
    while (curr != NULL)
    {
      prev = curr;
      curr = curr->next;
      if (curr != NULL && leastrecentlyused->lastreferenced < curr->lastreferenced)
      {
        leastrecentlyused = curr;
      }
    }
    if (leastrecentlyused == lrulist->head)
    {
      lrulist->head = lrulist->head->next;
    }
    else
    {
      prev->next = leastrecentlyused->next;
    }
    return leastrecentlyused;
  case CLOCK:
    sclock *clocklist = (sclock *)structure;
    // cycle through the underlying fifo and give each a second chance
    curr = clocklist->head;
    prev = NULL;
    while ((bool)(GET_REFERENCE_BIT((*(curr->pte)))))
    {
      // give each a second chance
      *(curr->pte) = UNSET_REFERENCE_BIT((*(curr->pte)));
      prev = curr;
      curr = curr->next;
      if (curr == NULL)
      {
        // cycle to head
        curr = clocklist->head;
        prev = NULL;
      }
    }
    // remove the curr
    if (prev == NULL)
    {
      clocklist->head = curr->next;
    }
    else
    {
      prev->next = curr->next;
    }
    return curr;
  case ECLOCK:
    eclock *eclocklist = (eclock *)structure;
    int step = 1;
    bool found = false;
  cycle:
    curr = eclocklist->head;
    prev = NULL;
    found = false;
    while (curr != NULL)
    {
      bool isreferenced = (bool)(GET_REFERENCE_BIT((*(curr->pte))));
      bool ismodified = (bool)(GET_MODIFY_BIT((*(curr->pte))));
      if (step == 1 || step == 3)
      {
        // look for <0,0> but do not reset the reference bits
        if (!isreferenced && !ismodified)
        {
          found = true;
          break;
        }
      }
      else if (step == 2 || step == 4)
      {
        // look for <0,1>, if step is 2 reset the reference bits
        if (!isreferenced && ismodified)
        {
          found = true;
          break;
        }
        if (step == 2)
        {
          *(curr->pte) = UNSET_REFERENCE_BIT((*(curr->pte)));
        }
      }
      prev = curr;
      curr = curr->next;
    }
    step++;
    if (step != 5 && !found)
      goto cycle;
    // according to the alogrithm, an evicted node is guaranteed
    if (prev == NULL)
    {
      eclocklist->head = curr->next;
    }
    else
    {
      prev->next = curr->next;
    }
    return curr;
  default:
    return NULL;
  }
}

void update_referencedtime(lru *list, uint16_t virtualaddr)
{
  node *curr = list->head;
  while (curr != NULL)
  {
    if (curr->virtualaddr == virtualaddr)
    {
      curr->lastreferenced = (double)(clock() - list->start);
      return;
    }
    curr = curr->next;
  }
}
