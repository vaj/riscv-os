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
  ram    (wxa!ri) : ORIGIN = 0x80200000, LENGTH = 4M

  /* guests' pysical location */
  ram_guest1 (wxa!ri) : ORIGIN = 0x80600000, LENGTH = 64M
  ram_guest2 (wxa!ri) : ORIGIN = 0x84600000, LENGTH = 32M
}

_MAXCORE = 8;
_CLV_SIZE = 1024;
_STACK_SIZE = 2048;
_ESTACK_SIZE = 1024;
_TLS_SIZE = _CLV_SIZE + _STACK_SIZE;

ram_guest1_start = ORIGIN(ram_guest1);
ram_guest1_size  = LENGTH(ram_guest1);
ram_guest2_start = ORIGIN(ram_guest2);
ram_guest2_size  = LENGTH(ram_guest2);

SECTIONS
{
  .text : {
    *(.reset)
    *.o(.text .text.*)
  } >ram AT>ram

  .rodata : {
    *.o(.rodata .rodata.*)
  } >ram AT>ram

  .data : {
    *.o(.data .data.*)
  } >ram AT>ram

  .sdata : {
    PROVIDE( __global_pointer$ = . + 0x800 );
    *.o(.sdata .sdata.*)
  } >ram AT>ram

  PROVIDE(_bss_start = .);

  .sbss : {
    *.o(.sbss .sbss.*)
  } >ram AT>ram

  .bss : {
    *.o(.bss .bss.*)
  } >ram AT>ram

  PROVIDE(_bss_end = .);

  /* tls data backup section */
  .system_tdata : {
    PROVIDE(_tdata_start = .);
    *.o(.tdata .tdata.*)
    PROVIDE(_tdata_end = .);
  } >ram AT>ram

  .system_tbss : {
    PROVIDE(_tbss_start = .);
    *.o(.tbss .tbss.*)
    PROVIDE(_tbss_end = .);
  } >ram AT>ram

  .tls :{
    . = ALIGN(16);
    PROVIDE(_tls_start = .);
    . = . + _TLS_SIZE * _MAXCORE;
    . = ALIGN(16);
    PROVIDE(_tls_end = .);
  } >ram AT>ram
}
