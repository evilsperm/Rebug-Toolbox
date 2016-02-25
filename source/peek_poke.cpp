#include "peek_poke.h"

int poke_syscall = 7;

u64 lv2_peek(u64 address)
{
	return Lv2Syscall1(6, address);
}

void lv2_poke(u64 address, u64 value)
{
	Lv2Syscall2(poke_syscall, address, value);
}

void lv2_poke32(u64 address, u32 value)
{
	u32 next = lv2_peek(address) & 0xffffffff;
	lv2_poke(address, (u64) value << 32 | next);
}

u64 lv1_peek(u64 address)
{
	return Lv2Syscall1(6, HV_BASE + address);
}

void lv1_poke(u64 address, u64 value)
{
	Lv2Syscall2(7, HV_BASE + address, value);
}

void install_new_poke()
{
	// install poke with icbi instruction
	lv2_poke(NEW_POKE_SYSCALL_ADDR, 0xF88300007C001FACULL);
	lv2_poke(NEW_POKE_SYSCALL_ADDR + 8, 0x4C00012C4E800020ULL);
	poke_syscall = NEW_POKE_SYSCALL;
}

void remove_new_poke()
{
	poke_syscall = 7;
	lv2_poke(NEW_POKE_SYSCALL_ADDR, 0xF821FF017C0802A6ULL);
	lv2_poke(NEW_POKE_SYSCALL_ADDR + 8, 0xFBC100F0FBE100F8ULL);
}
