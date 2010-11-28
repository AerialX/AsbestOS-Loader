/*  start.S - kernel relocation and launch code

Copyright (C) 2010  Hector Martin "marcan" <hector@marcansoft.com
Copyright (C) 2010  Francisco Muñoz "hermes" <www.elotrolado.net>

This code is licensed to you under the terms of the GNU GPL, version 2;
see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

# modified by Hermes to lauch from one application

#define DEST_H 0x1 // 0x10000 address


.text
.global _start
.section .start,"ax",@progbits
	.align 3
_start:
	mfmsr %r5
	# disable interrupts
	rldicl %r5, %r5, 48, 1
	rotldi %r5, %r5, 16
	mtmsrd %r5, 0

	li %r5, 1;
	rldicr %r5, %r5, 63, 0;

	mr %r6, %r5
	oris %r6, %r6, (entry_param)@h
	ori %r6, %r6, (entry_param)@l
	std %r3, 0(%r6)
	std %r4, 8(%r6)

	/* patch decrementer Interrupt. It is based in 3.41 firmware (but may be it works in all) */

	li %r6, 0x920;
	or %r6, %r6, %r5

	li %r3, _threads_start@h
	li %r4, _threads_start@l

	sth %r3, 0x92E(%r5)
	sth %r4, 0x932(%r5)

	dcbst 0, %r6
	sync
	icbi 0, %r6
	
_threads_start:
	/* both threads end up running here */

	/* if this is thread 1, go to _thread1_start */
	mfspr %r3, 0x88
	cntlzw. %r3, %r3
	bne _thread1_start

_thread1_wait:
	
	li %r3, 1;
	rldicr %r3, %r3, 63, 0;
	oris %r3, %r3, _thread1_active@h
	ori %r3, %r3, _thread1_active@l

	ld %r4, 0x0(%r3)
	cmpldi %r4, 0
	beq _thread1_wait

	
	/* copy the Linux kernel to LV2 temporal memory */

	li %r5, 1;
	rldicr %r5, %r5, 63, 0;
	mr %r6, %r5

	oris	%r5, %r5, DEST_H

	oris %r6, %r6, (entry_param)@h
	ori %r6, %r6, (entry_param)@l

	ld %r3, 0(%r6) // get len bytes
	ld %r4, 8(%r6) // get origin addr

	add %r3, %r3, %r5

	// copy to temp memory in DEST_H
	bl _move_loop

	/* get realmode address */
	
        lis %r3, (realmode)@h
	ori %r3, %r3, (realmode)@l

	/* switch to realmode */
	isync
	mtsrr0 %r3
	/* set MSR[DR:IR] = 00 and jump to srr0 */
	mfmsr %r3
	li %r4, 0x30
	andc %r3, %r3, %r4
	mtsrr1 %r3
	rfid


realmode:
        
	/* warning!!! i don't know how TOC register works. Originally, marcan uses '__toc_start+0x8000' but i cannot see the reason for '+0x8000' 
	and in theory, it overtakes the LV2 area when i allocate the code in 0x7FD000, but it works
	*/

	lis %r2, (__toc_start+0x8000)@h
	ori %r2, %r2, (__toc_start+0x8000)@l
	
	lis %r1, _stack_bot@h
	ori %r1, %r1, _stack_bot@l

	addi %r1, %r1, 64
	li %r0, 0
	std %r0, 0(%r1)

	/* clear BSS */
	lis %r3, __bss_start@h
	ori %r3, %r3, __bss_start@l
	lis %r4, __bss_end@h
	ori %r4, %r4, __bss_end@l


	li %r5, 0
_bss_loop:
	std %r5, 0(%r3)
	addi %r3, %r3, 8
	cmpld %r3, %r4
	blt _bss_loop


	bl main

	li      %r3, 1
	li      %r11, 0xFF
        sc	1

	.global kload
kload:
	mfmsr %r5
	# disable interrupts
	rldicl %r5, %r5, 48, 1
	rotldi %r5, %r5, 16
	mtmsrd %r5, 0

	# relocate to our real address
	
	li %r5, 0
	lis %r4, DEST_H

	lis %r6, (entry_param)@h
	ori %r6, %r6, (entry_param)@l
	ld %r3, 0(%r6)

	add %r3, %r3, %r5

_move_loop:
	ld %r8, 0(%r4)
	std %r8, 0(%r5)
	ld %r8, 8(%r4)
	std %r8, 8(%r5)
	ld %r8, 16(%r4)
	std %r8, 16(%r5)
	ld %r8, 24(%r4)
	std %r8, 24(%r5)
	dcbst 0, %r5
	sync
	icbi 0, %r5
	addi %r4, %r4, 0x20
	addi %r5, %r5, 0x20
	cmpld %r5, %r3
	blt _move_loop

	blr
	
	.global klaunch
klaunch:
	ba 0x100



	/* thread 1 runs this code - spin until we have a vector */
_thread1_start:

	mfmsr %r5
	# disable interrupts
	rldicl %r5, %r5, 48, 1
	rotldi %r5, %r5, 16
	mtmsrd %r5, 0

	lis %r3, _thread1_realmode@h
	ori %r3, %r3, _thread1_realmode@l

        isync
	mtsrr0 %r3
	/* set MSR[DR:IR] = 00 and jump to srr0 */
	mfmsr %r3
	li %r4, 0x30
	andc %r3, %r3, %r4
	mtsrr1 %r3
	rfid

_thread1_realmode:
	
	lis %r3, _thread1_active@h
	ori %r3, %r3, _thread1_active@l

	li %r4, 1
	std %r4, 0(%r3) // set _thread1_active
1:
	ld %r4, 8(%r3)  // test _thread1_release
	cmpldi %r4, 0
	beq 1b

	ld %r4, 16(%r3) // load _thread1_vector
	mtctr %r4
	bctr

	.align 3
	.global _thread1_active, _thread1_release, _thread1_vector
_thread1_active:
	.quad 0
_thread1_release:
	.quad 0
_thread1_vector:
	.quad 0x100
entry_param:	
	.quad 0 // param 1 (len)
	.quad 0 // param 2 (addr)

