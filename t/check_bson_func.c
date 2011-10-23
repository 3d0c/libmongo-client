#include <check.h>
#include <stdlib.h>

#include <lmc/bson.h>

START_TEST (test_func_bson_close)
{
  bson_t *b;
  bson_element_t *e;

  b = bson_new ();
  e = bson_element_create ("hello", BSON_TYPE_STRING,
			   "world", BSON_LENGTH_AUTO);

  b = bson_add_elements (b, e, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_length (b), 2);

  b = bson_close (b);
  b = bson_add_elements (b, e, BSON_END);
  ck_assert_int_eq (bson_length (b), 2);

  b = bson_open (b);
  b = bson_add_elements (b, bson_element_ref (e), BSON_END);
  ck_assert_int_eq (bson_length (b), 3);

  bson_unref (b);
}
END_TEST

START_TEST (test_func_bson_flatten)
{
  bson_t *b;
  uint32_t size, size2;

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);
  b = bson_close (b);
  size = bson_data_get_size (b);
  _ck_assert_int (size, >, 16);

  b = bson_open (b);
  b = bson_add_elements (b, bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14),
			 BSON_END);
  b = bson_close (b);
  size2 = bson_data_get_size (b);
  _ck_assert_int (size2, >, size);

  b = bson_reset_elements (b);
  b = bson_add_elements (b, bson_element_create ("pi", BSON_TYPE_DOUBLE, 3.14),
			 BSON_END);
  b = bson_close (b);

  _ck_assert_int (bson_data_get_size (b), <, size2);

  bson_unref (b);
}
END_TEST

START_TEST (test_func_bson_get_nth_element)
{
  bson_t *b;
  bson_element_t *e1, *e2;

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     bson_element_create ("answer", BSON_TYPE_INT32, 42),
     BSON_END);

  e1 = bson_get_nth_element (b, 2);
  ck_assert (bson_element_type_get (e1) == BSON_TYPE_STRING ||
	     bson_element_type_get (e1) == BSON_TYPE_INT32);

  b = bson_close (b);

  e2 = bson_get_nth_element (b, 2);
  ck_assert (bson_element_type_get (e2) == BSON_TYPE_STRING ||
	     bson_element_type_get (e2) == BSON_TYPE_INT32);

  ck_assert (e1 == e2);

  bson_unref (b);
}
END_TEST

START_TEST (test_func_bson_stream)
{
  bson_t *b;

  b = bson_new_build
    (bson_element_create ("hello", BSON_TYPE_STRING,
			  "world", BSON_LENGTH_AUTO),
     BSON_END);
  b = bson_close (b);

  ck_assert_int_eq (bson_data_get_size (b), 22);
  ck_assert (memcmp (bson_data_get (b),
		     "\x16\x00\x00\x00\x02hello\x00"
		     "\x06\x00\x00\x00world\x00\x00", 22) == 0);

  bson_unref (b);
}
END_TEST

Suite *
bson_func_suite (void)
{
  Suite *s;

  TCase *tc_manip, *tc_stream;

  s = suite_create ("BSON functional tests");

  tc_manip = tcase_create ("BSON manipulation");
  tcase_add_test (tc_manip, test_func_bson_close);
  tcase_add_test (tc_manip, test_func_bson_flatten);
  tcase_add_test (tc_manip, test_func_bson_get_nth_element);
  suite_add_tcase (s, tc_manip);

  tc_stream = tcase_create ("BSON stream");
  tcase_add_test (tc_stream, test_func_bson_stream);
  suite_add_tcase (s, tc_stream);

  return s;
}

int
main (void)
{
  int nfailed;
  Suite *s;
  SRunner *sr;

  s = bson_func_suite ();
  sr = srunner_create (s);

  srunner_run_all (sr, CK_ENV);
  nfailed = srunner_ntests_failed (sr);
  srunner_free (sr);

  return (nfailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
