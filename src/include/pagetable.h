#ifndef PAGETABLE_H
#define PAGETABLE_H

/**
 * Page Table Implementation
 * PTE(Page Table Entry): 16 bits
 * R | M | V | Frame Number
 * 1 | 1 | 1 | zerofilled log2 (fcount)
 * This means max bits available for frame number is 13. Hence max frame number is 2^13
 * 2 ^ 13 = 8192 (values ranging from 0 to 8191)
 *
 * VA(Virtual Address): 16 bits
 * If level is set to 1
 * Virtual Page Number | Offset
 *       10 bits      |     6 bits
 * If level is set to 2:
 * Outer Page Number | Virtual Page Number | Offset
 *      5 bits      |    5 bits        |     6 bits
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "swapspace.h"

#define MAX_FRAME_NUMBER (uint16_t)8191

#define SET_REFERENCE_BIT(pte) pte | 0x8000 // set reference bit
#define SET_MODIFY_BIT(pte) pte | 0x4000    // set modify bit
#define SET_VALID_BIT(pte) pte | 0x2000     // set valid bit

#define UNSET_REFERENCE_BIT(pte) pte & 0x7fff
#define UNSET_MODIFY_BIT(pte) pte & 0xbfff
#define UNSET_VALID_BIT(pte) pte & 0xdfff

#define GET_REFERENCE_BIT(pte) pte & 0x8000 // get reference bit
#define GET_MODIFY_BIT(pte) pte & 0x4000
#define GET_VALID_BIT(pte) pte & 0x2000

/**
 * page table enty
 * <fcount> least significant bits are used as frame number
 * one bit is used as reference bit
 * one bit is used as modify bit
 * one bit is used as valid/invalid bit
 */
typedef uint16_t pagetableentry;

enum PAGETABLE
{
  ONE_LEVEL,
  TWO_LEVEL
};

typedef struct singlepagetable
{
  pagetableentry entries[PAGES];
} singlepagetable;

singlepagetable *newpagetable(void);

singlepagetable *newpagetable(void);

typedef struct doublepagetable
{
  pagetableentry pagetables[32][32];
} doublepagetable;

doublepagetable *newdblpagetable(void);

bool update_framenumber(enum PAGETABLE type, void *table, const uint16_t virtualaddr, const uint16_t framenumber);

uint16_t get_framenumber(enum PAGETABLE type, const void *table, const uint16_t virtuaddr);

bool set_validpte(enum PAGETABLE type, void *table, const uint16_t virtualaddr);

bool unset_validpte(enum PAGETABLE type, void *table, const uint16_t virtualaddr);

bool set_referencedpte(enum PAGETABLE type, void *table, const uint16_t virtualaddr);

bool unset_referencedpte(enum PAGETABLE type, void *table, const uint16_t virtualaddr);

bool set_modifiedpte(enum PAGETABLE type, void *table, const uint16_t virtualaddr);

bool unset_modifiedpte(enum PAGETABLE type, void *table, const uint16_t virtualaddr);

bool isvalid_pte(enum PAGETABLE type, const void *table, const uint16_t virtualaddr);

bool isreferenced_pte(enum PAGETABLE type, const void *table, const uint16_t virtualaddr);

bool ismodified_pte(enum PAGETABLE type, const void *table, const uint16_t virtualaddr);

#endif