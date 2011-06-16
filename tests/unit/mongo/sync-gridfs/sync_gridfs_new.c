#include "test.h"
#include "mongo.h"

#include <errno.h>

void
test_mongo_sync_gridfs_new (void)
{
  mongo_sync_connection *conn;
  mongo_sync_gridfs *gfs;

  conn = test_make_fake_sync_conn (4, TRUE);

  ok (mongo_sync_gridfs_new (NULL, "test.fs") == NULL,
      "mongo_sync_gridfs_new() should fail with a NULL connection");

  gfs = mongo_sync_gridfs_new (conn, "test.fs");
  ok (gfs != NULL,
      "mongo_sync_gridfs_new() works");
  is (gfs->ns.prefix, "test.fs",
      "The namespace prefix is as specified");
  is (gfs->ns.files, "test.fs.files",
      "The files namespace is correct");
  is (gfs->ns.chunks, "test.fs.chunks",
      "The chunks namespace is correct");
  mongo_sync_gridfs_free (gfs, FALSE);

  gfs = mongo_sync_gridfs_new (conn, NULL);
  ok (gfs != NULL,
      "mongo_sync_gridfs_new() works");
  is (gfs->ns.prefix, "fs",
      "The namespace prefix is the default");
  is (gfs->ns.files, "fs.files",
      "The files namespace is correct");
  is (gfs->ns.chunks, "fs.chunks",
      "The chunks namespace is correct");
  mongo_sync_gridfs_free (gfs, FALSE);

  mongo_sync_disconnect (conn);
}

RUN_TEST (9, mongo_sync_gridfs_new);
