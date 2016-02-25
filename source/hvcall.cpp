#include "hvcall.h"
#include "peek_poke.h"

	int lv1_355_insert_htab_entry(u64 htab_id, u64 hpte_group, u64 hpte_v, u64 hpte_r,
			  u64 bolted_flag, u64 flags, u64 * hpte_index,
			  u64 * hpte_evicted_v, u64 * hpte_evicted_r)
    {
	INSTALL_HVSC_REDIRECT(0x9E);	// redirect to hvcall 158

	// call lv1_355_insert_htab_entry
	u64 ret = 0, ret_hpte_index = 0, ret_hpte_evicted_v =
	    0, ret_hpte_evicted_r = 0;
	__asm__ __volatile__("mr %%r3, %4;" "mr %%r4, %5;" "mr %%r5, %6;"
			     "mr %%r6, %7;" "mr %%r7, %8;" "mr %%r8, %9;"
			     SYSCALL(HVSC_SYSCALL) "mr %0, %%r3;" "mr %1, %%r4;"
			     "mr %2, %%r5;" "mr %3, %%r6;":"=r"(ret),
			     "=r"(ret_hpte_index), "=r"(ret_hpte_evicted_v),
			     "=r"(ret_hpte_evicted_r)
			     :"r"(htab_id), "r"(hpte_group), "r"(hpte_v),
			     "r"(hpte_r), "r"(bolted_flag), "r"(flags)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

	REMOVE_HVSC_REDIRECT();

	*hpte_index = ret_hpte_index;
	*hpte_evicted_v = ret_hpte_evicted_v;
	*hpte_evicted_r = ret_hpte_evicted_r;
	return (int)ret;
    }
/*
    int lv1_355_write_virtual_uart( u64 port_number, u64 buffer, u64 bytes, u64 *bytes_written )
    {
    INSTALL_HVSC_REDIRECT(163);

    u64 ret = 0, ret_bytes = 0;

    __asm__ __volatile__(
                "mr %%r3, %2;"
                "mr %%r4, %3;"
                "mr %%r5, %4;"
                 SYSCALL(HVSC_SYSCALL)
                "mr %0, %%r3;"
                "mr %1, %%r4;"
                 :"=r"(ret), "=r"(ret_bytes)
                 :"r"(port_number), "r"(buffer), "r"(bytes)
                 :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "lr", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

    REMOVE_HVSC_REDIRECT();

    *bytes_written = ret_bytes;
    return (int)ret;
    }
    int lv1_355_allocate_memory(u64 size, u64 page_size_exp, u64 flags, u64 * addr,
			u64 * muid)
    {
	INSTALL_HVSC_REDIRECT(0x0);	// redirect to hvcall 0

	// call lv1_355_allocate_memory
	u64 ret = 0, ret_addr = 0, ret_muid = 0;
	__asm__ __volatile__("mr %%r3, %3;"
			     "mr %%r4, %4;"
			     "li %%r5, 0;"
			     "mr %%r6, %5;"
			     SYSCALL(HVSC_SYSCALL)
			     "mr %0, %%r3;"
			     "mr %1, %%r4;"
			     "mr %2, %%r5;":"=r"(ret), "=r"(ret_addr),
			     "=r"(ret_muid)
			     :"r"(size), "r"(page_size_exp), "r"(flags)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

	REMOVE_HVSC_REDIRECT();

	*addr = ret_addr;
	*muid = ret_muid;
	return (int)ret;
    }
*/	
    int lv1_355_undocumented_function_114(u64 start, u64 page_size, u64 size,
				  u64 * lpar_addr)
    {
	INSTALL_HVSC_REDIRECT(0x72);	// redirect to hvcall 114

	// call lv1_355_undocumented_function_114
	u64 ret = 0, ret_lpar_addr = 0;
	__asm__ __volatile__("mr %%r3, %2;"
			     "mr %%r4, %3;"
			     "mr %%r5, %4;"
			     SYSCALL(HVSC_SYSCALL)
			     "mr %0, %%r3;"
			     "mr %1, %%r4;":"=r"(ret), "=r"(ret_lpar_addr)
			     :"r"(start), "r"(page_size), "r"(size)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

	REMOVE_HVSC_REDIRECT();

	*lpar_addr = ret_lpar_addr;
	return (int)ret;
    }

   void lv1_355_undocumented_function_115(u64 lpar_addr)
   {
	INSTALL_HVSC_REDIRECT(0x73);	// redirect to hvcall 115

	// call lv1_355_undocumented_function_115
	__asm__ __volatile__("mr %%r3, %0;" SYSCALL(HVSC_SYSCALL)
			     :	// no return registers
			     :"r"(lpar_addr)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

	REMOVE_HVSC_REDIRECT();
   }

	int lv1_insert_htab_entry(u64 htab_id, u64 hpte_group, u64 hpte_v, u64 hpte_r,
			  u64 bolted_flag, u64 flags, u64 * hpte_index,
			  u64 * hpte_evicted_v, u64 * hpte_evicted_r)
    {

	// call lv1_insert_htab_entry
	u64 ret = 0, ret_hpte_index = 0, ret_hpte_evicted_v =
	    0, ret_hpte_evicted_r = 0;
	__asm__ __volatile__("mr %%r3, %4;" "mr %%r4, %5;" "mr %%r5, %6;"
			     "mr %%r6, %7;" "mr %%r7, %8;" "mr %%r8, %9;"
			     "li %%r10, 0x9e;" "li %%r11, 10;" "sc;" "mr %0, %%r3;" "mr %1, %%r4;"
			     "mr %2, %%r5;" "mr %3, %%r6;":"=r"(ret),
			     "=r"(ret_hpte_index), "=r"(ret_hpte_evicted_v),
			     "=r"(ret_hpte_evicted_r)
			     :"r"(htab_id), "r"(hpte_group), "r"(hpte_v),
			     "r"(hpte_r), "r"(bolted_flag), "r"(flags)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

	*hpte_index = ret_hpte_index;
	*hpte_evicted_v = ret_hpte_evicted_v;
	*hpte_evicted_r = ret_hpte_evicted_r;
	return (int)ret;
    }
    int lv1_write_virtual_uart( u64 port_number, u64 buffer, u64 bytes, u64 *bytes_written )
    {
    u64 ret = 0, ret_bytes = 0;

    __asm__ __volatile__(
                "mr %%r3, %2;"
                "mr %%r4, %3;"
                "mr %%r5, %4;"
				"li %%r10, 163;"
				"li %%r11, 10;"
                "sc;"
                "mr %0, %%r3;"
                "mr %1, %%r4;"
                 :"=r"(ret), "=r"(ret_bytes)
                 :"r"(port_number), "r"(buffer), "r"(bytes)
                 :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "lr", "ctr", "xer", "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

    *bytes_written = ret_bytes;
    return (int)ret;
    }
	
    int lv1_allocate_memory(u64 size, u64 page_size_exp, u64 flags, u64 * addr,
			u64 * muid)
    {
	// call lv1_allocate_memory
	u64 ret = 0, ret_addr = 0, ret_muid = 0;
	__asm__ __volatile__("mr %%r3, %3;"
			     "mr %%r4, %4;"
			     "li %%r5, 0;"
			     "mr %%r6, %5;"
				 "li %%r10, 0;"
				 "li %%r11, 10;"
			     "sc;"
			     "mr %0, %%r3;"
			     "mr %1, %%r4;"
			     "mr %2, %%r5;":"=r"(ret), "=r"(ret_addr),
			     "=r"(ret_muid)
			     :"r"(size), "r"(page_size_exp), "r"(flags)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

	*addr = ret_addr;
	*muid = ret_muid;
	return (int)ret;
    }

    int lv1_undocumented_function_114(u64 start, u64 page_size, u64 size,
				  u64 * lpar_addr)
    {
	// call lv1_undocumented_function_114
	u64 ret = 0, ret_lpar_addr = 0;
	__asm__ __volatile__("mr %%r3, %2;"
			     "mr %%r4, %3;"
			     "mr %%r5, %4;"
			     "li %%r10, 0x72;"
				 "li %%r11, 10;"
				 "sc;"
			     "mr %0, %%r3;"
			     "mr %1, %%r4;":"=r"(ret), "=r"(ret_lpar_addr)
			     :"r"(start), "r"(page_size), "r"(size)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

	*lpar_addr = ret_lpar_addr;
	return (int)ret;
    }


   void lv1_undocumented_function_115(u64 lpar_addr)
   {

	// call lv1_undocumented_function_115
	__asm__ __volatile__("mr %%r3, %0;" "li %%r10, 0x73;" "li %%r11, 10;" "sc;" 
			     :	// no return registers
			     :"r"(lpar_addr)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

    }

    u64 lv2_alloc(u64 size, u64 pool)
    {
	// setup syscall to redirect to alloc func
	u64 original_syscall_code_1 = lv2_peek(HVSC_SYSCALL_ADDR);
	u64 original_syscall_code_2 = lv2_peek(HVSC_SYSCALL_ADDR + 8);
	u64 original_syscall_code_3 = lv2_peek(HVSC_SYSCALL_ADDR + 16);
	lv2_poke(HVSC_SYSCALL_ADDR, 0x7C0802A67C140378ULL);
	lv2_poke(HVSC_SYSCALL_ADDR + 8, 0x4BECB6317E80A378ULL);
	lv2_poke(HVSC_SYSCALL_ADDR + 16, 0x7C0803A64E800020ULL);

	u64 ret = 0;
	__asm__ __volatile__("mr %%r3, %1;"
			     "mr %%r4, %2;"
			     SYSCALL(HVSC_SYSCALL) "mr %0, %%r3;":"=r"(ret)
			     :"r"(size), "r"(pool)
			     :"r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			     "r9", "r10", "r11", "r12", "lr", "ctr", "xer",
			     "cr0", "cr1", "cr5", "cr6", "cr7", "memory");

	// restore original syscall code
	lv2_poke(HVSC_SYSCALL_ADDR, original_syscall_code_1);
	lv2_poke(HVSC_SYSCALL_ADDR + 8, original_syscall_code_2);
	lv2_poke(HVSC_SYSCALL_ADDR + 16, original_syscall_code_3);

	return ret;
    }
	



