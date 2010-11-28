#include <psl1ght/lv2.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "stage2.bin.h"

// Define this if using hermes' stage2 payload (as this repo normally does)
#define HERMES

#ifndef HERMES
#define STAGE2_BASE (void*)0x8000000000700000
#else
#define STAGE2_BASE (void*)0x80000000007fd000
#include "kernel.bin.h"
#endif

#define SYSCALL_PEEK 6
#define SYSCALL_POKE 7
#define SYSCALL_TRASH 11

// lv2 retail 3.41
#define LV2_MEMCPY    0x800000000007C01C

static void Poke(void* addr, u64 value)
{
	Lv2Syscall2(SYSCALL_POKE, (u64)addr, value);
}

static u64 Peek(void* addr)
{
	return Lv2Syscall1(SYSCALL_PEEK, (u64)addr);
}

int main(int argc, const char* argv[])
{
	printf("Loading AsbestOS Stage2 into memory...\n");
	void* opd = (void*)Peek((u64*)LV2_SYSCALL_TABLE + SYSCALL_TRASH);
	Poke(opd, LV2_MEMCPY);
	Lv2Syscall3(SYSCALL_TRASH, (u64)STAGE2_BASE, (u64)stage2_bin, sizeof(stage2_bin));

	printf("Launching...\n");
	Poke(opd, (u64)STAGE2_BASE);
#ifndef HERMES
	Lv2Syscall0(SYSCALL_TRASH);
#else
	Lv2Syscall2(SYSCALL_TRASH, sizeof(kernel_bin), (u64)kernel_bin);
#endif
	return 0;
}
