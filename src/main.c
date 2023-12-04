#include <stdlib.h>
#include <stdio.h>

#include "memsimk.h"
#include "memsimarg.h"

cmd_args *process_args;
memsim *simulator;

int main(const int argc, const char *argv[])
{

  process_args = new_cmdargs();
  if (!init_cmdargs(process_args, argc, argv))
    goto exit;

  printf("***** MEMSIM ARGUMENTS ******\n");
  print_cmdargs(process_args);
  printf("*****************************\n");

  simulator = new_memsim(
      process_args->level,
      process_args->fcount,
      process_args->swapfile,
      process_args->outfile,
      process_args->algo);

  read_source(simulator, process_args->addrfile, process_args->tick);
  printf("DONE\n");
exit:
  free_cmdargs(process_args);
  exit(EXIT_FAILURE);
}