#ifndef LIBMONGO_CLIENT_TEST_H
#define LIBMONGO_CLIENT_TEST_H 1

#include "tap.h"

#include <lmc.h>
#include <errno.h>
#include <string.h>

typedef struct
{
  char *primary_host;
  int primary_port;

  char *secondary_host;
  int secondary_port;

  char *db;
  char *coll;
  char *ns;

  char *gfs_prefix;
} func_config_t;

extern func_config_t config;

#define begin_network_tests(n)						\
  do									\
    {									\
      skip(!test_env_setup (), n, "Environment not set up for network tests")

#define end_network_tests()			\
      endskip;					\
      test_env_free();				\
    } while (0)

#define RUN_TEST(n, t) \
  int		       \
  main (void)	       \
  {		       \
    plan (n);	       \
    test_##t ();       \
    return 0;	       \
  }

lmc_bool_t test_env_setup (void);
void test_env_free (void);

#define RUN_NET_TEST(n, t)						\
  int									\
  main (void)								\
  {									\
    if (!test_env_setup ())						\
      printf ("1..0 # skip, Environment not set up for network tests"); \
    else								\
      {									\
	plan (n);							\
	test_##t ();							\
      }									\
    test_env_free ();							\
    return 0;								\
  }

bson_t *test_bson_generate_full (void);

#if 0
mongo_packet *test_mongo_wire_generate_reply (gboolean valid,
					      gint32 nreturn,
					      gboolean with_docs);
mongo_sync_connection *test_make_fake_sync_conn (gint fd,
						 gboolean slaveok);
#endif

#endif
