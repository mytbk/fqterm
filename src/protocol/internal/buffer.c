/*
 * buffer.c: SSH buffer
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

#include "buffer.h"
#include <endian.h>

#define INITSIZE 2048

int buffer_init(buffer *b)
{
	b->p = (uint8_t *)malloc(INITSIZE);
	if (b->p) {
		b->alloc = INITSIZE;
		b->offs = 0;
		b->sz = 0;
		return 1;
	} else {
		return 0;
	}
}

static int ensure(buffer *b, size_t len)
{
	if (b->offs + b->sz + len <= b->alloc)
		return 1;

	uint8_t *r = (uint8_t *)realloc(b->p, (b->alloc + len) * 2);
	if (r == NULL)
		return 0;
	b->alloc = (b->alloc + len) * 2;
	b->p = r;
	return 1;
}

int buffer_append(buffer *b, const uint8_t *s, size_t len)
{
	if (ensure(b, len)) {
		memcpy(b->p + b->offs + b->sz, s, len);
		b->sz += len;
		return 1;
	} else {
		return 0;
	}
}

int buffer_append_string(buffer *b, const char *s, size_t len)
{
	uint32_t beint = htobe32(len);
	if (ensure(b, len + 4)) {
		*(uint32_t *)(b->p + b->offs + b->sz) = beint;
		memcpy(b->p + b->offs + b->sz + 4, s, len);
		b->sz += len + 4;
		return 1;
	} else {
		return 0;
	}
}

int buffer_append_byte(buffer *b, uint8_t x)
{
	if (ensure(b, 1)) {
		b->p[b->offs + b->sz] = x;
		b->sz += 1;
	} else {
		return 0;
	}
}

int buffer_append_be16(buffer *b, uint16_t x)
{
	uint16_t beint = htobe16(x);
	if (ensure(b, 2)) {
		*(uint16_t *)(b->p + b->offs + b->sz) = beint;
		b->sz += 2;
	} else {
		return 0;
	}
}

int buffer_append_be32(buffer *b, uint32_t x)
{
	uint32_t beint = htobe32(x);
	if (ensure(b, 4)) {
		*(uint32_t *)(b->p + b->offs + b->sz) = beint;
		b->sz += 4;
	} else {
		return 0;
	}
}

void buffer_get(buffer *b, uint8_t *s, size_t len)
{
	memcpy(s, buffer_data(b), len);
	buffer_consume(b, len);
}

uint8_t buffer_get_u8(buffer *b)
{
	uint8_t c = *buffer_data(b);
	buffer_consume(b, 1);
	return c;
}

uint16_t buffer_get_u16(buffer *b)
{
	uint16_t u = be16toh(*(uint16_t *)buffer_data(b));
	buffer_consume(b, 2);
	return u;
}

uint32_t buffer_get_u32(buffer *b)
{
	uint32_t u = be32toh(*(uint32_t *)buffer_data(b));
	buffer_consume(b, 4);
	return u;
}
