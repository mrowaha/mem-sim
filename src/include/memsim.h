#ifndef MEMSIM_H
#define MEMSIM_H

#include <stdio.h>

#include "memsimarg.h"
#include "swapspace.h"
#include "pagetable.h"

typedef struct memsim
{
  swapspace *ss;
  enum PAGETABLE type;
  void *pagetable;
  ALGO algo;
  void *structure;
  page *memory;
  int framecount;
  int curr;
  FILE *outfile;
} memsim;

memsim *new_memsim(int level, int fcount, char *swapfile, char *outfile, ALGO algo);

void free_memsim(memsim *);

void read_source(memsim *, const char *inputfile);

#endif