/* sync-gridfs.h - libmong-client GridFS API
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

/** @file src/sync-gridfs.h
 * MongoDB GridFS API.
 *
 * @addtogroup mongo_sync
 * @{
 */

#ifndef LIBMONGO_SYNC_GRIDFS_H
#define LIBMONGO_SYNC_GRIDFS_H 1

#include <mongo-sync.h>
#include <glib.h>

G_BEGIN_DECLS

/** @defgroup mongo_sync_gridfs_api Mongo GridFS API
 *
 * @addtogroup mongo_sync_gridfs_api
 * @{
 */

/** Opaque GridFS object. */
typedef struct _mongo_sync_gridfs mongo_sync_gridfs;

/** Create a new GridFS object.
 *
 * @param conn is the MongoDB connection to base the filesystem object
 * on.
 * @param ns_prefix is the prefix the GridFS collections should be
 * under.
 *
 * @returns A newly allocated GridFS object, or NULL on error.
 */
mongo_sync_gridfs *mongo_sync_gridfs_new (mongo_sync_connection *conn,
					  const gchar *ns_prefix);

/** Close and free a GridFS object.
 *
 * @param gfs is the GridFS object to free up.
 * @param disconnect signals whether to free the underlying connection
 * aswell.
 */
void mongo_sync_gridfs_free (mongo_sync_gridfs *gfs, gboolean disconnect);

/** Get the default chunk size of a GridFS object.
 *
 * @param gfs is the GridFS object to get the default chunk size of.
 *
 * @returns The chunk size in bytes, or -1 on error.
 */
gint32 mongo_sync_gridfs_get_chunk_size (mongo_sync_gridfs *gfs);

/** Set the default chunk size of a GridFS object.
 *
 * @param gfs is the GridFS object to set the default chunk size of.
 * @param chunk_size is the desired default chunk size.
 *
 * @returns TRUE on success, FALSE otherwise.
 */
gboolean mongo_sync_gridfs_set_chunk_size (mongo_sync_gridfs *gfs,
					   gint32 chunk_size);

G_END_DECLS

/** @} */

/** @} */

#endif
