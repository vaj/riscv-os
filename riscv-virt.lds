/*
MIT License

Copyright (c) 2021 VA Linux Systems Japan, K.K.

Author : Hirokazu Takahashi <taka@valinux.co.jp>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

OUTPUT_ARCH( "riscv" )

ENTRY( _start )

MEMORY
{
  ram_system (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 64M
  ram_app    (wxa!ri) : ORIGIN = 0x84000000, LENGTH = 64M
}

_CLV_SIZE = 1048;
_STACK_SIZE = 4096;
_ESTACK_SIZE = 1024;
_TLS_SIZE = _CLV_SIZE + _STACK_SIZE;
_MAXCORE = 8;

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

  PROVIDE(_system_bss_start = .);

  .system_sbss : {
    main.o(.sbss .sbss.*)
    primitives.o(.sbss .sbss.*)
  } >ram_system AT>ram_system

  .system_bss :{
    main.o(.bss .bss.*)
    primitives.o(.bss .bss.*)
  } >ram_system AT>ram_system

  PROVIDE(_system_bss_end = .);

  /* tls data backup section */
  .system_tdata : {
    PROVIDE(_tdata_start = .);
    main.o(.tdata .tdata.*)
    primitives.o(.tdata .tdata.*)
    PROVIDE(_tdata_end = .);
  } >ram_system AT>ram_system

  .system_tbss : {
    PROVIDE(_tbss_start = .);
    main.o(.tbss .tbss.*)
    primitives.o(.tbss .tbss.*)
    PROVIDE(_tbss_end = .);
  } >ram_system AT>ram_system

  .tls :{
    . = ALIGN(16);
    PROVIDE(_tls_start = .);
    . = . + _TLS_SIZE * _MAXCORE;
    . = ALIGN(16);
    PROVIDE(_tls_end = .);
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
