#ifndef HVCALL_H
#define HVCALL_H

#include "common.h"

#define HPTE_V_BOLTED			0x0000000000000010ULL
#define HPTE_V_LARGE			0x0000000000000004ULL
#define HPTE_V_VALID			0x0000000000000001ULL
#define HPTE_R_PROT_MASK		0x0000000000000003ULL
#define MM_EA2VA(ea)			((ea) & ~0x8000000000000000ULL)

#define SC_QUOTE_(x) #x
#define SYSCALL(num) "li %%r11, " SC_QUOTE_(num) "; sc;"

#define INSTALL_HVSC_REDIRECT(hvcall) u64 original_syscall_code_1 = lv2_peek(HVSC_SYSCALL_ADDR); \
	u64 original_syscall_code_2 = lv2_peek(HVSC_SYSCALL_ADDR + 8); \
	u64 original_syscall_code_3 = lv2_peek(HVSC_SYSCALL_ADDR + 16); \
	u64 original_syscall_code_4 = lv2_peek(HVSC_SYSCALL_ADDR + 24); \
	lv2_poke(HVSC_SYSCALL_ADDR, 0x7C0802A6F8010010ULL);	\
	lv2_poke(HVSC_SYSCALL_ADDR + 8, 0x3960000044000022ULL | (u64)hvcall << 32);	\
	lv2_poke(HVSC_SYSCALL_ADDR + 16, 0xE80100107C0803A6ULL); \
	lv2_poke(HVSC_SYSCALL_ADDR + 24, 0x4e80002060000000ULL);
	
#define REMOVE_HVSC_REDIRECT() lv2_poke(HVSC_SYSCALL_ADDR, original_syscall_code_1); \
	lv2_poke(HVSC_SYSCALL_ADDR + 8, original_syscall_code_2); \
	lv2_poke(HVSC_SYSCALL_ADDR + 16, original_syscall_code_3); \
	lv2_poke(HVSC_SYSCALL_ADDR + 24, original_syscall_code_4);

int lv1_insert_htab_entry(u64 htab_id, u64 hpte_group, u64 hpte_v, u64 hpte_r, u64 bolted_flag, u64 flags, u64 *hpte_index, u64 *hpte_evicted_v, u64 *hpte_evicted_r);
int lv1_allocate_memory(u64 size, u64 page_size_exp, u64 flags, u64 *addr, u64 *muid);
int lv1_undocumented_function_114(u64 start, u64 page_size, u64 size, u64 *lpar_addr);
void lv1_undocumented_function_115(u64 lpar_addr);
int lv1_write_virtual_uart( u64 port_number, u64 buffer, u64 bytes, u64 *bytes_written );

int lv1_355_insert_htab_entry(u64 htab_id, u64 hpte_group, u64 hpte_v, u64 hpte_r, u64 bolted_flag, u64 flags, u64 *hpte_index, u64 *hpte_evicted_v, u64 *hpte_evicted_r);
int lv1_355_undocumented_function_114(u64 start, u64 page_size, u64 size, u64 *lpar_addr);
void lv1_355_undocumented_function_115(u64 lpar_addr);


u64 lv2_alloc(u64 size, u64 pool);

#endif

