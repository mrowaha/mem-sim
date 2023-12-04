#ifndef SWAPSPACE_H
#define SWAPSPACE_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/* page size in bytes */
#define PAGESIZE 64

/* maximum number of pages in backing store */
#define PAGES 1024

typedef struct page
{
  uint8_t content[PAGESIZE];
} page;

page new_page(void);

typedef int ss_descriptor;

/**
 * swapspace structs for the virtual memory backing store
 * @filename: filename of the backing store file, maximum characters can be 63 (excluding the NULL character)
 * @pages: backing store pages, maximum 1024 pages
 * @memorymap: backing store mmap
 */
typedef struct swapspace
{
  char filename[64];
  ss_descriptor descriptor;
  page *memorymap;
  size_t size;
} swapspace;

typedef struct newswapspace
{
  swapspace *ss;
  bool isnew;
} newswapspace;

newswapspace new_swapspace(char *filename);

void free_swapspace(swapspace **);

page *get_pagecpy(const swapspace *, const int idx);

bool write_page(swapspace *, const int idx, const page *);

void walk_swapspace(const swapspace *);

bool validate_newswapspace(const swapspace *);

#endif