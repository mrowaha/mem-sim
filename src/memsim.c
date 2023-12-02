#include <stdlib.h>
#include <stdio.h>
#include "memsimarg.h"

cmd_args *process_args;

int main(const int argc, const char *argv[])
{
  process_args = new_cmdargs();
  if (!init_cmdargs(process_args, argc, argv))
    goto exit;

  print_cmdargs(process_args);

exit:
  free_cmdargs(process_args);
  return EXIT_SUCCESS;
}