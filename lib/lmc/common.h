/* lmc/common.h - common macros for libmongo-client
 * Copyright (C) 2011 Gergely Nagy <algernon@balabit.hu>
 * This file is part of the libmongo-client library.
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/** @file lib/lmc/common.h
 * Common macros for libmongo-client.
 */

#ifndef LMC_COMMON_H
#define LMC_COMMON_H 1

#include <inttypes.h>

/* Guard C code in headers, while including them from C++ */
#ifdef  __cplusplus
# define LMC_BEGIN_DECLS  extern "C" {
# define LMC_END_DECLS    }
#else
# define LMC_BEGIN_DECLS
# define LMC_END_DECLS
#endif

/** @defgroup lmc_common Common Macros
 * @addtogroup lmc_common
 * @{
 */

/** Boolean values.
 */
typedef enum
{
  FALSE = 0, /**< False value. */
  TRUE = 1 /**< True value. */
} lmc_bool_t;

/** @} */

#endif
