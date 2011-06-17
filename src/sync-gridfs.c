/* sync-gridfs.c - libmongo-client GridFS implementation
 * Copyright 2011 Gergely Nagy <algernon@balabit.hu>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file src/sync-gridfs.c
 * MongoDB GridFS implementation.
 */

#include "sync-gridfs.h"
#include "libmongo-private.h"

#include <errno.h>

mongo_sync_gridfs *
mongo_sync_gridfs_new (mongo_sync_connection *conn,
		       const gchar *ns_prefix)
{
  mongo_sync_gridfs *gfs;

  if (!conn)
    {
      errno = ENOTCONN;
      return NULL;
    }

  gfs = g_new (mongo_sync_gridfs, 1);
  gfs->conn = conn;
  if (ns_prefix)
    gfs->ns.prefix = g_strdup (ns_prefix);
  else
    gfs->ns.prefix = g_strdup ("fs");

  gfs->ns.files = g_strconcat (gfs->ns.prefix, ".files", NULL);
  gfs->ns.chunks = g_strconcat (gfs->ns.prefix, ".chunks", NULL);

  gfs->chunk_size = 256 * 1024;

  return gfs;
}

void
mongo_sync_gridfs_free (mongo_sync_gridfs *gfs, gboolean disconnect)
{
  if (!gfs)
    {
      errno = ENOTCONN;
      return;
    }

  g_free (gfs->ns.prefix);
  g_free (gfs->ns.files);
  g_free (gfs->ns.chunks);

  if (disconnect)
    mongo_sync_disconnect (gfs->conn);

  g_free (gfs);
  errno = 0;
}

gint32
mongo_sync_gridfs_get_chunk_size (mongo_sync_gridfs *gfs)
{
  if (!gfs)
    {
      errno = ENOTCONN;
      return -1;
    }
  return gfs->chunk_size;
}

gboolean
mongo_sync_gridfs_set_chunk_size (mongo_sync_gridfs *gfs,
				  gint32 chunk_size)
{
  if (!gfs)
    {
      errno = ENOTCONN;
      return FALSE;
    }
  if (chunk_size < 1)
    {
      errno = EINVAL;
      return FALSE;
    }

  gfs->chunk_size = chunk_size;
  return TRUE;
}

void
mongo_sync_gridfs_file_free (mongo_sync_gridfs_file *gfile)
{
  if (!gfile)
    {
      errno = EINVAL;
      return;
    }
  bson_free (gfile->meta);
  g_free (gfile);

  errno = 0;
}

mongo_sync_gridfs_file *
mongo_sync_gridfs_find (mongo_sync_gridfs *gfs, const bson *query)
{
  mongo_sync_gridfs_file *f;
  mongo_packet *p;
  bson_cursor *c;

  if (!gfs)
    {
      errno = ENOTCONN;
      return NULL;
    }
  if (!query)
    {
      errno = EINVAL;
      return NULL;
    }

  p = mongo_sync_cmd_query (gfs->conn, gfs->ns.files, 0, 0, 1, query, NULL);
  if (!p)
    return NULL;

  f = g_new0 (mongo_sync_gridfs_file, 1);
  f->gfs = gfs;

  if (!mongo_wire_reply_packet_get_nth_document (p, 1, &f->meta))
    {
      int e = errno;

      mongo_sync_gridfs_file_free (f);
      errno = e;
      return NULL;
    }
  bson_finish (f->meta);

  c = bson_find (f->meta, "length");
  bson_cursor_get_int32 (c, &f->length);
  bson_cursor_free (c);

  c = bson_find (f->meta, "chunkSize");
  bson_cursor_get_int32 (c, &f->chunk_size);
  bson_cursor_free (c);

  if (f->length == 0 || f->chunk_size == 0)
    {
      mongo_sync_gridfs_file_free (f);
      errno = EPROTO;
      return NULL;
    }

  c = bson_find (f->meta, "_id");
  if (bson_cursor_type (c) != BSON_TYPE_OID)
    {
      mongo_sync_gridfs_file_free (f);
      bson_cursor_free (c);
      errno = EPROTO;
      return NULL;
    }
  bson_cursor_get_oid (c, &f->oid);
  bson_cursor_free (c);

  return f;
}

mongo_sync_cursor *
mongo_sync_gridfs_file_get_chunks (mongo_sync_gridfs_file *gfile,
				   gint start, gint num)
{
  bson *q;
  mongo_sync_cursor *cursor;
  mongo_packet *p;

  if (!gfile)
    {
      errno = ENOTCONN;
      return NULL;
    }
  if (start < 0 || num < 0)
    {
      errno = EINVAL;
      return NULL;
    }

  q = bson_new_sized (32);
  bson_append_oid (q, "files_id", gfile->oid);
  bson_finish (q);

  p = mongo_sync_cmd_query (gfile->gfs->conn, gfile->gfs->ns.chunks, 0,
			    start, num, q, NULL);
  cursor = mongo_sync_cursor_new (gfile->gfs->conn,
				  gfile->gfs->ns.chunks, p);
  bson_free (q);

  return cursor;
}
