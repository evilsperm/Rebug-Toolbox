
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

#include <string.h>
#include <sha1.h>

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#define blk0(i) (block->l[i])

#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ \
	block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

#define R0(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);

#define R1(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);

#define R2(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);

#define R3(v,w,x,y,z,i) \
	z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
	w = rol(w, 30);

#define R4(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
	w=rol(w, 30);

static void SHA1Transform(uint32_t state[5], const unsigned char buffer[64]);

static void SHA1Transform(uint32_t state[5], const unsigned char buffer[64])
{
	uint32_t a, b, c, d, e;
	typedef union
	{
		uint8_t c[64];
		uint32_t l[16];
	} CHAR64LONG16;
	CHAR64LONG16* block;
	uint8_t workspace[64];

	block = (CHAR64LONG16 *) workspace;

	memcpy(block, buffer, 64);

	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];

	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
 	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;

	a = b = c = d = e = 0;

	memset(block, 0, 64);
}

void SHA1Init(SHA1_CTX* context)
{
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
  context->count[0] = context->count[1] = 0;
}

void SHA1Update(SHA1_CTX* context, const void *_data, uint32_t len)
{
	const unsigned char *data = _data;
	uint32_t i, j;

	j = (context->count[0] >> 3) & 63;

	if ((context->count[0] += len << 3) < (len << 3))
		context->count[1]++;

	context->count[1] += (len >> 29);

	if ((j + len) > 63)
	{
		memcpy(&context->buffer[j], data, (i = 64-j));

		SHA1Transform(context->state, context->buffer);

		for ( ; i + 63 < len; i += 64)
			SHA1Transform(context->state, &data[i]);

		j = 0;
	}
	else
	{
		i = 0;
	}

	memcpy(&context->buffer[j], &data[i], len - i);
}

void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
	unsigned char finalcount[8];
	uint32_t i;

	for (i = 0; i < 8; i++)
		finalcount[i] =
			(unsigned char) ((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);

	SHA1Update(context, (unsigned char *) "\200", 1);

	while ((context->count[0] & 504) != 448)
		SHA1Update(context, (unsigned char *) "\0", 1);

	SHA1Update(context, finalcount, 8);

	for (i = 0; i < 20; i++)
		digest[i] = (unsigned char) ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);

	i = 0;

	memset(context->buffer, 0, 64);
	memset(context->state, 0, 20);
	memset(context->count, 0, 8);
	memset(finalcount, 0, 8);
}

void hmac_sha1_vector(const uint8_t *key, int key_len, int num_elem,
	const uint8_t *addr[], const int *len, uint8_t *mac)
{
	unsigned char k_pad[64];
	unsigned char tk[20];
	const uint8_t *_addr[6];
	int _len[6], i;

	if (num_elem > 5)
		return;

	if (key_len > 64)
	{
		sha1_vector(1, &key, &key_len, tk);
		key = tk;
		key_len = 20;
	}

	memset(k_pad, 0, sizeof(k_pad));
	memcpy(k_pad, key, key_len);

	for (i = 0; i < 64; i++)
		k_pad[i] ^= 0x36;

	_addr[0] = k_pad;
	_len[0] = 64;

	for (i = 0; i < num_elem; i++)
	{
		_addr[i + 1] = addr[i];
		_len[i + 1] = len[i];
	}

	sha1_vector(1 + num_elem, _addr, _len, mac);

	memset(k_pad, 0, sizeof(k_pad));
	memcpy(k_pad, key, key_len);

	for (i = 0; i < 64; i++)
		k_pad[i] ^= 0x5c;

	_addr[0] = k_pad;
	_len[0] = 64;
	_addr[1] = mac;
	_len[1] = SHA1_MAC_LEN;

	sha1_vector(2, _addr, _len, mac);
}

void hmac_sha1(const uint8_t *key, int key_len, const uint8_t *data, int data_len, uint8_t *mac)
{
	hmac_sha1_vector(key, key_len, 1, &data, &data_len, mac);
}

void sha1_vector(int num_elem, const uint8_t *addr[], const int *len, uint8_t *mac)
{
	SHA1_CTX ctx;
	int i;

	SHA1Init(&ctx);

	for (i = 0; i < num_elem; i++)
    	SHA1Update(&ctx, addr[i], len[i]);

	SHA1Final(mac, &ctx);
}

