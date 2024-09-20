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

#include "vma.h"

extern unsigned char ram_app_size[];     /* The size must be a power of 2 */
extern unsigned char ram_app_start[];    /* The address must be multiples of the size */
extern unsigned char ram_system_size[];  /* The size must be a power of 2 */
extern unsigned char ram_system_start[]; /* The address must be multiples of the size */

#define SHARED_ASID  0UL

extern void SetSATP(unsigned long pagetable);

static pte_t pgtable_root[NUM_PTE_IN_PAGE] __attribute__ ((aligned (PAGESIZE)));
static pte_t pgtable_lvl2_mem[NUM_PTE_IN_PAGE] __attribute__ ((aligned (PAGESIZE)));
static pte_t pgtable_lvl2_io[NUM_PTE_IN_PAGE] __attribute__ ((aligned (PAGESIZE)));

#define UART_SPACE 0x10000000U

static void init_ptes(pte_t* pte, unsigned int begin, unsigned int end, unsigned long pfn, unsigned long mode)
{
    int ent;

    for (ent = begin; ent < end; ent++, pfn += NUM_PTE_IN_PAGE) {
        pte[ent] = (pfn<<PTE_PPN_SHIFT) | mode;
    }
}

/*
 * Note: Set 'A' and 'D' bits in a PTE because some RISC-V implementations
 *       don't update these bits and just raise a page-fault execption,
 *       which software is expected to handle.
 */
void SetupPageTables(void)
{
    unsigned long offset;

    /* map the S-mode space */
    offset = SUPER_PAGE_NUMBER(ram_system_start)%NUM_PTE_IN_PAGE;
    init_ptes(pgtable_lvl2_mem, offset, offset + SUPER_PAGE_NUMBER(ram_system_size), PAGE_NUMBER(ram_system_start), PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D);

    /* map the U-mode space */
    offset = SUPER_PAGE_NUMBER(ram_app_start)%NUM_PTE_IN_PAGE;
    init_ptes(pgtable_lvl2_mem, offset, offset + SUPER_PAGE_NUMBER(ram_app_size), PAGE_NUMBER(ram_app_start), PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D);

    offset = HUGE_PAGE_NUMBER(ram_system_start);
    pgtable_root[offset] = (PAGE_NUMBER(pgtable_lvl2_mem)<<PTE_PPN_SHIFT) | PTE_V | PTE_G;

    /* map the I/O space for uart */
    offset = SUPER_PAGE_NUMBER(UART_SPACE)%NUM_PTE_IN_PAGE;
    init_ptes(pgtable_lvl2_io, offset, offset + SUPER_PAGE_NUMBER(SUPER_PAGESIZE), PAGE_NUMBER(UART_SPACE), PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D);

    offset = HUGE_PAGE_NUMBER(UART_SPACE);
    pgtable_root[offset] = (PAGE_NUMBER(pgtable_lvl2_io)<<PTE_PPN_SHIFT) | PTE_V | PTE_G;
}

void EnableMMU(void)
{
    SetSATP(ATP_MODE_SV39 | ATP_ASID(SHARED_ASID) | PAGE_NUMBER(pgtable_root));
}
