#include <stdio.h>
#include <stdbool.h>
#include "algorithms.h"
#include "pagetable.h"
// include pagetable macros

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

fifo *new_fifo(int fcount)
{
  fifo *pte_fifo = (fifo *)malloc(sizeof(fifo));
  pte_fifo->framecount = fcount;
  pte_fifo->size = 0;
  pte_fifo->head = NULL;
  return pte_fifo;
}

clock *new_clock(int fcount)
{
  clock *pte_clock = (clock *)malloc(sizeof(clock));
  pte_clock->framecount = fcount;
  pte_clock->size = 0;
  pte_clock->head = NULL;
  return pte_clock;
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
    return;
  default:
    return;
  }
}

node *evict_node(ALGO algo, void *structure)
{
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
    node *curr = clocklist->head, *prev = NULL;
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
    return NULL;
  default:
    return NULL;
  }
}