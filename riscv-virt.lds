OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{
  ram_system (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 64M
  ram_app    (wxa!ri) : ORIGIN = 0x84000000, LENGTH = 64M
}

SECTIONS
{
  .system_text : {
    PROVIDE(_text_start = .);
    *(.reset)
    main.o(.text .text.*)
    primitives.o(.text .text.*)
    PROVIDE(_text_end = .);
  } >ram_system AT>ram_system

  .system_rodata : {
    main.o(.rodata .rodata.*)
    primitives.o(.rodata .rodata.*)
    *(.note.* )
  } >ram_system AT>ram_system

  .system_data : {
    . = ALIGN(4096);
    main.o(.data .data.*)
    primitives.o(.data .data.*)
  } >ram_system AT>ram_system

  .system_sdata : {
    PROVIDE( __global_pointer$ = . + 0x800 );
    main.o(.sdata .sdata.*)
    primitives.o(.sdata .sdata.*)
  } >ram_system AT>ram_system

  . = ALIGN(16);
  PROVIDE(_system_bss_start = .);

  .system_sbss : {
    main.o(.sbss .sbss.*)
    primitives.o(.sbss .sbss.*)
  } >ram_system AT>ram_system

  .system_bss :{
    main.o(.bss .bss.*)
    primitives.o(.bss .bss.*)
  } >ram_system AT>ram_system

  . = ALIGN(16);
  PROVIDE(_system_bss_end = .);

  .exc_stack :{
    . = ALIGN(16);
    PROVIDE(_exc_stack_start = .);
    . = . + 1024;
    PROVIDE(_exc_stack_end = .);
  } >ram_system AT>ram_system

  .stack :{
    . = ALIGN(16);
    PROVIDE(_stack_start = .);
    . = . + 4096;
    PROVIDE(_stack_end = .);
  } >ram_system AT>ram_system

  .text : {
    *(.text .text.*)
  } >ram_app AT>ram_app

  .rodata : {
    *(.rodata .rodata.*)
  } >ram_app AT>ram_app

  .data : {
    . = ALIGN(4096);
    *(.data .data.*)
  } >ram_app AT>ram_app

  .bss :{
    . = ALIGN(16);
    PROVIDE(_bss_start = .);
    *(.bss .bss.*)
    . = ALIGN(16);
    PROVIDE(_bss_end = .);
  } >ram_app AT>ram_app

  ram_app_start = ORIGIN(ram_app);
  ram_app_size = LENGTH(ram_app);
}
