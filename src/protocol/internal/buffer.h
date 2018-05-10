/*
 * buffer.h: SSH buffer
 * Copyright (C) 2018  Iru Cai <mytbk920423@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SSH_BUFFER_MAX 10000000

#ifdef __cplusplus
extern "C" {
#endif /* } */

typedef struct
{
	uint8_t *p;
	size_t alloc;
	size_t offs;
	size_t sz;
} buffer;

/* if success, return 1, otherwise return 0 */
int buffer_init(buffer *);
static inline void buffer_clear(buffer *b) { b->offs = b->sz = 0; }
static inline void buffer_deinit(buffer *b) { free(b->p); }
static inline uint8_t *buffer_data(buffer *b) { return b->p + b->offs; }
static inline size_t buffer_len(buffer *b) { return b->sz; }
int buffer_append(buffer *b, const uint8_t *s, size_t len);
int buffer_append_string(buffer *b, const char *s, size_t len);
int buffer_append_byte(buffer *b, uint8_t);
int buffer_append_be16(buffer *b, uint16_t);
int buffer_append_be32(buffer *b, uint32_t);

/* the following functions doesn't check on buffer size */
static inline void buffer_consume(buffer *b, size_t L)
{
	b->offs += L;
	b->sz -= L;
}
void buffer_get(buffer *b, uint8_t *s, size_t len);
uint32_t buffer_get_u32(buffer *b);
uint16_t buffer_get_u16(buffer *b);
uint8_t buffer_get_u8(buffer *b);
#ifdef __cplusplus
}
#endif

#endif
