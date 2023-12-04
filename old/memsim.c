#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "algorithms.h"
#include "memsim.h"

void write_log(
    memsim *simulator,
    uint16_t VA,
    uint16_t PFN,
    bool pgfault)
{
  FILE *outfile = simulator->outfile;
  char log[100];
  uint16_t PTE1, PTE2, OFFSET;
  if (simulator->type == ONE_LEVEL)
  {
    PTE1 = (VA & 0x07c0) >> 6;
    PTE2 = 0;
  }
  else
  {
    PTE1 = (VA & 0xffc0) >> 6;
    PTE2 = (VA & 0xf800) >> 11;
  }
  OFFSET = VA & 0x003f;
  uint16_t PA = (PFN << 6) | OFFSET;
  char *PF = pgfault ? "pgfault" : " ";
  sprintf(log, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x %s\n", VA, PTE1, PTE2, OFFSET, PFN, PA, PF);
  printf("%s", log);
  fflush(stdout);
  fprintf(outfile, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x %s\n", VA, PTE1, PTE2, OFFSET, PFN, PA, PF);
}

bool str_to_uint16(const char *str, uint16_t *res)
{
  char *end;
  errno = 0;
  long val = strtol(str, &end, 16);
  *res = (uint16_t)val;
  return true;
}

bool str_to_uint8(const char *str, uint8_t *res)
{
  if (str == NULL)
  {
    *res = (uint8_t)0;
    return true;
  }
  char *end;
  errno = 0;
  long val = strtol(str, &end, 0);
  *res = (uint8_t)val;
  return true;
}

memsim *new_memsim(
    int level,
    int fcount,
    char *swapfile,
    char *outfile,
    ALGO algo)
{
  memsim *simulator = (memsim *)malloc(sizeof(memsim));

  simulator->algo = algo;
  switch (algo)
  {
  case FIFO:
    simulator->structure = (void *)new_fifo(fcount);
    break;
  case CLOCK:
    simulator->structure = (void *)new_sclock(fcount);
    break;
  case ECLOCK:
    simulator->structure = (void *)new_eclock(fcount);
    break;
  case LRU:
    simulator->structure = (void *)new_lru(fcount);
    break;
  default:
    fprintf(stderr, "[ERROR] invalid algorithm type: %d\n", algo);
    free(simulator);
    return NULL;
  }

  newswapspace newss = new_swapspace(swapfile);
  if (newss.isnew)
  {
    if (validate_newswapspace(newss.ss))
    {
      simulator->ss = newss.ss;
    }
    else
    {
      free_swapspace(&(newss.ss));
      free(simulator);
      return NULL;
    }
  }
  else
  {
    // assign without validation
    simulator->ss = newss.ss;
  }

  simulator->outfile = fopen(outfile, "w");
  if (simulator->outfile == NULL)
  {
    fprintf(stderr, "[ERROR] failed to open outfile for write\n");
    free_swapspace(&(newss.ss));
    free(simulator);
    return NULL;
  }

  if (level == 1)
  {
    // initialize a ONE_LEVEL page table
    simulator->pagetable = (void *)newpagetable();
    simulator->type = ONE_LEVEL;
  }
  else
  {
    // else initialize to a TWO_LEVEL page table
    simulator->pagetable = (void *)newdblpagetable();
    simulator->type = TWO_LEVEL;
  }

  // initialize memory to fcount and all frames zeroed out
  simulator->framecount = fcount;
  simulator->memory = (page *)malloc(sizeof(page) * fcount);
  for (int i = 0; i < fcount; i++)
  {
    simulator->memory[i] = new_page();
  }

  simulator->curr = 0;
  return simulator;
}

void read_source(memsim *simulator, const char *inputfile, const int tick)
{

  FILE *infile = fopen(inputfile, "r");
  if (infile == NULL)
  {
    perror("read_source");
    return;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char *token = NULL; /* 'w' for write or 'r' for read */

  int memoryreferences = 0;
  int pagefaults = 0;
  while ((read = getline(&line, &len, infile)) != -1)
  {
    token = strtok(line, " ");
    if (strcmp(token, "w") == 0)
    {
      // is a write operation
      // if the page is valid, simply set its reference bit
      uint16_t virtualaddr;
      token = strtok(NULL, " ");
      if (!str_to_uint16(token, &virtualaddr))
      {
        fprintf(stderr, "[ERROR] failed to parse virtual address to uint16_t\n");
        break;
      }
      uint8_t value;
      token = strtok(NULL, " ");
      if (!str_to_uint8(token, &value))
      {
        fprintf(stderr, "[ERROR] failed to parse value to uint8_t\n");
        break;
      }

      if (isvalid_pte(simulator->type, simulator->pagetable, virtualaddr))
      {
        set_referencedpte(simulator->type, simulator->pagetable, virtualaddr);
        set_modifiedpte(simulator->type, simulator->pagetable, virtualaddr);
        uint16_t offset = virtualaddr & 0x003f;
        uint16_t framenumber = get_framenumber(simulator->type, simulator->pagetable, virtualaddr);
        page *frame = simulator->memory + framenumber;
        frame->content[offset] = value;
        write_log(simulator, virtualaddr, framenumber, false);
        if (simulator->algo == LRU)
        {
          update_referencedtime((lru *)simulator->structure, virtualaddr);
        }
      }
      else
      {
        pagefaults++;
        uint16_t page_idx = (virtualaddr & 0xffc0) >> 6;
        page *pagedin_page = get_pagecpy(simulator->ss, page_idx);
        pagetableentry *pte_ref = get_pte_reference(simulator->type, simulator->pagetable, virtualaddr);

        if (simulator->curr < simulator->framecount)
        {
          memcpy(simulator->memory + simulator->curr, pagedin_page, sizeof(page));
          update_framenumber(simulator->type, simulator->pagetable, virtualaddr, simulator->curr);
          set_validpte(simulator->type, simulator->pagetable, virtualaddr);
          set_referencedpte(simulator->type, simulator->pagetable, virtualaddr);
          set_modifiedpte(simulator->type, simulator->pagetable, virtualaddr);
          uint16_t offset = virtualaddr & 0x003f;
          page *frame = simulator->memory + simulator->curr;
          frame->content[offset] = value; // and write

          insert_pte(
              simulator->algo,
              simulator->structure,
              pte_ref,
              simulator->memory + simulator->curr,
              virtualaddr);
          write_log(simulator, virtualaddr, simulator->curr, true);
          simulator->curr++;
        }
        else
        {
          node *evicted_page = evict_node(
              simulator->algo,
              simulator->structure);
          uint16_t framenumber = get_framenumber(simulator->type, simulator->pagetable, evicted_page->virtualaddr);
          insert_pte(
              simulator->algo,
              simulator->structure,
              pte_ref,
              simulator->memory + framenumber,
              virtualaddr);
          unset_validpte(simulator->type, simulator->pagetable, evicted_page->virtualaddr);
          unset_referencedpte(simulator->type, simulator->pagetable, evicted_page->virtualaddr);
          if (ismodified_pte(simulator->type, simulator->pagetable, evicted_page->virtualaddr))
          {
            // overwrite the corresponding page in the swapspace
            write_page(simulator->ss, page_idx, evicted_page->frame);
          }
          unset_modifiedpte(simulator->type, simulator->pagetable, evicted_page->virtualaddr);

          // page in from the swap store
          memcpy(simulator->memory + framenumber, pagedin_page, sizeof(page));
          update_framenumber(simulator->type, simulator->pagetable, virtualaddr, framenumber);
          set_validpte(simulator->type, simulator->pagetable, virtualaddr);
          set_referencedpte(simulator->type, simulator->pagetable, virtualaddr);
          set_modifiedpte(simulator->type, simulator->pagetable, virtualaddr);
          uint16_t offset = virtualaddr & 0x003f;
          page *frame = simulator->memory + framenumber;
          frame->content[offset] = value; // and write

          write_log(simulator, virtualaddr, framenumber, true);
        }
        free(pagedin_page);
      }
    }
    else if (strcmp(token, "r") == 0)
    {
      // is a read operation
      uint16_t virtualaddr;
      token = strtok(NULL, " ");
      if (!str_to_uint16(token, &virtualaddr))
      {
        fprintf(stderr, "[ERROR] failed to parse virtual address to uint16_t\n");
        break;
      }

      printf("here 1\n");
      fflush(stdout);
      // if the page is valid, simply set its reference bit
      if (isvalid_pte(simulator->type, simulator->pagetable, virtualaddr))
      {
        printf("here 2\n");
        fflush(stdout);

        set_referencedpte(simulator->type, simulator->pagetable, virtualaddr);
        uint16_t framenumber = get_framenumber(simulator->type, simulator->pagetable, virtualaddr);
        write_log(simulator, virtualaddr, framenumber, false);
        if (simulator->algo == LRU)
        {
          update_referencedtime((lru *)simulator->structure, virtualaddr);
        }
      }
      else
      {
        printf("here 3\n");
        fflush(stdout);

        pagefaults++;
        uint16_t page_idx = (virtualaddr & 0xffc0) >> 6;
        page *pagedin_page = get_pagecpy(simulator->ss, page_idx);
        pagetableentry *pte_ref = get_pte_reference(simulator->type, simulator->pagetable, virtualaddr);

        if (simulator->curr < simulator->framecount)
        {
          printf("here 4\n");
          fflush(stdout);

          memcpy(simulator->memory + simulator->curr, pagedin_page, sizeof(page));
          update_framenumber(simulator->type, simulator->pagetable, virtualaddr, simulator->curr);
          set_validpte(simulator->type, simulator->pagetable, virtualaddr);
          set_referencedpte(simulator->type, simulator->pagetable, virtualaddr);

          insert_pte(
              simulator->algo,
              simulator->structure,
              pte_ref,
              simulator->memory + simulator->curr,
              virtualaddr);
          write_log(simulator, virtualaddr, simulator->curr, true);
          simulator->curr++;
        }
        else
        {
          printf("here 5\n");
          fflush(stdout);

          node *evicted_page = evict_node(
              simulator->algo,
              simulator->structure);
          printf("here 6\n");
          fflush(stdout);
          uint16_t framenumber = get_framenumber(simulator->type, simulator->pagetable, evicted_page->virtualaddr);
          insert_pte(
              simulator->algo,
              simulator->structure,
              pte_ref,
              simulator->memory + framenumber,
              virtualaddr);
          printf("here 7\n");
          fflush(stdout);

          unset_validpte(simulator->type, simulator->pagetable, evicted_page->virtualaddr);
          unset_referencedpte(simulator->type, simulator->pagetable, evicted_page->virtualaddr);
          if (ismodified_pte(simulator->type, simulator->pagetable, evicted_page->virtualaddr))
          {
            // overwrite the corresponding page in the swapspace
            write_page(simulator->ss, page_idx, evicted_page->frame);
          }
          unset_modifiedpte(simulator->type, simulator->pagetable, evicted_page->virtualaddr);

          // page in from the swap store
          memcpy(simulator->memory + framenumber, pagedin_page, sizeof(page));
          update_framenumber(simulator->type, simulator->pagetable, virtualaddr, framenumber);
          set_validpte(simulator->type, simulator->pagetable, virtualaddr);
          set_referencedpte(simulator->type, simulator->pagetable, virtualaddr);
          write_log(simulator, virtualaddr, framenumber, true);
        }
        free(pagedin_page);
      }
    }
    else
    {
      // is an invalid operation
      fprintf(stderr, "[ERROR] read_source: invalid operation %s\n", token);
      break;
    }

    memoryreferences++;
    if (memoryreferences == tick)
    {
      memoryreferences = 0;
      reset_references(simulator);
    }
  }
  free(line);
  fclose(infile);

  fprintf(simulator->outfile, "%d", pagefaults);
}

void reset_references(memsim *simulator)
{
  if (simulator->type == ONE_LEVEL)
  {
    for (uint16_t i = 0; i < PAGES; i++)
    {
      uint16_t virtualaddr = i << 6;
      unset_referencedpte(ONE_LEVEL, simulator->pagetable, virtualaddr);
    }
  }
  else
  {
    doublepagetable *pgtable = (doublepagetable *)simulator->pagetable;
    for (uint16_t i = 0; i < (uint16_t)pgtable->maxouttable; i++)
    {
      if (pgtable->pagetables[i] == NULL)
      {
        continue;
      }
      uint16_t virtualaddr = i << 6;
      unset_referencedpte(TWO_LEVEL, simulator->pagetable, virtualaddr);
    }
  }
}

void free_memsim(memsim *simulator)
{
  if (simulator)
  {
    if (simulator->type == TWO_LEVEL)
    {
      doublepagetable *temp = (doublepagetable *)(simulator->pagetable);
      for (int i = 0; i < temp->maxouttable; i++)
      {
        if (temp->pagetables[i] != NULL)
        {
          free(temp->pagetables[i]);
        }
      }
    }
    free_swapspace(&(simulator->ss));
    free(simulator->pagetable);
    free(simulator->memory);
    switch (simulator->algo)
    {
    case FIFO:
      free_fifo((fifo *)simulator->structure);
      break;
    case CLOCK:
      free_sclock((sclock *)simulator->structure);
      break;
    case ECLOCK:
      free_eclock((eclock *)simulator->structure);
      break;
    case LRU:
      free_lru((lru *)simulator->structure);
      break;
    }
    free(simulator);
  }
}