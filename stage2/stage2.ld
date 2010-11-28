/*  stage2.ld - AsbestOS stage2 linker script

Copyright (C) 2010  Hector Martin "marcan" <hector@marcansoft.com
Copyright (C) 2010  Francisco Muñoz "hermes" <www.elotrolado.net>

This code is licensed to you under the terms of the GNU GPL, version 2;
see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

OUTPUT_FORMAT("elf64-powerpc")
OUTPUT_ARCH(powerpc:common64)

/* stage2 runs in realmode */
__base = 0x7FD000;


ENTRY(_start)

PHDRS {
	stub		PT_LOAD FLAGS(7);
}

SECTIONS {
	. = __base;
	__self_start = .;

	.start : {
		*(.start)
	} :stub = 0

	.text : {
		*(.text)
		*(.text.*)
	}

	. = ALIGN(32);
	.opd : {
		*(.opd)
	}
	.toc : {
		__toc_start = .;
		*(.got)
		*(.toc)
	}

	.rodata : {
		*(.rodata)
		*(.rodata.*)
	}

	. = ALIGN(32);
	.data : {
		*(.data)
		*(.data.*)
		*(.sdata)
		*(.sdata.*)
	}
	. = (( . +67)&0xFFFFFFC0) - 4;
	.pad : {
		LONG(0);
	}

	
	__bss_start = .;
	.bss : {
		*(.sbss)
		*(.sbss.*)
		*(COMMON)
		*(.bss)
		*(.bss.*)
	}
	__bss_end = .;

	. = ALIGN(32);

	.stack : {
		_stack_top = .;
		. += 2048;
		_stack_bot = .;
	} 

}