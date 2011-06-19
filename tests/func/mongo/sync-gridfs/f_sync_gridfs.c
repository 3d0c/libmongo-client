#include "test.h"
#include "mongo.h"

#define FILE_SIZE 1024 * 1024 + 12345

static guint8 noname_oid[12];
static guint8 named_oid[12];

gchar *
oid_to_string (const guint8* oid)
{
  static gchar os[25];
  gint j;

  for (j = 0; j < 12; j++)
    sprintf (&os[j * 2], "%02x", oid[j]);
  os[25] = 0;
  return os;
}

void
test_func_sync_gridfs_put (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_file *gfile;
  bson *meta;
  guint8 *data;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);
  meta = bson_build (BSON_TYPE_STRING, "filename", "libmongo-test", -1,
		     BSON_TYPE_NONE);
  bson_finish (meta);

  data = g_malloc (FILE_SIZE);
  memset (data, 'x', FILE_SIZE);

  gfile = mongo_sync_gridfs_file_new_from_buffer (gfs, meta, data, FILE_SIZE);
  ok (gfile != NULL,
      "GridFS file upload (with metadata) works!");
  memcpy (named_oid, mongo_sync_gridfs_file_get_id (gfile), 12);
  note ("Named file ID : %s\n", oid_to_string (named_oid));
  mongo_sync_gridfs_file_free (gfile);

  gfile = mongo_sync_gridfs_file_new_from_buffer (gfs, NULL, data, FILE_SIZE);
  ok (gfile != NULL,
      "GridFS file upload (w/o metadata) works!");
  memcpy (noname_oid, mongo_sync_gridfs_file_get_id (gfile), 12);
  note ("Noname file ID: %s\n", oid_to_string (noname_oid));
  mongo_sync_gridfs_file_free (gfile);

  g_free (data);
  bson_free (meta);
  mongo_sync_gridfs_free (gfs, TRUE);
}

void
validate_file (mongo_sync_gridfs *gfs, const bson *query, guint8 *oid)
{
  mongo_sync_gridfs_file *f;
  mongo_sync_cursor *cursor;
  gint64 n = 0, tsize = 0;

  f = mongo_sync_gridfs_find (gfs, query);

  ok (memcmp (mongo_sync_gridfs_file_get_id (f), oid, 12) == 0,
      "File _id matches");
  cmp_ok (mongo_sync_gridfs_file_get_length (f), "==", FILE_SIZE,
	  "File length matches");
  cmp_ok (mongo_sync_gridfs_file_get_chunk_size (f), "==",
	  mongo_sync_gridfs_get_chunk_size (gfs),
	  "File chunk size matches");

  note ("File info:\n\tid = %s; length = %d; chunk_size = %d; date = %lu; md5 = %s; n = %d\n",
	oid_to_string (mongo_sync_gridfs_file_get_id (f)),
	mongo_sync_gridfs_file_get_length (f),
	mongo_sync_gridfs_file_get_chunk_size (f),
	mongo_sync_gridfs_file_get_date (f),
	mongo_sync_gridfs_file_get_md5 (f),
	mongo_sync_gridfs_file_get_chunks (f));

  cursor = mongo_sync_gridfs_file_cursor_new (f, 0, 0);
  while (mongo_sync_cursor_next (cursor))
    {
      gint32 size;
      guint8 *data;

      data = mongo_sync_gridfs_file_cursor_get_chunk (cursor, &size);
      g_free (data);

      tsize += size;
      n++;
    }
  mongo_sync_cursor_free (cursor);

  cmp_ok (mongo_sync_gridfs_file_get_length (f), "==", tsize,
	  "File size matches the sum of its chunks");
  cmp_ok (mongo_sync_gridfs_file_get_chunks (f), "==", n,
	  "Number of chunks matches the expected number");

  mongo_sync_gridfs_file_free (f);
}

void
test_func_sync_gridfs_get (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  bson *query;

  conn = mongo_sync_connect (config.primary_host, config.primary_port, TRUE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);

  query = bson_build (BSON_TYPE_STRING, "filename", "libmongo-test", -1,
		      BSON_TYPE_NONE);
  bson_finish (query);
  validate_file (gfs, query, named_oid);
  bson_free (query);

  query = bson_build (BSON_TYPE_OID, "_id", noname_oid,
		      BSON_TYPE_NONE);
  bson_finish (query);
  validate_file (gfs, query, noname_oid);
  bson_free (query);

  mongo_sync_gridfs_free (gfs, TRUE);
}

void
test_func_sync_gridfs (void)
{
  mongo_util_oid_init (0);

  test_func_sync_gridfs_put ();
  test_func_sync_gridfs_get ();
}

RUN_NET_TEST (12, func_sync_gridfs);
