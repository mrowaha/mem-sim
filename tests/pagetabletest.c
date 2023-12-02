#include <criterion/criterion.h>
#include <criterion/new/assert.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pagetable.h"

Test(pagetable, frameinsertion)
{
  pagetable *pt = newpagetable(2, 8);
  uint16_t framenumber = get_framenumber(pt, 0);
  cr_assert(framenumber == 0);

  update_framenumber(pt, 0, 10);
  framenumber = get_framenumber(pt, 0);
  cr_assert(framenumber == 10);

  update_framenumber(pt, 0, 20);
  framenumber = get_framenumber(pt, 0);
  cr_assert(framenumber == 20);

  update_framenumber(pt, 0, 8192);
  framenumber = get_framenumber(pt, 0);
  cr_assert(framenumber == 20); // remains unchanged

  update_framenumber(pt, 0, 8191);
  framenumber = get_framenumber(pt, 0);
  cr_assert(framenumber == 8191); // edge case

  free(pt);
}

Test(pagetable, frameinsertion_multi_index)
{
  pagetable *pt = newpagetable(2, 8);
  uint16_t framenumber = get_framenumber(pt, 0);
  cr_assert(framenumber == 0);

  update_framenumber(pt, 0, 10);
  framenumber = get_framenumber(pt, 0);
  cr_assert(framenumber == 10);

  update_framenumber(pt, 2, 20);
  framenumber = get_framenumber(pt, 2);
  cr_assert(framenumber == 20);

  update_framenumber(pt, 3, 8192);
  framenumber = get_framenumber(pt, 3);
  cr_assert(framenumber == 0); // remains unchanged

  update_framenumber(pt, 3, 8191);
  framenumber = get_framenumber(pt, 3);
  cr_assert(framenumber == 8191); // edge case

  free(pt);
}

Test(pagetable, metabits)
{
  pagetable *pt = newpagetable(2, 8);
  cr_assert(isvalid_pte(pt, 0) == false);
  cr_assert(isreferenced_pte(pt, 0) == false);
  cr_assert(ismodified_pte(pt, 0) == false);

  set_validpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == true);
  cr_assert(isreferenced_pte(pt, 0) == false);
  cr_assert(ismodified_pte(pt, 0) == false);

  unset_validpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == false);
  cr_assert(isreferenced_pte(pt, 0) == false);
  cr_assert(ismodified_pte(pt, 0) == false);

  set_referencedpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == false);
  cr_assert(isreferenced_pte(pt, 0) == true);
  cr_assert(ismodified_pte(pt, 0) == false);

  unset_modifiedpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == false);
  cr_assert(isreferenced_pte(pt, 0) == true);
  cr_assert(ismodified_pte(pt, 0) == false);

  set_modifiedpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == false);
  cr_assert(isreferenced_pte(pt, 0) == true);
  cr_assert(ismodified_pte(pt, 0) == true);

  set_validpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == true);
  cr_assert(isreferenced_pte(pt, 0) == true);
  cr_assert(ismodified_pte(pt, 0) == true);

  unset_referencedpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == true);
  cr_assert(isreferenced_pte(pt, 0) == false);
  cr_assert(ismodified_pte(pt, 0) == true);

  unset_modifiedpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == true);
  cr_assert(isreferenced_pte(pt, 0) == false);
  cr_assert(ismodified_pte(pt, 0) == false);

  unset_validpte(pt, 0);
  cr_assert(isvalid_pte(pt, 0) == false);
  cr_assert(isreferenced_pte(pt, 0) == false);
  cr_assert(ismodified_pte(pt, 0) == false);

  free(pt);
}

Test(pagetable, metabits_indexone)
{
  pagetable *pt = newpagetable(2, 8);
  for (int i = 0; i < PAGES; i++)
  {
    cr_assert(isvalid_pte(pt, i) == false);
    cr_assert(isreferenced_pte(pt, i) == false);
    cr_assert(ismodified_pte(pt, i) == false);

    set_validpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == true, "value of i: %d\n", i);
    cr_assert(isreferenced_pte(pt, i) == false);
    cr_assert(ismodified_pte(pt, i) == false);

    unset_validpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == false);
    cr_assert(isreferenced_pte(pt, i) == false);
    cr_assert(ismodified_pte(pt, i) == false);

    set_referencedpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == false);
    cr_assert(isreferenced_pte(pt, i) == true);
    cr_assert(ismodified_pte(pt, i) == false);

    unset_modifiedpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == false);
    cr_assert(isreferenced_pte(pt, i) == true);
    cr_assert(ismodified_pte(pt, i) == false);

    set_modifiedpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == false);
    cr_assert(isreferenced_pte(pt, i) == true);
    cr_assert(ismodified_pte(pt, i) == true);

    set_validpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == true);
    cr_assert(isreferenced_pte(pt, i) == true);
    cr_assert(ismodified_pte(pt, i) == true);

    unset_referencedpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == true);
    cr_assert(isreferenced_pte(pt, i) == false);
    cr_assert(ismodified_pte(pt, i) == true);

    unset_modifiedpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == true);
    cr_assert(isreferenced_pte(pt, i) == false);
    cr_assert(ismodified_pte(pt, i) == false);

    unset_validpte(pt, i);
    cr_assert(isvalid_pte(pt, i) == false);
    cr_assert(isreferenced_pte(pt, i) == false);
    cr_assert(ismodified_pte(pt, i) == false);
  }
  free(pt);
}