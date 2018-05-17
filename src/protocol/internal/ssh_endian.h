/*
 * ssh_endian.h: endian conversion
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

#ifndef SSH_ENDIAN_H
#define SSH_ENDIAN_H

#include <stdint.h>

#if defined(WIN32)
static inline uint16_t bswap_16(uint16_t x)
{
	return (x << 8) | (x >> 8);
}

static inline uint32_t bswap_32(uint32_t x)
{
	return (bswap_16(x) << 16) | bswap_16(x >> 16);
}

#define htobe16 bswap_16
#define be16toh bswap_16
#define htobe32 bswap_32
#define be32toh bswap_32

#else
#include <endian.h>
#endif

static inline uint16_t ntohu16(const unsigned char *buf)
{
	return be16toh(*(uint16_t *)buf);
}

static inline uint32_t ntohu32(const unsigned char *buf)
{
	return be32toh(*(uint32_t *)buf);
}

static inline void htonu32(unsigned char *buf, uint32_t number)
{
	*(uint32_t *)buf = htobe32(number);
}

static inline void htonu16(unsigned char *buf, uint16_t number)
{
	*(uint16_t *)buf = htobe16(number);
}

#endif
