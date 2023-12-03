#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pagetable.h"

Test(singlepagetable, frameinsertion)
{
  void *pt = (void *)newpagetable();
  uint16_t framenumber = get_framenumber(ONE_LEVEL, pt, 0);
  cr_assert(framenumber == 0);

  update_framenumber(ONE_LEVEL, pt, 0, 10);
  framenumber = get_framenumber(ONE_LEVEL, pt, 0);
  cr_assert(framenumber == 10);

  update_framenumber(ONE_LEVEL, pt, 0, 20);
  framenumber = get_framenumber(ONE_LEVEL, pt, 0);
  cr_assert(framenumber == 20);

  update_framenumber(ONE_LEVEL, pt, 0, 8192);
  framenumber = get_framenumber(ONE_LEVEL, pt, 0);
  cr_assert(framenumber == 20); // remains unchanged

  update_framenumber(ONE_LEVEL, pt, 0, 8191);
  framenumber = get_framenumber(ONE_LEVEL, pt, 0);
  cr_assert(framenumber == 8191); // edge case
  free(pt);
}

Test(singlepagetable, frameinsertion_multi_index)
{
  void *pt = (void *)newpagetable();
  uint16_t framenumber = get_framenumber(ONE_LEVEL, pt, 0);
  cr_assert(framenumber == 0);

  update_framenumber(ONE_LEVEL, pt, 0, 10);
  framenumber = get_framenumber(ONE_LEVEL, pt, 0);
  cr_assert(framenumber == 10);

  update_framenumber(ONE_LEVEL, pt, 2, 20);
  framenumber = get_framenumber(ONE_LEVEL, pt, 2);
  cr_assert(framenumber == 20);

  update_framenumber(ONE_LEVEL, pt, 3, 8192);
  framenumber = get_framenumber(ONE_LEVEL, pt, 3);
  cr_assert(framenumber == 20); // remains unchanged

  update_framenumber(ONE_LEVEL, pt, 3, 8191);
  framenumber = get_framenumber(ONE_LEVEL, pt, 3);
  cr_assert(framenumber == 8191); // edge case

  free(pt);
}

Test(singlepagetable, metabits)
{
  void *pt = (void *)newpagetable();
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == false);

  set_validpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == true);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == false);

  unset_validpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == false);

  set_referencedpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == true);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == false);

  unset_modifiedpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == true);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == false);

  set_modifiedpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == true);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == true);

  set_validpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == true);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == true);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == true);

  unset_referencedpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == true);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == true);

  unset_modifiedpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == true);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == false);

  unset_validpte(ONE_LEVEL, pt, 0);
  cr_assert(isvalid_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(isreferenced_pte(ONE_LEVEL, pt, 0) == false);
  cr_assert(ismodified_pte(ONE_LEVEL, pt, 0) == false);

  free(pt);
}

Test(singlepagetable, metabits_indexone)
{
  void *pt = (void *)newpagetable();
  for (int i = 0; i < PAGES; i++)
  {
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == false);

    set_validpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == true, "value of i: %d\n", i);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == false);

    unset_validpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == false);

    set_referencedpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == true);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == false);

    unset_modifiedpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == true);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == false);

    set_modifiedpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == true);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == true);

    set_validpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == true);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == true);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == true);

    unset_referencedpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == true);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == true);

    unset_modifiedpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == true);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == false);

    unset_validpte(ONE_LEVEL, pt, i);
    cr_assert(isvalid_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(isreferenced_pte(ONE_LEVEL, pt, i) == false);
    cr_assert(ismodified_pte(ONE_LEVEL, pt, i) == false);
  }
  free(pt);
}