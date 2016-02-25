#ifndef H_COMMON
#define H_COMMON

#define MAXPATHLEN 1024

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lv2.h"

// LV2
#define HVSC_SYSCALL				811                  	// which syscall to overwrite with hvsc redirect
#define HVSC_SYSCALL_ADDR_341		0x80000000001BB414ULL	// where above syscall is in lv2 3.41
#define HVSC_SYSCALL_ADDR_355		0x8000000000195540ULL	// where above syscall is in lv2 3.55
#define HVSC_SYSCALL_ADDR_355D		0x800000000019B8FCULL	// where above syscall is in lv2 3.55 DEX
#define HVSC_SYSCALL_ADDR_421		0x80000000001B60A0ULL	// where above syscall is in lv2 4.21
#define HVSC_SYSCALL_ADDR_421D		0x80000000001BC490ULL	// where above syscall is in lv2 4.21 DEX
#define HVSC_SYSCALL_ADDR_430		0x80000000001B6428ULL	// where above syscall is in lv2 4.30 // Fixed
#define HVSC_SYSCALL_ADDR_430D		0x80000000001BC818ULL	// where above syscall is in lv2 4.30 DEX
#define HVSC_SYSCALL_ADDR_431		0x80000000001B6430ULL	// where above syscall is in lv2 4.31
#define HVSC_SYSCALL_ADDR_440		0x80000000001B540CULL	// where above syscall is in lv2 4.40 // Fixed
#define HVSC_SYSCALL_ADDR_441		0x80000000001B5414ULL	// where above syscall is in lv2 4.41 // Fixed
#define HVSC_SYSCALL_ADDR_441D		0x80000000001BB804ULL	// where above syscall is in lv2 4.41 DEX
#define HVSC_SYSCALL_ADDR_446		0x80000000001B5674ULL	// where above syscall is in lv2 4.46 // Fixed
#define HVSC_SYSCALL_ADDR_446D		0x80000000001BBA64ULL	// where above syscall is in lv2 4.46 DEX
#define HVSC_SYSCALL_ADDR_450		0x80000000001A6750ULL	// where above syscall is in lv2 4.50 // Fixed
#define HVSC_SYSCALL_ADDR_450D		0x80000000001ACC00ULL	// where above syscall is in lv2 4.50 DEX
#define HVSC_SYSCALL_ADDR_453		0x80000000001A68B8ULL	// where above syscall is in lv2 4.53 // Fixed
#define HVSC_SYSCALL_ADDR_453D		0x80000000001ACD68ULL	// where above syscall is in lv2 4.53 DEX
#define HVSC_SYSCALL_ADDR_455		0x80000000001A7DA4ULL	// where above syscall is in lv2 4.55 // Fixed	
#define HVSC_SYSCALL_ADDR_455D		0x80000000001AE254ULL	// where above syscall is in lv2 4.55 DEX
#define HVSC_SYSCALL_ADDR_460		0x80000000001A6A14ULL	// where above syscall is in lv2 4.60 // Fixed		
#define HVSC_SYSCALL_ADDR_465		0x80000000001A6A1CULL	// where above syscall is in lv2 4.65 // Fixed
#define HVSC_SYSCALL_ADDR_465D		0x80000000001ACECCULL	// where above syscall is in lv2 4.65 DEX <- peek( SYSCALL_TABLE_465D + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_466		0x80000000001A6A1CULL	// where above syscall is in lv2 4.66 // Fixed
#define HVSC_SYSCALL_ADDR_466D		0x80000000001ACECCULL	// where above syscall is in lv2 4.65 DEX <- peek( SYSCALL_TABLE_465D + HVSC_SYSCALL*8)
#define HVSC_SYSCALL_ADDR_470		0x80000000002A0EE0ULL	// where above syscall is in lv2 4.70 // 
#define HVSC_SYSCALL_ADDR_470D		0x80000000002BC084ULL	// where above syscall is in lv2 4.70 DEX
#define HVSC_SYSCALL_ADDR_475		0x80000000002A0F58ULL	// where above syscall is in lv2 4.75, 4.76, 4.78 // 
#define HVSC_SYSCALL_ADDR_475D		0x80000000002BC0FCULL	// where above syscall is in lv2 4.75, 4.76, 4.78 DEX
//#define HVSC_SYSCALL_ADDR_476		0x80000000002A0F58ULL	// where above syscall is in lv2 4.76 // 
//#define HVSC_SYSCALL_ADDR_476D		0x80000000002BC0FCULL	// where above syscall is in lv2 4.76 DEX

#define NEW_POKE_SYSCALL			813                  	// which syscall to overwrite with new poke
#define NEW_POKE_SYSCALL_ADDR_341	0x80000000001BB93CULL	// where above syscall is in lv2 3.41
#define NEW_POKE_SYSCALL_ADDR_355	0x8000000000195A68ULL	// where above syscall is in lv2 3.55
#define NEW_POKE_SYSCALL_ADDR_355D	0x800000000019BE24ULL	// where above syscall is in lv2 3.55 DEX
#define NEW_POKE_SYSCALL_ADDR_421	0x80000000001B65C8ULL	// where above syscall is in lv2 4.21
#define NEW_POKE_SYSCALL_ADDR_421D	0x80000000001BC71CULL	// where above syscall is in lv2 4.21 DEX
#define NEW_POKE_SYSCALL_ADDR_430	0x80000000001B6950ULL	// where above syscall is in lv2 4.30 // Fixed
#define NEW_POKE_SYSCALL_ADDR_430D	0x80000000001BCD40ULL	// where above syscall is in lv2 4.30 DEX
#define NEW_POKE_SYSCALL_ADDR_431	0x80000000001B6958ULL	// where above syscall is in lv2 4.31
#define NEW_POKE_SYSCALL_ADDR_440	0x80000000001B5934ULL	// where above syscall is in lv2 4.40 // Fixed
#define NEW_POKE_SYSCALL_ADDR_441	0x80000000001B593CULL	// where above syscall is in lv2 4.41 // Fixed
#define NEW_POKE_SYSCALL_ADDR_441D  0x80000000001BBD2CULL   // where above syscall is in lv2 4.41 DEX
#define NEW_POKE_SYSCALL_ADDR_446	0x80000000001B5B9CULL	// where above syscall is in lv2 4.46 // Fixed
#define NEW_POKE_SYSCALL_ADDR_446D	0x80000000001BBF8CULL	// where above syscall is in lv2 4.46 DEX
#define NEW_POKE_SYSCALL_ADDR_450	0x80000000001A6C78ULL	// where above syscall is in lv2 4.50 // Fixed
#define NEW_POKE_SYSCALL_ADDR_450D  0x80000000001AD128ULL   // where above syscall is in lv2 4.50 DEX
#define NEW_POKE_SYSCALL_ADDR_453	0x80000000001A6DE0ULL	// where above syscall is in lv2 4.53 // Fixed
#define NEW_POKE_SYSCALL_ADDR_453D	0x80000000001AD290ULL	// where above syscall is in lv2 4.53 DEX
#define NEW_POKE_SYSCALL_ADDR_455	0x80000000001A82CCULL	// where above syscall is in lv2 4.55 // Fixed	
#define NEW_POKE_SYSCALL_ADDR_455D	0x80000000001AE77CULL	// where above syscall is in lv2 4.55 DEX
#define NEW_POKE_SYSCALL_ADDR_460	0x80000000001A6F3CULL	// where above syscall is in lv2 4.60 // Fixed
#define NEW_POKE_SYSCALL_ADDR_465	0x80000000001A6F44ULL	// where above syscall is in lv2 4.65 // Fixed
#define NEW_POKE_SYSCALL_ADDR_465D	0x80000000001AD3F4ULL	// where above syscall is in lv2 4.65 DEX 
#define NEW_POKE_SYSCALL_ADDR_466	0x80000000001A6F44ULL	// where above syscall is in lv2 4.66 // Fixed
#define NEW_POKE_SYSCALL_ADDR_466D	0x80000000001AD3F4ULL	// where above syscall is in lv2 4.66 DEX <- peek( SYSCALL_TABLE_466D + NEW_POKE_SYSCALL*8)
#define NEW_POKE_SYSCALL_ADDR_470	0x80000000002A1408ULL	// where above syscall is in lv2 4.70
#define NEW_POKE_SYSCALL_ADDR_470D	0x80000000002BC5ACULL   // where above syscall is in lv2 4.70 DEX
#define NEW_POKE_SYSCALL_ADDR_475	0x80000000002A1480ULL	// where above syscall is in lv2 4.75 , 4.76, 4.78 CEX
#define NEW_POKE_SYSCALL_ADDR_475D	0x80000000002BC624ULL   // where above syscall is in lv2 4.75 , 4.76, 4.78 DEX
//#define NEW_POKE_SYSCALL_ADDR_476	0x80000000002A1480ULL	// where above syscall is in lv2 4.76
//#define NEW_POKE_SYSCALL_ADDR_476D	0x80000000002BC624ULL   // where above syscall is in lv2 4.76 DEX

#define SYSCALL_TABLE_341			0x80000000002EB128ULL	// 3.41
#define SYSCALL_TABLE_355			0x8000000000346570ULL	// 3.55
#define SYSCALL_TABLE_355D			0x8000000000361578ULL	// 3.55 DEX
#define SYSCALL_TABLE_421			0x800000000035BCA8ULL	// 4.21
#define SYSCALL_TABLE_421D			0x800000000037A1B0ULL	// 4.21 DEX
#define SYSCALL_TABLE_430			0x800000000035DBE0ULL	// 4.30 // Fixed
#define SYSCALL_TABLE_430D          0x800000000037C068ULL   // 4.30 DEX
#define SYSCALL_TABLE_431			0x800000000035DBE0ULL	// 4.31
#define SYSCALL_TABLE_440			0x800000000035E260ULL	// 4.40 // Fixed
#define SYSCALL_TABLE_441			0x800000000035E260ULL	// 4.41 // Fixed
#define SYSCALL_TABLE_441D          0x800000000037C9E8ULL   // 4.41 DEX
#define SYSCALL_TABLE_446			0x800000000035E860ULL	// 4.46 // Fixed
#define SYSCALL_TABLE_446D          0x800000000037CFE8ULL   // 4.46 DEX
#define SYSCALL_TABLE_450			0x800000000035F0D0ULL	// 4.50 // Fixed
#define SYSCALL_TABLE_450D          0x8000000000383658ULL   // 4.50 DEX
#define SYSCALL_TABLE_453			0x800000000035F300ULL	// 4.53 // Fixed
#define SYSCALL_TABLE_453D          0x8000000000385108ULL   // 4.53 DEX
#define SYSCALL_TABLE_455			0x8000000000362680ULL	// 4.55 // Fixed
#define SYSCALL_TABLE_455D          0x8000000000388488ULL   // 4.55 DEX
#define SYSCALL_TABLE_460			0x8000000000363A18ULL	// 4.60 // Fixed
#define SYSCALL_TABLE_465			0x8000000000363A18ULL	// 4.65 // Fixed
#define SYSCALL_TABLE_465D			0x800000000038A120ULL	// 4.65 DEX
#define SYSCALL_TABLE_466			0x8000000000363A18ULL	// 4.66 // Fixed
#define SYSCALL_TABLE_466D			0x800000000038A120ULL	// 4.66 DEX
#define SYSCALL_TABLE_470			0x8000000000363B60ULL	// 4.70
#define SYSCALL_TABLE_470D			0x800000000038A368ULL	// 4.70 DEX
#define SYSCALL_TABLE_475			0x8000000000363BE0ULL	// 4.75
#define SYSCALL_TABLE_475D			0x800000000038A3E8ULL	// 4.75 DEX , TOC : 3758E0
#define SYSCALL_TABLE_476			0x8000000000363BE0ULL	// 4.76
#define SYSCALL_TABLE_476D			0x800000000038A3E8ULL	// 4.76 DEX , TOC : 3758E0


#define SYSCALL_PTR(n)				(SYSCALL_TABLE + 8 * (n))


// LV1
#define HV_BASE						0x8000000014000000ULL	// where in lv2 to map lv1
#define HV_SIZE						0x001000				// 0x1000 (we need 4k from lv1 only)
#define HV_PAGE_SIZE				0x0c					// 4k = 0x1000 (1 << 0x0c)

// 3.55?
#define	HV_START_OFFSET				0x363000				// remove lv2 protection
#define	HV_START_OFFSET2			0x16f000				// set lv2 access rights for sys_storage
															// at address 0x16f3b8 (3.55)
#define HV_OFFSET					0x000a78				// at address 0x363a78

// 4.21
#define HV_START_OFFSET_421			0x370A28				// 4.21 lv2 protection
#define	HV_START_OFFSET2_421		0x16F758				// set lv2 access rights for sys_storage

// 4.30 - 4.53
#define HV_START_OFFSET_430			0x370AA8				// 4.30 lv2 protection
#define	HV_START_OFFSET2_430		0x16FA60    			// set lv2 access rights for sys_storage
/*
// 4.31
#define HV_START_OFFSET_431			0x370AA8				// 4.31 lv2 protection
#define	HV_START_OFFSET2_431		0x16FA60    			// set lv2 access rights for sys_storage

// 4.40
#define	HV_START_OFFSET_440 		0x370AA8				// remove lv2 protection
#define	HV_START_OFFSET2_440		0x16FA60    			// set lv2 access rights for sys_storage

// 4.41
#define	HV_START_OFFSET_441 		0x370AA8				// remove lv2 protection
#define	HV_START_OFFSET2_441		0x16FA60    			// set lv2 access rights for sys_storage

// 4.46
#define	HV_START_OFFSET_446 		0x370AA8				// remove lv2 protection
#define	HV_START_OFFSET2_446		0x16FA60    		// set lv2 access rights for sys_storage

// 4.50
#define	HV_START_OFFSET_450 		0x370AA8				// remove lv2 protection
#define	HV_START_OFFSET2_450		0x16FA60        		// set lv2 access rights for sys_storage

// 4.53
#define	HV_START_OFFSET_453 		0x370AA8				// remove lv2 protection
#define	HV_START_OFFSET2_453		0x16FA60    			// set lv2 access rights for sys_storage
*/
// 4.55-4.78
#define	HV_START_OFFSET_455 		0x370F28				// remove lv2 protection // Fixed
#define	HV_START_OFFSET2_455		0x16FA60    			// set lv2 access rights for sys_storage
/*
// 4.60 
#define	HV_START_OFFSET_460 		0x370F28				// remove lv2 protection // Fixed
#define	HV_START_OFFSET2_460		0x16FA60    			// set lv2 access rights for sys_storage

// 4.65
#define	HV_START_OFFSET_465		0x370F28				// remove lv2 protection // Fixed
#define	HV_START_OFFSET2_465		0x16FA60     			// set lv2 access rights for sys_storage

// 4.66
#define	HV_START_OFFSET_466		0x370F28				// remove lv2 protection // Fixed
#define	HV_START_OFFSET2_466		0x16FA60     			// set lv2 access rights for sys_storage

// 4.70
#define	HV_START_OFFSET_470		0x370F28				// remove lv2 protection // Fixed
#define	HV_START_OFFSET2_470		0x16FA60     			// set lv2 access rights for sys_storage

// 4.75
#define	HV_START_OFFSET_475		0x370F28				// remove lv2 protection // Fixed
#define	HV_START_OFFSET2_475		0x16FA60     			// set lv2 access rights for sys_storage */
//////////////////////////////////////////////////////////////////////////////////


extern u64 HVSC_SYSCALL_ADDR;
extern u64 NEW_POKE_SYSCALL_ADDR;
extern u64 SYSCALL_TABLE;

#endif

