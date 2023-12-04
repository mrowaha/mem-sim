#ifndef MEMSIM_H
#define MEMSIM_H

#include <stdio.h>

#include "memsimarg.h"
#include "swapspace.h"
#include "pagetable.h"
#include "algorithmsk.h"

typedef struct memsim
{
  ALGO pagereplaceralgo;
  void *pagereplacer;
  PAGETABLE type;
  void *vpt;
  swapspace *ss;
  page *memory;
  int currmemorysize;
  int framecount;
  FILE *outfile;
} memsim;

memsim *new_memsim(int level, int fcount, char *swapfile, char *outfile, ALGO algo);

void free_memsim(memsim *);

void read_source(memsim *, const char *inputfile, const int tick);

void reset_references(memsim *);

#endif