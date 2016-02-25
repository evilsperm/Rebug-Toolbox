
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _FLASH_OS_AREA_H_
#define _FLASH_OS_AREA_H_

#include <stdint.h>

#define OS_AREA_SEGMENT_SIZE			0x200

#define HEADER_MAGIC				"cell_ext_os_area"
#define HEADER_VERSION				1

#define DB_MAGIC				0x2d64622du
#define DB_VERSION				1

enum os_area_ldr_format {
	HEADER_LDR_FORMAT_RAW = 0,
	HEADER_LDR_FORMAT_GZIP = 1,
};

enum os_area_boot_flag {
	PARAM_BOOT_FLAG_GAME_OS = 0,
	PARAM_BOOT_FLAG_OTHER_OS = 1,
};

struct os_area_header {
	uint8_t magic[16];
	uint32_t version;
	uint32_t db_area_offset;
	uint32_t ldr_area_offset;
	uint32_t res1;
	uint32_t ldr_format;
	uint32_t ldr_size;
	uint32_t res2[6];
};

struct os_area_params {
	uint32_t boot_flag;
	uint32_t res1[3];
	uint32_t num_params;
	uint32_t res2[3];
	/* param 0 */
	int64_t rtc_diff;
	uint8_t av_multi_out;
	uint8_t ctrl_button;
	uint8_t res3[6];
	/* param 1 */
	uint8_t static_ip_addr[4];
	uint8_t network_mask[4];
	uint8_t default_gateway[4];
	uint8_t res4[4];
	/* param 2 */
	uint8_t dns_primary[4];
	uint8_t dns_secondary[4];
	uint8_t res5[8];
};

struct os_area_db {
	uint32_t magic;
	uint16_t version;
	uint16_t res1;
	uint16_t index_64;
	uint16_t count_64;
	uint16_t index_32;
	uint16_t count_32;
	uint16_t index_16;
	uint16_t count_16;
	uint32_t res2;
	uint8_t res3[1000];
};

#endif
