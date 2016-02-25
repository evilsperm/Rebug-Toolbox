/*-
 * Copyright (C) 2011, 2012 glevand <geoffrey.levand@mail.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PS3DM_MSG_H
#define _PS3DM_MSG_H

#define PS3DM_PID(fid, id)	((fid) | (id))
#define PS3DM_FID(pid)		((pid) & ~0xffful)

enum ps3dm_function_packet {
	/* Virtual TRM */

	PS3DM_FID_VTRM				= 0x00002000,
	PS3DM_PID_VTRM_INIT			= PS3DM_PID(PS3DM_FID_VTRM, 1),
	PS3DM_PID_VTRM_GET_STATUS		= PS3DM_PID(PS3DM_FID_VTRM, 2),
	PS3DM_PID_VTRM_STORE_WITH_UPDATE	= PS3DM_PID(PS3DM_FID_VTRM, 3),
	PS3DM_PID_VTRM_STORE			= PS3DM_PID(PS3DM_FID_VTRM, 4),
	PS3DM_PID_VTRM_RETRIEVE			= PS3DM_PID(PS3DM_FID_VTRM, 5),

	/* Secure RTC */

	PS3DM_FID_SRTC				= 0x00003000,
	PS3DM_PID_SRTC_GET_TIME			= PS3DM_PID(PS3DM_FID_SRTC, 2),
	PS3DM_PID_SRTC_SET_TIME			= PS3DM_PID(PS3DM_FID_SRTC, 3),

	/* Storage manager */

	PS3DM_FID_SM				= 0x00005000,
	PS3DM_PID_SM_SET_ENCDEC_KEY		= PS3DM_PID(PS3DM_FID_SM, 1),
	PS3DM_PID_SM_SET_DEL_ENCDEC_KEY		= PS3DM_PID(PS3DM_FID_SM, 2),
	PS3DM_PID_SM_GET_RND			= PS3DM_PID(PS3DM_FID_SM, 3),
	PS3DM_PID_SM_DRIVE_AUTH			= PS3DM_PID(PS3DM_FID_SM, 4),
	PS3DM_PID_SM_GET_VERSION		= PS3DM_PID(PS3DM_FID_SM, 6),

	/* Update manager */

	PS3DM_FID_UM				= 0x00006000,
	PS3DM_PID_UM_READ_EPROM			= PS3DM_PID(PS3DM_FID_UM, 0xb),
	PS3DM_PID_UM_WRITE_EPROM		= PS3DM_PID(PS3DM_FID_UM, 0xc),

	/* SC manager */

	PS3DM_FID_SCM				= 0x00009000,
	PS3DM_PID_SCM_GET_REGION_DATA		= PS3DM_PID(PS3DM_FID_SCM, 6),
	PS3DM_PID_SCM_READ_EEPROM			= PS3DM_PID(PS3DM_FID_SCM, 0xB),
	PS3DM_PID_SCM_WRITE_EEPROM			= PS3DM_PID(PS3DM_FID_SCM, 0xC),

	/* Indi Info manager */

	PS3DM_FID_IIM				= 0x00017000,
	PS3DM_PID_IIM_GET_DATA_SIZE		= PS3DM_PID(PS3DM_FID_IIM, 1),
	PS3DM_PID_IIM_GET_DATA			= PS3DM_PID(PS3DM_FID_IIM, 2),

	/* AIM */

	PS3DM_FID_AIM				= 0x00019000,
	PS3DM_PID_AIM_GET_DEV_TYPE		= PS3DM_PID(PS3DM_FID_AIM, 2),
	PS3DM_PID_AIM_GET_DEV_ID		= PS3DM_PID(PS3DM_FID_AIM, 3),
};

struct ps3dm_header {
	uint32_t tag;
	uint32_t fid;			/* enum ps3dm_function_packet */
	uint32_t payload_length;
	uint32_t reply_length;
};

#define PS3DM_HDR(_p)	((struct ps3dm_header *) (_p))

struct ps3dm_ss_header {
	uint64_t pid;			/* enum ps3dm_function_packet */
	uint64_t fid;			/* enum ps3dm_function_packet */
	uint32_t status;
	uint8_t res[4];
	uint64_t laid;
	uint64_t paid;
};

#define PS3DM_SS_HDR(_p)	((struct ps3dm_ss_header *) (_p))

struct ps3dm_vtrm_init {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
};

struct ps3dm_vtrm_store_with_update {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint8_t data[64];
};

struct ps3dm_vtrm_retrieve {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* out */
	uint8_t data[64];
	/* in */
	uint64_t nth;
};

struct ps3dm_srtc_get_time {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint64_t epoch;
	/* out */
	uint64_t time;
	uint64_t status;
};

struct ps3dm_srtc_set_time {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint64_t time;
};

struct ps3dm_sm_set_encdec_key {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint8_t key[24];
	uint64_t key_length;
	/* out */
	uint64_t key_index;
};

struct ps3dm_sm_set_del_encdec_key {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint64_t arg;
};

struct ps3dm_sm_get_rnd {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* out */
	uint8_t rnd[24];
};

struct ps3dm_sm_drive_auth {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint64_t arg;
};

struct ps3dm_sm_get_version {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* out */
	uint8_t version[8];
};

struct ps3dm_um_read_eprom {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint32_t offset;
	uint8_t res[4];
	/* out */
	uint8_t data;
};

struct ps3dm_um_write_eprom {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint32_t offset;
	uint8_t res[4];
	uint8_t data;
};

struct ps3dm_scm_get_region_data {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint64_t id;
	uint64_t data_size;
	/* out */
	uint8_t data[48];
};

struct ps3dm_scm_write_eeprom {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	u32 offset;
	u8 res1[4];
	u32 nwrite;
	u8 res2[4];
	u64 buf_size;
	u8 buf[0x50];
};

struct ps3dm_iim_get_data_size {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint64_t index;
	/* out */
	uint64_t size;
};

struct ps3dm_iim_get_data {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* in */
	uint64_t index;
	uint64_t buf_size;
	/* out */
	uint8_t buf[0];
	/* uint64_t data_size */
};

struct ps3dm_aim_get_dev_type {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* out */
	uint8_t data[16];
};

struct ps3dm_aim_get_dev_id {
	struct ps3dm_header dm_hdr;
	struct ps3dm_ss_header ss_hdr;
	/* out */
	uint8_t data[16];
};

static inline void
ps3dm_init_header(struct ps3dm_header *hdr, uint32_t tag, uint32_t fid,
    uint32_t payload_length, uint32_t reply_length)
{
	hdr->tag = tag;
	hdr->fid = fid;
	hdr->payload_length = payload_length;
	hdr->reply_length = reply_length;
}

static inline void
ps3dm_init_ss_header( struct ps3dm_ss_header *hdr, uint64_t pid,
    uint64_t laid, uint64_t paid)
{
	hdr->pid = pid;
	hdr->fid = PS3DM_FID(pid);
	hdr->laid = laid;
	hdr->paid = paid;
}

#endif /* _PS3DM_MSG_H */
