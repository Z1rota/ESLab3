ENTRY(Reset_Handler)
MEMORY
{
 FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
 RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 192K
}
_estack = ORIGIN(RAM) + LENGTH(RAM);
PHDRS { flash PT_LOAD FLAGS(5); ram PT_LOAD FLAGS(6); }
SECTIONS
{
 .isr_vector : { KEEP(*(.isr_vector)) } > FLASH :flash
 .text : { *(.text*) *(.rodata*) KEEP(*(.init)) KEEP(*(.fini)) . = ALIGN(4); } > FLASH :flash
 .ARM.extab : { *(.ARM.extab*) } > FLASH :flash
 .ARM.exidx : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } > FLASH :flash
 .preinit_array : { PROVIDE_HIDDEN(__preinit_array_start = .); KEEP(*(.preinit_array*)) PROVIDE_HIDDEN(__preinit_array_end = .); } > FLASH :flash
 .init_array : { PROVIDE_HIDDEN(__init_array_start = .); KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*)) PROVIDE_HIDDEN(__init_array_end = .); } > FLASH :flash
 .fini_array : { PROVIDE_HIDDEN(__fini_array_start = .); KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array*)) PROVIDE_HIDDEN(__fini_array_end = .); } > FLASH :flash
 _sidata = LOADADDR(.data);
 .data : { . = ALIGN(4); _sdata = .; *(.data*) . = ALIGN(4); _edata = .; } > RAM AT> FLASH :ram
 .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; *(.bss*) *(COMMON) . = ALIGN(4); _ebss = .; . = ALIGN(8); end = .; _end = .; . += 0xA00; } > RAM :ram
 ASSERT(. <= _estack, "RAM overflow")
}
