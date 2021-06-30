OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{
  ram   (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS
{
  .text : {
    PROVIDE(_text_start = .);
    *(.reset)
    *(.text .text.*)
    PROVIDE(_text_end = .);
  } >ram AT>ram

  .rodata : {
    *(.rodata .rodata.*)
    *(.note.* )
  } >ram AT>ram

  .data : {
    . = ALIGN(4096);
    *(.data .data.*)
  } >ram AT>ram

  .sdata : {
    PROVIDE( __global_pointer$ = . + 0x800 );
    *(.sdata .sdata.*)
  } >ram AT>ram

  . = ALIGN(16);
  PROVIDE(_bss_start = .);

  .sbss : {
    *(.sbss .sbss.*)
  } >ram AT>ram

  .bss :{
    *(.bss .bss.*)
  } >ram AT>ram

  . = ALIGN(16);
  PROVIDE(_bss_end = .);

  .stack :{
    . = ALIGN(16);
    PROVIDE(_stack_start = .);
    . = . + 4096;
    PROVIDE(_stack_end = .);
  } >ram AT>ram
}
