#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memsimarg.h"

/**
 * this function maps string repr of algorithm to enum
 * @algo_str: the string repr of teh algorithm
 */
ALGO get_algo(const char *algo_str)
{

  if (strcmp(algo_str, "FIFO") == 0)
  {
    return FIFO;
  }
  else if (strcmp(algo_str, "LRU") == 0)
  {
    return LRU;
  }
  else if (strcmp(algo_str, "CLOCK") == 0)
  {
    return CLOCK;
  }
  else if (strcmp(algo_str, "ECLOCK") == 0)
  {
    return ECLOCK;
  }

  return INVALID_ALGO;
}

void get_algo_str(ALGO algo, char **algo_str)
{

  if (*algo_str != NULL)
  {
    fprintf(stderr, "get_algo_str: expects a NULL char pointer\n");
    return;
  }

  if (algo == FIFO)
  {
    *algo_str = (char *)malloc(sizeof(char) * 5);
    strcpy(*algo_str, "FIFO");
  }
  else if (algo == LRU)
  {
    *algo_str = (char *)malloc(sizeof(char) * 4);
    strcpy(*algo_str, "LRU");
  }
  else if (algo == CLOCK)
  {
    *algo_str = (char *)malloc(sizeof(char) * 6);
    strcpy(*algo_str, "CLOCK");
  }
  else if (algo == ECLOCK)
  {
    *algo_str = (char *)malloc(sizeof(char) * 7);
    strcpy(*algo_str, "ECLOCK");
  }
}

cmd_args *new_cmdargs(void)
{
  cmd_args *args = (cmd_args *)malloc(sizeof(cmd_args));
  args->addrfile = NULL;
  args->swapfile = NULL;
  args->outfile = NULL;
  args->fcount = -1;
  args->algo = FIFO;
  args->tick = -1;
  return args;
}

void free_cmdargs(cmd_args *args)
{
  if (args != NULL)
  {
    if (args->addrfile != NULL)
    {
      free(args->addrfile);
    }

    if (args->swapfile != NULL)
    {
      free(args->swapfile);
    }

    if (args->outfile != NULL)
    {
      free(args->outfile);
    }
    free(args);
  }
}

void print_cmdargs(const cmd_args *args)
{
  printf("-p [leve]: %d\n", args->level);
  printf("-f [fcount]: %d\n", args->fcount);

  char *algo_str = NULL;
  get_algo_str(args->algo, &algo_str);
  printf("-a [algo]: %s (%d)\n", algo_str, args->algo);
  free(algo_str);

  printf("-t [tick]: %d\n", args->tick);
  if (args->addrfile != NULL)
  {
    printf("-r [addrfile]: %s\n", args->addrfile);
  }
  else
  {
    fprintf(stderr, "[ERROR] addrfile not found");
  }

  if (args->swapfile != NULL)
  {
    printf("-s [swapfile]: %s\n", args->swapfile);
  }
  else
  {
    fprintf(stderr, "[ERROR] swapfile not found");
  }

  if (args->outfile != NULL)
  {
    printf("-o [outfile]: %s\n", args->outfile);
  }
  else
  {
    fprintf(stderr, "[ERROR] outfile not found");
  }
}

#define HAS_LEVEL (int)0x0000001
#define HAS_ADDRFILE (int)0x0000010
#define HAS_SWAPFILE (int)0x0000100
#define HAS_FCOUNT (int)0x0001000
#define HAS_ALGO (int)0x0010000
#define HAS_TICK (int)0x0100000
#define HAS_OUTFILE (int)0x1000000

bool init_cmdargs(cmd_args *args, const int argc, const char *argv[])
{
  int validation = 0;

  if (argc != 15)
  {
    fprintf(stderr, "[ERROR] incomplete args\n");
    return false;
  }

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-p") == 0)
    {
      /* validate level arg */
      if (i == argc - 1)
      {
        fprintf(stderr, "[ERROR] -p requires a value 1 or 2\n");
        return false;
      }
      int level = atoi(argv[i + 1]);
      if (level != 1 && level != 2)
      {
        fprintf(stderr, "[ERROR] -p can only have value 1 or 2\n");
        return false;
      }
      validation = validation | HAS_LEVEL;
      args->level = level;
    }
    else if (strcmp(argv[i], "-r") == 0)
    {
      /* validate addrfile arg */
      if (i == argc - 1)
      {
        fprintf(stderr, "[ERROR] -r requires a value\n");
        return false;
      }
      int addrfile_length = sizeof(argv[i + 1]);
      args->addrfile = (char *)malloc(addrfile_length);
      strcpy(args->addrfile, argv[i + 1]);
      validation = validation | HAS_ADDRFILE;
    }
    else if (strcmp(argv[i], "-s") == 0)
    {
      /* validate swapfile arg */
      if (i == argc - 1)
      {
        fprintf(stderr, "[ERROR] -s requires a value\n");
        return false;
      }
      int swapfile_length = sizeof(argv[i + 1]);
      args->swapfile = (char *)malloc(swapfile_length);
      strcpy(args->swapfile, argv[i + 1]);
      validation = validation | HAS_SWAPFILE;
    }
    else if (strcmp(argv[i], "-f") == 0)
    {
      /* validate fcount arg */
      if (i == argc - 1)
      {
        fprintf(stderr, "[ERROR] -f requires a value\n");
        return false;
      }
      int fcount = atoi(argv[i + 1]);
      if (fcount < 4 || fcount > 128)
      {
        fprintf(stderr, "[ERROR] -f can only have a value between 4 and 128\n");
        return false;
      }
      validation = validation | HAS_FCOUNT;
      args->fcount = fcount;
    }
    else if (strcmp(argv[i], "-a") == 0)
    {
      /* validate algo arg */
      if (i == argc - 1)
      {
        fprintf(stderr, "[ERROR] -a requires a value\n");
        return false;
      }
      ALGO algo = get_algo(argv[i + 1]);
      if (algo == INVALID_ALGO)
      {
        fprintf(stderr, "[ERROR] -a can only have FIFO, LRU, CLOCK or ECLOCK\n");
        return false;
      }
      args->algo = algo;
      validation = validation | HAS_ALGO;
    }
    else if (strcmp(argv[i], "-t") == 0)
    {
      /* validate tick arg */
      if (i == argc - 1)
      {
        fprintf(stderr, "[ERROR] -t requires a value\n");
        return false;
      }
      int tick = atoi(argv[i + 1]);
      validation = validation | HAS_TICK;
      args->tick = tick;
    }
    else if (strcmp(argv[i], "-o") == 0)
    {
      /* validate outfile arg */
      if (i == argc - 1)
      {
        fprintf(stderr, "[ERROR] -o requires a value\n");
        return false;
      }
      int outfile_length = sizeof(argv[i + 1]);
      args->outfile = (char *)malloc(sizeof(char) * outfile_length);
      strcpy(args->outfile, argv[i + 1]);
      validation = validation | HAS_OUTFILE;
    } /* else ignore invalid args */
  }

  if (!(validation & HAS_LEVEL))
  {
    fprintf(stderr, "[ERROR] missing -p <level> value\n");
    return false;
  }

  if (!(validation & HAS_ADDRFILE))
  {
    fprintf(stderr, "[ERROR] missing -r <addrfile> value\n");
    return false;
  }

  if (!(validation & HAS_SWAPFILE))
  {
    fprintf(stderr, "[ERROR] missing -s <swapfile> value\n");
    return false;
  }

  if (!(validation & HAS_FCOUNT))
  {
    fprintf(stderr, "[ERROR] missing -f <fcount> value\n");
    return false;
  }

  if (!(validation & HAS_ALGO))
  {
    fprintf(stderr, "[ERROR] missing -a <algo> value\n");
    return false;
  }

  if (!(validation & HAS_TICK))
  {
    fprintf(stderr, "[ERROR] missing -t <tick> value\n");
    return false;
  }

  if (!(validation & HAS_OUTFILE))
  {
    fprintf(stderr, "[ERROR] missing -o <outfile> value\n");
    return false;
  }

  return true;
}