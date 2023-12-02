#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "swapspace.h"

page new_page(void)
{
  page newpage;
  for (int i = 0; i < PAGESIZE; i++)
  {
    newpage.content[i] = 0;
  }
  return newpage;
}

bool make_backingstore(swapspace *ss)
{
  size_t backingstore_size = PAGES * sizeof(page);
  errno = 0;
  if (ftruncate((int)ss->descriptor, backingstore_size) != 0)
  {
    perror("make_backingstore");
    return false;
  }

  munmap(ss->memorymap, ss->size);
  ss->size = backingstore_size;
  ss->memorymap = mmap(NULL, backingstore_size, PROT_READ | PROT_WRITE, MAP_SHARED, ss->descriptor, 0);
  for (int i = 0; i < PAGES; i++)
  {
    ss->memorymap[i] = new_page();
  }

  printf("[INFO] wrote zero filled new pages\n");
  return true;
}

newswapspace new_swapspace(char *filename)
{
  newswapspace invalidreturn = {.isnew = false, .ss = NULL};
  // validate filename length
  printf("string length: %ld\n", strlen(filename));
  if (strlen(filename) >= 64)
  {
    fprintf(stderr, "new_swapspace: filename cannot be longer than 63 characters");
    return invalidreturn;
  }
  swapspace *ss = (swapspace *)malloc(sizeof(swapspace));
  strcpy(ss->filename, filename);

  // open backing store
  FILE *ss_store;
  bool exists = true;
  if (access(filename, F_OK) != 0)
  {
    // backing store does not exist
    // create the backing store and initialize to zero
    ss_store = fopen(filename, "wb+");
    if (ss_store == NULL)
    {
      perror("new_swapspace");
      free(ss);
      return invalidreturn;
    }
    exists = false;
    printf("[INFO] created new empty swapspace\n");
  }
  else
  {
    ss_store = fopen(filename, "ab+");
    if (ss_store == NULL)
    {
      perror("new_swapspace");
      free(ss);
      return invalidreturn;
    }
    printf("[INFO] swapspace already exists\n");
  }

  struct stat sb;
  ss->descriptor = fileno(ss_store);
  if (fstat(ss->descriptor, &sb) == -1)
  {

    perror("could not get backing store stats");
    close(ss->descriptor);
    free(ss_store);
    free(ss);
    return invalidreturn;
  }

  ss->memorymap = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, ss->descriptor, 0);
  ss->size = sb.st_size;

  if (!exists && !make_backingstore(ss))
  {
    free(ss_store);
    free_swapspace(&ss);
    return invalidreturn;
  }

  free(ss_store);
  newswapspace validreturn = {.isnew = !exists, .ss = ss};
  return validreturn;
}

void free_swapspace(swapspace **ss)
{
  if (ss != NULL && (*ss) != NULL)
  {
    munmap((*ss)->memorymap, (*ss)->size);
    close((*ss)->descriptor);
    free((*ss));
    *ss = NULL;
  }
}

page *get_page(const swapspace *ss, const int idx)
{
  return NULL;
}

bool write_page(swapspace *ss, const int idx, const page *pg)
{
  return false;
}

void walk_swapspace(const swapspace *ss)
{
  printf("size of the file: %zu\n", ss->size);
}

bool validate_newswapspace(const swapspace *ss)
{
  size_t expectedsize = sizeof(page) * PAGES;
  printf("[INFO] expected size of the file: %zu\n", expectedsize);
  printf("[INFO] size of the file: %zu\n", ss->size);
  if (expectedsize != ss->size)
  {
    fprintf(stderr, "[ERROR] new swapspace validation failed\n");
    return false;
  }

  for (int i = 0; i < PAGES; i++)
  {
    page sspage = ss->memorymap[i];
    for (int j = 0; j < PAGESIZE; j++)
    {
      if (sspage.content[j] != (char)0)
      {
        fprintf(stderr, "[ERROR] invalid non-zero fill encountered\n");
        return false;
      }
    }
  }

  printf("[INFO] validated new swapspace\n");
  return true;
}