
/*
 * SHA1 hash implementation and interface functions
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 */

#ifndef _SHA1_H_
#define _SHA1_H_

#include <stdint.h>

#define SHA1_MAC_LEN 20

struct SHA1Context
{
	uint32_t state[5];
	uint32_t count[2];
	unsigned char buffer[64];
};

typedef struct SHA1Context SHA1_CTX;

void SHA1Init(SHA1_CTX *context);

void SHA1Update(SHA1_CTX *context, const void *data, uint32_t len);

void SHA1Final(unsigned char digest[20], SHA1_CTX *context);

void sha1_vector(int num_elem, const uint8_t *addr[], const int *len, uint8_t *mac);

void hmac_sha1_vector(const uint8_t *key, int key_len, int num_elem,
	const uint8_t *addr[], const int *len, uint8_t *mac);

void hmac_sha1(const uint8_t *key, int key_len, const uint8_t *data, int data_len, uint8_t *mac);

#endif
