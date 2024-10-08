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

/* currently supports two vms */
#define N_VM  2

static pte_t pgtable_root_vm[N_VM][NUM_PTE_IN_PAGEx4] __attribute__ ((aligned (PAGESIZEx4)));
static pte_t pgtable_lvl2_vm[N_VM][NUM_PTE_IN_PAGE] __attribute__ ((aligned (PAGESIZE)));
static pte_t pgtable_lvl2_io[N_VM][NUM_PTE_IN_PAGE] __attribute__ ((aligned (PAGESIZE)));

static void init_ptes(pte_t* pte, unsigned int begin, unsigned int end, unsigned long pfn, unsigned long mode)
{
    int ent;

    for (ent = begin; ent < end; ent++, pfn += NUM_PTE_IN_PAGE) {
        pte[ent] = (pfn<<PTE_PPN_SHIFT) | mode;
    }
}

/* map the guest's physical space */
unsigned long CreateVmSpace(unsigned long vm, unsigned long pstart, unsigned long psize, unsigned long vaddr)
{
    pte_t* pgtable_root;
    pte_t* pgtable_lvl2;
    unsigned long offset;

    pgtable_root = pgtable_root_vm[vm];
    pgtable_lvl2 = pgtable_lvl2_vm[vm];

    /* Note: PTE_U must be set against all valid ptes for guests */
    offset = SUPER_PAGE_NUMBER(vaddr)%NUM_PTE_IN_PAGE;
    init_ptes(pgtable_lvl2, offset, offset + SUPER_PAGE_NUMBER(psize), PAGE_NUMBER(pstart), PTE_V | PTE_R | PTE_W | PTE_X | PTE_G | PTE_U | PTE_A | PTE_D);

    offset = HUGE_PAGE_NUMBER(vaddr);
    pgtable_root[offset] = (PAGE_NUMBER(pgtable_lvl2)<<PTE_PPN_SHIFT) | PTE_V | PTE_G;
    return ATP_MODE_SV39 | ATP_VMID(vm) | PAGE_NUMBER(pgtable_root);
}

/* map io space */
unsigned long CreateVmIoSpace(unsigned long vm, unsigned long pstart, unsigned long psize)
{
    pte_t* pgtable_root;
    pte_t* pgtable_lvl2;
    unsigned long vaddr = pstart;
    unsigned long offset;

    pgtable_root = pgtable_root_vm[vm];
    pgtable_lvl2 = pgtable_lvl2_io[vm];

    offset = SUPER_PAGE_NUMBER(vaddr)%NUM_PTE_IN_PAGE;
    init_ptes(pgtable_lvl2, offset, offset + SUPER_PAGE_NUMBER(psize), PAGE_NUMBER(pstart), PTE_V | PTE_R | PTE_W | PTE_X | PTE_G | PTE_U | PTE_A | PTE_D);

    offset = HUGE_PAGE_NUMBER(vaddr);
    pgtable_root[offset] = (PAGE_NUMBER(pgtable_lvl2)<<PTE_PPN_SHIFT) | PTE_V | PTE_G;
    return ATP_MODE_SV39 | ATP_VMID(vm) | PAGE_NUMBER(pgtable_root);
}
