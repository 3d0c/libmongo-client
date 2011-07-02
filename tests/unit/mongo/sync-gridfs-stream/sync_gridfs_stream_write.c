#include "test.h"
#include "mongo.h"

void
test_mongo_sync_gridfs_stream_write (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;
  mongo_sync_gridfs_stream *stream;
  bson *meta;
  guint8 buffer[4096];

  meta = bson_build (BSON_TYPE_STRING, "my-id", "sync_gridfs_stream_write", -1,
		     BSON_TYPE_NONE);
  bson_finish (meta);

  ok (mongo_sync_gridfs_stream_write (NULL, buffer, sizeof (buffer)) == FALSE,
      "mongo_sync_gridfs_stream_write() should fail with a NULL connection");

  begin_network_tests (3);

  conn = mongo_sync_connect (config.primary_host, config.primary_port, FALSE);
  gfs = mongo_sync_gridfs_new (conn, config.gfs_prefix);

  stream = mongo_sync_gridfs_stream_new (gfs, meta);

  ok (mongo_sync_gridfs_stream_write (stream, NULL, sizeof (buffer)) == FALSE,
      "mongo_sync_gridfs_stream_write() should fail with a NULL buffer");
  ok (mongo_sync_gridfs_stream_write (stream, buffer, 0) == FALSE,
      "mongo_sync_gridfs_stream_write() should fail with 0 size");
  ok (mongo_sync_gridfs_stream_write (stream, buffer, sizeof (buffer)) == TRUE,
      "mongo_sync_gridfs_stream_write() works");

  mongo_sync_gridfs_stream_close (stream);
  mongo_sync_gridfs_free (gfs, TRUE);

  end_network_tests ();

  bson_free (meta);
}

RUN_TEST (4, mongo_sync_gridfs_stream_write);
