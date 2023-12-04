#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "memsimk.h"

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
    simulator->vpt = (void *)newpagetable();
    simulator->type = ONE_LEVEL;
  }
  else
  {
    // else initialize to a TWO_LEVEL page table
    simulator->vpt = (void *)newdblpagetable();
    simulator->type = TWO_LEVEL;
  }

  // initialize memory to fcount and all frames zeroed out
  simulator->framecount = fcount;
  simulator->memory = (page *)malloc(sizeof(page) * fcount);
  simulator->currmemorysize = 0;
  for (int i = 0; i < fcount; i++)
  {
    simulator->memory[i] = new_page();
  }

  simulator->pagereplaceralgo = algo;
  switch (algo)
  {
  case FIFO:
    simulator->pagereplacer = (void *)new_fifo(
        simulator->type,
        simulator->vpt,
        fcount);
    break;
  case CLOCK:
    simulator->pagereplacer = (void *)new_sclock(
        simulator->type,
        simulator->vpt,
        fcount);
    break;
  case ECLOCK:
    simulator->pagereplacer = (void *)new_eclock(
        simulator->type,
        simulator->vpt,
        fcount);
    break;
  case LRU:
    simulator->pagereplacer = (void *)new_lru(
        simulator->type,
        simulator->vpt,
        fcount);
    break;
  default:
    fprintf(stderr, "[ERROR] invalid algorithm type: %d\n", algo);
    free_swapspace(&(newss.ss));
    free(simulator->vpt);
    free(simulator->memory);
    free(simulator);
    return NULL;
  }
  return simulator;
}

void free_memsim(memsim *simulator)
{
  if (simulator)
  {
    if (simulator->type == TWO_LEVEL)
    {
      doublepagetable *temp = (doublepagetable *)(simulator->vpt);
      for (int i = 0; i < temp->maxouttable; i++)
      {
        if (temp->pagetables[i] != NULL)
        {
          free(temp->pagetables[i]);
        }
      }
    }
    free_swapspace(&(simulator->ss));
    free(simulator->vpt);
    free(simulator->memory);
    switch (simulator->pagereplaceralgo)
    {
    case FIFO:
      free_fifo((fifo *)simulator->pagereplacer);
      break;
    case CLOCK:
      free_sclock((sclock *)simulator->pagereplacer);
      break;
    case ECLOCK:
      free_eclock((eclock *)simulator->pagereplacer);
      break;
    case LRU:
      free_lru((lru *)simulator->pagereplacer);
    }
    free(simulator);
  }
}

void reset_references(memsim *simulator)
{
  if (simulator->type == ONE_LEVEL)
  {
    for (uint16_t i = 0; i < PAGES; i++)
    {
      uint16_t virtualaddr = i << 6;
      unset_referencedpte(ONE_LEVEL, simulator->vpt, virtualaddr);
    }
  }
  else
  {
    doublepagetable *pgtable = (doublepagetable *)simulator->vpt;
    for (uint16_t i = 0; i < (uint16_t)pgtable->maxouttable; i++)
    {
      if (pgtable->pagetables[i] == NULL)
      {
        continue;
      }
      uint16_t virtualaddr = i << 6;
      unset_referencedpte(TWO_LEVEL, simulator->vpt, virtualaddr);
    }
  }
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

      if (isvalid_pte(simulator->type, simulator->vpt, virtualaddr))
      {
        set_referencedpte(simulator->type, simulator->vpt, virtualaddr);
        set_modifiedpte(simulator->type, simulator->vpt, virtualaddr);
        uint16_t framenumber = get_framenumber(simulator->type, simulator->vpt, virtualaddr);
        writetopage(simulator->memory + framenumber, virtualaddr, value);
        write_log(simulator, virtualaddr, framenumber, false);
        if (simulator->pagereplaceralgo == LRU)
        {
          update_referencedtime((lru *)simulator->pagereplacer, virtualaddr);
        }
      }
      else
      {
        pagefaults++;
        struct pagefaultresult result = handlepagefault(
            simulator->pagereplaceralgo,
            simulator->pagereplacer,
            virtualaddr,
            simulator->memory,
            &(simulator->currmemorysize),
            simulator->ss,
            true,
            value);
        write_log(simulator, virtualaddr, result.PFN, true);
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

      // if the page is valid, simply set its reference bit
      if (isvalid_pte(simulator->type, simulator->vpt, virtualaddr))
      {
        set_referencedpte(simulator->type, simulator->vpt, virtualaddr);
        uint16_t framenumber = get_framenumber(simulator->type, simulator->vpt, virtualaddr);
        write_log(simulator, virtualaddr, framenumber, false);
        if (simulator->pagereplaceralgo == LRU)
        {
          update_referencedtime((lru *)simulator->pagereplacer, virtualaddr);
        }
      }
      else
      {
        pagefaults++;
        struct pagefaultresult result = handlepagefault(
            simulator->pagereplaceralgo,
            simulator->pagereplacer,
            virtualaddr,
            simulator->memory,
            &(simulator->currmemorysize),
            simulator->ss,
            false,
            0);
        write_log(simulator, virtualaddr, result.PFN, true);
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
  fflush(simulator->outfile);
}
