#ifndef MM_H
#define MM_H

#include "common.h"

int mm_insert_htab_entry(u64 va_addr, u64 lpar_addr, u64 prot, u64 *index);
int mm_map_lpar_memory_region(u64 lpar_start_addr, u64 ea_start_addr, u64 size, u64 page_shift, u64 prot);
int mm_355_insert_htab_entry(u64 va_addr, u64 lpar_addr, u64 prot, u64 *index);
int mm_355_map_lpar_memory_region(u64 lpar_start_addr, u64 ea_start_addr, u64 size, u64 page_shift, u64 prot);

#endif

