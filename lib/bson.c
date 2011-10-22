/* lib/bson.c - BSON API
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

/** @file lib/bson.c
 */

#include <lmc/common.h>
#include <lmc/endian.h>
#include <lmc/bson.h>

#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>

typedef struct _bson_node_t bson_node_t;

struct _bson_node_t
{
  bson_element_t *e;
  bson_node_t *next;
};

struct _bson_t
{
  sig_atomic_t ref;
  lmc_bool_t closed;

  struct
  {
    uint32_t len;
    bson_node_t *head;
  } elements;
  struct
  {
    uint32_t alloc;
    uint32_t len;
    uint8_t data[0];
  } stream;
};

bson_t *
bson_new (void)
{
  bson_t *b;

  b = (bson_t *)calloc (1, sizeof (bson_t));

  b->ref = 1;

  return b;
}

bson_t *
bson_ref (bson_t *b)
{
  if (b)
    b->ref++;
  return b;
}

bson_t *
bson_unref (bson_t *b)
{
  if (!b)
    return NULL;
  b->ref--;
  if (b->ref <= 0)
    {
      if (b->elements.len > 0)
	{
	  bson_node_t *o = b->elements.head;

	  do
	    {
	      bson_node_t *n = o->next;

	      bson_element_unref (o->e);
	      free (o);
	      o = n;
	    }
	  while (o);
	}

      free (b);
      return NULL;
    }
  return b;
}

bson_t *
bson_open (bson_t *b)
{
  if (b)
    b->closed = FALSE;
  return b;
}

bson_t *
bson_close (bson_t *b)
{
  if (b)
    b->closed = TRUE;
  return b;
}

uint32_t
bson_length (bson_t *b)
{
  if (!b)
    return 0;
  return b->elements.len;
}

static inline bson_node_t *
_bson_node_get_nth (bson_t *b, uint32_t n)
{
  uint32_t i;
  bson_node_t *node = b->elements.head;

  for (i = 1; i < n; i++)
    node = node->next;

  return node;
}

static inline bson_t *
bson_append_va (bson_t *b, va_list ap)
{
  bson_node_t *s, *t, dummy;
  bson_element_t *e;
  uint32_t c = 0;

  if (!b || b->closed)
    return b;

  s = _bson_node_get_nth (b, b->elements.len);
  if (!s)
    s = &dummy;
  t = s;

  while ((e = va_arg (ap, bson_element_t *)) != NULL)
    {
      bson_node_t *n = (bson_node_t *)malloc (sizeof (bson_node_t));
      n->e = e;
      n->next = NULL;

      t->next = n;
      t = t->next;
      c++;
    }
  b->elements.len += c;
  if (!b->elements.head)
    b->elements.head = s->next;

  return b;
}

bson_t *
bson_append (bson_t *b, ...)
{
  va_list ap;

  va_start (ap, b);
  b = bson_append_va (b, ap);
  va_end (ap);
  return b;
}
