#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pagetable.h"

void *pt;
void setup(void)
{
  pt = (void *)newdblpagetable();
}

void teardown(void)
{
  free(pt);
}

TestSuite(doublepagetable, .init = setup, .fini = teardown);

Test(doublepagetable, frameinsertion)
{
  uint16_t framenumber = get_framenumber(TWO_LEVEL, pt, 0);
  cr_assert(framenumber == 0);

  update_framenumber(TWO_LEVEL, pt, 0, 10);
  framenumber = get_framenumber(TWO_LEVEL, pt, 0);
  cr_assert(framenumber == 10);

  update_framenumber(TWO_LEVEL, pt, 0, 20);
  framenumber = get_framenumber(TWO_LEVEL, pt, 0);
  cr_assert(framenumber == 20);

  update_framenumber(TWO_LEVEL, pt, 0, 8192);
  framenumber = get_framenumber(TWO_LEVEL, pt, 0);
  cr_assert(framenumber == 20); // remains unchanged

  update_framenumber(TWO_LEVEL, pt, 0, 8191);
  framenumber = get_framenumber(TWO_LEVEL, pt, 0);
  cr_assert(framenumber == 8191); // edge case
}

Test(doublepagetable, frameinsertion_multi_index)
{
  for (uint16_t i = 0; i < PAGES; i++)
  {
    uint16_t virtualaddr = i << 6;
    uint16_t framenumber = get_framenumber(TWO_LEVEL, pt, virtualaddr);
    cr_assert(framenumber == 0);

    update_framenumber(TWO_LEVEL, pt, virtualaddr, 10);
    framenumber = get_framenumber(TWO_LEVEL, pt, virtualaddr);
    cr_assert(framenumber == 10);

    update_framenumber(TWO_LEVEL, pt, virtualaddr, 20);
    framenumber = get_framenumber(TWO_LEVEL, pt, virtualaddr);
    cr_assert(framenumber == 20);

    update_framenumber(TWO_LEVEL, pt, virtualaddr, 8192);
    framenumber = get_framenumber(TWO_LEVEL, pt, virtualaddr);
    cr_assert(framenumber == 20); // remains unchanged

    update_framenumber(TWO_LEVEL, pt, virtualaddr, 8191);
    framenumber = get_framenumber(TWO_LEVEL, pt, virtualaddr);
    cr_assert(framenumber == 8191); // edge case
  }
}

Test(doublepagetable, metabits)
{
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == false);

  set_validpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == true);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == false);

  unset_validpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == false);

  set_referencedpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == true);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == false);

  unset_modifiedpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == true);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == false);

  set_modifiedpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == true);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == true);

  set_validpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == true);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == true);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == true);

  unset_referencedpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == true);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == true);

  unset_modifiedpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == true);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == false);

  unset_validpte(TWO_LEVEL, pt, 0);
  cr_assert(isvalid_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(TWO_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(TWO_LEVEL, pt, 0) == false);
}

Test(doublepagetable, metabits_multi_index)
{
  for (int i = 0; i < PAGES; i++)
  {
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == false);

    set_validpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == true, "value of i: %d\n", i);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == false);

    unset_validpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == false);

    set_referencedpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == true);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == false);

    unset_modifiedpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == true);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == false);

    set_modifiedpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == true);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == true);

    set_validpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == true);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == true);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == true);

    unset_referencedpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == true);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == true);

    unset_modifiedpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == true);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == false);

    unset_validpte(TWO_LEVEL, pt, i);
    cr_assert(isvalid_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(TWO_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(TWO_LEVEL, pt, i) == false);
  }
}