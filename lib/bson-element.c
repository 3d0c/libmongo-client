/* lib/bson-element.c - BSON element API
 * Copyright (C) 2011 Gergely Nagy <algernon@balabit.hu>
 * This file is part of the libmongo-client library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */

/** @file lib/bson-element.c
 */

#include <lmc/common.h>
#include <lmc/endian.h>
#include <lmc/bson-element.h>

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>

#define BSON_ELEMENT_NAME(e) ((char *)(e->as_typed.data))
#define BSON_ELEMENT_VALUE(e) \
  ((bson_element_value_t *)(e->as_typed.data + e->name_len))

typedef union
{
  double dbl;
  int32_t i32;
  struct
  {
    int32_t len;
    char str[0];
  } str;
} bson_element_value_t;

struct _bson_element_t
{
  sig_atomic_t ref;
  uint32_t alloc;
  uint32_t len;
  uint32_t name_len;

  union
  {
    struct
    {
      uint8_t type;
      uint8_t data[0];
    } as_typed;
    uint8_t bytestream[0];
  };
};

bson_element_t *
bson_element_new_sized (uint32_t size)
{
  bson_element_t *e;

  e = (bson_element_t *)malloc (sizeof (bson_element_t) + size);
  memset (e, 0, sizeof (bson_element_t));

  e->ref = 1;
  e->as_typed.type = BSON_TYPE_NONE;
  e->alloc = size;
  e->len = 0;
  e->name_len = 0;

  return e;
}

bson_element_t *
bson_element_new (void)
{
  return bson_element_new_sized (0);
}

bson_element_t *
bson_element_ref (bson_element_t *e)
{
  if (e)
    e->ref++;
  return e;
}

bson_element_t *
bson_element_unref (bson_element_t *e)
{
  if (!e)
    return NULL;

  e->ref--;
  if (e->ref <= 0)
    {
      free (e);
      return NULL;
    }
  return e;
}

bson_element_type_t
bson_element_type_get (bson_element_t *e)
{
  if (!e)
    return BSON_TYPE_NONE;
  return e->as_typed.type;
}

bson_element_t *
bson_element_type_set (bson_element_t *e,
		       bson_element_type_t type)
{
  if (!e)
    return NULL;

  e->as_typed.type = type;
  return e;
}

const char *
bson_element_name_get (bson_element_t *e)
{
  if (!e)
    return NULL;
  return BSON_ELEMENT_NAME (e);
}

static inline bson_element_t *
bson_element_ensure_space (bson_element_t *e, uint32_t size)
{
  if (e->alloc < size)
    {
      e = (bson_element_t *)realloc (e, sizeof (bson_element_t) + size);
      e->alloc = size;
    }
  e->len = size;
  return e;
}

bson_element_t *
bson_element_name_set (bson_element_t *e,
		       const char *name)
{
  if (!e)
    return NULL;

  if (name)
    {
      size_t name_len = strlen (name) + 1;
      uint32_t size = e->len - e->name_len;

      e = bson_element_ensure_space (e, size + name_len);

      memmove (e->as_typed.data + name_len,
	       e->as_typed.data + e->name_len,
	       size + 1);
      memcpy (e->as_typed.data, name, name_len);
      e->name_len = name_len;

      return e;
    }

  memmove (e->as_typed.data,
	   e->as_typed.data + e->name_len - 1,
	   e->len - e->name_len + 1);
  e->len -= e->name_len;
  e->name_len = 0;
  return e;
}

const uint8_t *
bson_element_data_get (bson_element_t *e)
{
  if (!e || e->len == 0)
    return NULL;
  return e->as_typed.data + e->name_len + 1;
}

int32_t
bson_element_data_get_size (bson_element_t *e)
{
  if (!e)
    return -1;
  return e->len - e->name_len;
}

bson_element_t *
bson_element_data_reset (bson_element_t *e)
{
  if (!e)
    return NULL;
  e->len = e->name_len;
  return e;
}

bson_element_t *
bson_element_data_append (bson_element_t *e, const uint8_t *data,
			  uint32_t size)
{
  if (!e || !data || size == 0)
    return e;

  e = bson_element_ensure_space (e, e->len + size);
  memcpy (e->as_typed.data + e->len - size + 1, data, size);
  return e;
}

bson_element_t *
bson_element_data_set (bson_element_t *e, const uint8_t *data,
		       uint32_t size)
{
  if (!e)
    return NULL;
  bson_element_data_reset (e);
  return bson_element_data_append (e, data, size);
}

const uint8_t *
bson_element_stream_get (bson_element_t *e)
{
  if (!e)
    return NULL;
  return e->bytestream;
}

int32_t
bson_element_stream_get_size (bson_element_t *e)
{
  if (!e)
    return -1;
  return e->len + e->name_len + 1;
}

static inline bson_element_value_t *
bson_element_data_type_get (bson_element_t *e, bson_element_type_t type)
{
  if (bson_element_type_get (e) != type || e->len == 0)
    return NULL;
  return BSON_ELEMENT_VALUE (e);
}

bson_element_t *
bson_element_value_set_double (bson_element_t *e,
			       double val)
{
  if (!e)
    return NULL;

  e = bson_element_type_set (e, BSON_TYPE_DOUBLE);
  e = bson_element_ensure_space (e, sizeof (double));
  BSON_ELEMENT_VALUE (e)->dbl = LMC_DOUBLE_TO_LE (val);
  return e;
}

lmc_bool_t
bson_element_value_get_double (bson_element_t *e,
			       double *oval)
{
  bson_element_value_t *v = bson_element_data_type_get (e, BSON_TYPE_DOUBLE);

  if (!v || !oval)
    return FALSE;

  *oval = LMC_DOUBLE_FROM_LE (v->dbl);
  return TRUE;
}

bson_element_t *
bson_element_value_set_int32 (bson_element_t *e,
			      int32_t val)
{
  if (!e)
    return NULL;

  e = bson_element_type_set (e, BSON_TYPE_INT32);
  e = bson_element_ensure_space (e, sizeof (int32_t));
  BSON_ELEMENT_VALUE (e)->i32 = LMC_INT32_TO_LE (val);
  return e;
}

lmc_bool_t
bson_element_value_get_int32 (bson_element_t *e,
			      int32_t *oval)
{
  bson_element_value_t *v = bson_element_data_type_get (e, BSON_TYPE_INT32);

  if (!v || !oval)
    return FALSE;

  *oval = LMC_INT32_FROM_LE (v->i32);
  return TRUE;
}

bson_element_t *
bson_element_value_set_string (bson_element_t *e,
			       const char *val,
			       int32_t length)
{
  int32_t l = length;

  if (!e)
    return NULL;

  if (l < 0)
    l = strlen (val);

  e = bson_element_type_set (e, BSON_TYPE_STRING);
  e = bson_element_ensure_space (e, sizeof (int32_t));
  BSON_ELEMENT_VALUE (e)->str.len = LMC_INT32_TO_LE (l);
  e->len--;

  e = bson_element_data_append (e, (uint8_t *)val, l);
  return bson_element_data_append (e, (uint8_t *)"\0", 1);
}

lmc_bool_t
bson_element_value_get_string (bson_element_t *e,
			       const char **oval)
{
  bson_element_value_t *v = bson_element_data_type_get (e, BSON_TYPE_STRING);

  if (!v || !oval)
    return FALSE;

  *oval = v->str.str;
  return TRUE;
}

static bson_element_t *
bson_element_value_set_va (bson_element_t *e, bson_element_type_t type,
			   va_list ap)
{
  switch (type)
    {
    case BSON_TYPE_NONE:
    case BSON_TYPE_MIN:
    case BSON_TYPE_MAX:
      break;
    case BSON_TYPE_DOUBLE:
      e = bson_element_value_set_double (e, va_arg (ap, double));
      break;
    case BSON_TYPE_STRING:
      {
	char *s = va_arg (ap, char *);
	e = bson_element_value_set_string (e, s, va_arg (ap, int32_t));
	break;
      }
    case BSON_TYPE_INT32:
      e = bson_element_value_set_int32 (e, va_arg (ap, int32_t));
      break;
    default:
      break;
    }
  return e;
}

bson_element_t *
bson_element_value_set (bson_element_t *e,
			bson_element_type_t type, ...)
{
  va_list ap;

  if (!e)
    return NULL;

  va_start (ap, type);
  e = bson_element_value_set_va (e, type, ap);
  va_end (ap);
  return e;
}

bson_element_t *
bson_element_set (bson_element_t *e, const char *name,
		  bson_element_type_t type, ...)
{
  va_list ap;

  if (!e)
    return NULL;

  e = bson_element_name_set (e, name);
  va_start (ap, type);
  e = bson_element_value_set_va (e, type, ap);
  va_end (ap);
  return e;
}

bson_element_t *
bson_element_create (const char *name,
		     bson_element_type_t type, ...)
{
  va_list ap;
  bson_element_t *e;

  e = bson_element_name_set (bson_element_new (), name);
  va_start (ap, type);
  e = bson_element_value_set_va (e, type, ap);
  va_end (ap);

  if (bson_element_type_get (e) != type)
    e = bson_element_unref (e);

  return e;
}
