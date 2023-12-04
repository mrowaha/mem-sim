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

void insert_pte_clock(clock *list, node *newnode)
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

clock *new_clock(int fcount)
{
  clock *pte_clock = (clock *)malloc(sizeof(clock));
  pte_clock->framecount = fcount;
  pte_clock->size = 0;
  pte_clock->head = NULL;
  return pte_clock;
}

void free_clock(clock *structure)
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

void insert_pte(ALGO algo, void *structure, pagetableentry *pte, page *frame, uint16_t va)
{
  node *newnode;
  switch (algo)
  {
  case FIFO:
    newnode = (node *)malloc(sizeof(node));
    newnode->frame = frame;
    newnode->pte = pte;
    newnode->virtualaddr = va;
    insert_pte_fifo((fifo *)structure, newnode);
    return;
  case LRU:
    return;
  case CLOCK:
    newnode = (node *)malloc(sizeof(node));
    newnode->frame = frame;
    newnode->pte = pte;
    newnode->virtualaddr = va;
    insert_pte_clock((clock *)structure, newnode);
    return;
  case ECLOCK:
    newnode = (node *)malloc(sizeof(node));
    newnode->frame = frame;
    newnode->pte = pte;
    newnode->virtualaddr = va;
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
    return NULL;
  case CLOCK:
    clock *clocklist = (clock *)structure;
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
  cycle:
    curr = eclocklist->head;
    prev = NULL;
    while (curr != NULL)
    {
      bool isreferenced = (bool)(GET_REFERENCE_BIT((*(curr->pte))));
      bool ismodified = (bool)(GET_MODIFY_BIT((*(curr->pte))));
      if (step == 1 || step == 3)
      {
        // look for <0,0> but do not reset the reference bits
        if (!isreferenced && !ismodified)
        {
          break;
        }
      }
      else if (step == 2 || step == 4)
      {
        // look for <0,1>, if step is 2 reset the reference bits
        if (!isreferenced && ismodified)
        {
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
    if (step < 4)
    {
      step++;
      goto cycle;
    }
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