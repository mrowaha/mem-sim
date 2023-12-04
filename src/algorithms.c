#include <stdbool.h>
#include "algorithms.h"

node *insert_pte_fifo(fifo *list, node *newnode)
{
  if (list->head == NULL)
  {
    list->head = newnode;
    list->head->next = NULL;
  }
  else
  {
    bool evict = list->size >= list->framecount;
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
    if (evict)
    {
      node *evictednode = list->head;
      list->head = list->head->next;
      evictednode->next = NULL;
      return evictednode;
    }
  }
  return NULL;
}

fifo *new_fifo(int fcount)
{
  fifo *pte_fifo = (fifo *)malloc(sizeof(fifo));
  pte_fifo->framecount = fcount;
  pte_fifo->size = 0;
  pte_fifo->head = NULL;
  return pte_fifo;
}

node *insert_pte(ALGO algo, void *structure, pagetableentry *pte, page *frame, uint16_t va)
{
  switch (algo)
  {
  case FIFO:
    node *newnode = (node *)malloc(sizeof(node));
    newnode->frame = frame;
    newnode->pte = pte;
    newnode->virtualaddr = va;
    return insert_pte_fifo((fifo *)structure, newnode);
  case LRU:
    return NULL;
  case CLOCK:
    return NULL;
  case ECLOCK:
    return NULL;
  default:
    return NULL;
  }
}

node *to_be_evicted(ALGO algo, void *structure)
{
  switch (algo)
  {
  case FIFO:
    fifo *list = (fifo *)structure;
    return list->head;
  case LRU:
    return NULL;
  case CLOCK:
    return NULL;
  case ECLOCK:
    return NULL;
  default:
    return NULL;
  }
}