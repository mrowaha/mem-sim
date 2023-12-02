#include <stdlib.h>
#include <stdio.h>
#include "memsimarg.h"
#include "swapspace.h"

cmd_args *process_args;
swapspace *ss;

int main(const int argc, const char *argv[])
{
  process_args = new_cmdargs();
  if (!init_cmdargs(process_args, argc, argv))
    goto exit;

  printf("***** MEMSIM ARGUMENTS ******\n");
  print_cmdargs(process_args);
  printf("*****************************\n");

  newswapspace backingstore = new_swapspace(process_args->swapfile);
  if (backingstore.ss != NULL)
  {
    if (backingstore.isnew)
    {
      validate_newswapspace(backingstore.ss);
    }
  }

exit:
  free_swapspace(ss);
  free_cmdargs(process_args);
  return EXIT_SUCCESS;
}