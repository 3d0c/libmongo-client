#include "test.h"

#include <string.h>

void
test_bson_oid_new_with_time (void)
{
  bson_oid_t *oid1, *oid2, *oid3;
  char *oid1_s, *oid2_s;

  ok (bson_oid_new_with_time (0, 0) == NULL,
      "bson_oid_new_with_time() should fail before bson_oid_init()");

  bson_oid_init (0);
  ok ((oid1 = bson_oid_new_with_time (0, 1)) != NULL,
      "bson_oid_new_with_time() works");
  cmp_ok (oid1->bytes[11], "==", 1,
	  "bson_oid_new_with_time() returns an OID with the currect seq ID");

  oid2 = bson_oid_new_with_time (0, 2);
  oid3 = bson_oid_new_with_time (0, 2);

  ok (memcmp (oid2, oid1, 12) > 0,
      "OIDs with higher sequence ID sort higher");
  ok (memcmp (oid2, oid3, 12) == 0,
      "OIDs with the same sequence ID are equal (within a second)");
  free (oid2);
  free (oid3);

  oid2 = bson_oid_new_with_time (1, 0);

  oid1_s = bson_oid_as_string (oid1);
  oid2_s = bson_oid_as_string (oid2);

  ok (memcmp (oid2, oid1, 12) > 0,
      "OIDs with the same sequence ID, a few seconds later sort higher; "
      "oid1=%s; oid2=%s", oid1_s, oid2_s);
  free (oid2_s);
  free (oid1_s);
  free (oid2);
  free (oid1);
}

RUN_TEST (6, bson_oid_new_with_time);
