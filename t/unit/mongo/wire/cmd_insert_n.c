#include "test.h"

static void
test_mongo_wire_cmd_insert_n (void)
{
  bson_t *ins, *tmp;
  const bson_t *docs[10];
  mongo_wire_packet_t *p;

  mongo_wire_packet_header_t hdr;
  const uint8_t *data;
  int32_t data_size;

  bson_cursor_t *c;
  int32_t pos;

  ins = test_bson_generate_full ();
  tmp = bson_new ();

  docs[0] = ins;
  docs[1] = tmp;
  docs[2] = ins;
  docs[3] = ins;
  docs[4] = NULL;
  docs[5] = ins;

  ok (mongo_wire_cmd_insert_n (1, NULL, 1, docs) == NULL,
      "mongo_wire_cmd_insert_n() fails with a NULL namespace");
  ok (mongo_wire_cmd_insert_n (1, "test.ns", 1, NULL) == NULL,
      "mongo_wire_cmd_insert_n() fails with no documents");
  ok (mongo_wire_cmd_insert_n (1, "test.ns", 0, docs) == NULL,
      "mongo_wire_cmd_insert_n() fails with no documents");
  ok (mongo_wire_cmd_insert_n (1, "test.ns", 2, docs) == NULL,
      "mongo_wire_cmd_insert_n() fails with an unfinished document");
  bson_finish (tmp);
  ok (mongo_wire_cmd_insert_n (1, "test.ns", 5, docs) == NULL,
      "mongo_wire_cmd_insert_n() fails with a NULL document in the array");
  ok ((p = mongo_wire_cmd_insert_n (1, "test.ns", 3, docs)) != NULL,
      "mongo_wire_cmd_insert() works");
  bson_free (ins);
  bson_free (tmp);

  /* Test basic header data */
  mongo_wire_packet_get_header (p, &hdr);
  cmp_ok ((data_size = mongo_wire_packet_get_data (p, &data)), "!=", -1,
	  "Packet data size appears fine");

  cmp_ok (hdr.length, "==", sizeof (mongo_wire_packet_header_t) + data_size,
	  "Packet header length is correct");
  cmp_ok (hdr.id, "==", 1, "Header ID is ok");
  cmp_ok (hdr.resp_to, "==", 0, "Response ID is ok");

  /*
   * Test the first document
   */

  /* pos = zero + collection_name + NULL */
  pos = sizeof (int32_t) + strlen ("test.ns") + 1;
  ok ((ins = bson_new_from_data (data + pos,
				 bson_stream_doc_size (data, pos) - 1)) != NULL,
      "First document is included");
  bson_finish (ins);

  c = bson_cursor_find (bson_cursor_new (ins), "int32");
  ok (lmc_error_isok (c),
      "BSON contains 'int32'");
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT32,
	  "int32 has correct type");
  bson_cursor_next (c);
  cmp_ok (bson_cursor_type (c), "==", BSON_TYPE_INT64,
	  "next element has correct type too");
  ok (!lmc_error_isok (bson_cursor_next (c)),
      "No more data after the update BSON object");
  bson_cursor_free (c);

  /*
   * Test the second document
   */
  pos += bson_size (ins);
  ok ((tmp = bson_new_from_data (data + pos,
				 bson_stream_doc_size (data, pos) - 1)) != NULL,
      "Second document is included");
  bson_finish (tmp);
  cmp_ok (bson_size (tmp), "==", 5,
	  "Second document is empty");

  bson_free (ins);
  bson_free (tmp);
  mongo_wire_packet_free (p);
}

RUN_TEST (17, mongo_wire_cmd_insert_n);
