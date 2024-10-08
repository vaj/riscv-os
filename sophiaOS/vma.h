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

typedef unsigned long  pte_t;

#define PTE_V   (1UL<<0U)
#define PTE_R   (1UL<<1U)
#define PTE_W   (1UL<<2U)
#define PTE_X   (1UL<<3U)
#define PTE_U   (1UL<<4U)
#define PTE_G   (1UL<<5U)
#define PTE_A   (1UL<<6U)
#define PTE_D   (1UL<<7U)
#define PTE_PPN_SHIFT     10U

#define PAGESIZE_SHIFT    12U
#define PAGESIZE          (1UL<<PAGESIZE_SHIFT)    //4KB
#define PTESIZE           sizeof(pte_t)
#define NUM_PTE_IN_PAGE   (PAGESIZE/PTESIZE)
#define SUPER_PAGESIZE    (PAGESIZE*NUM_PTE_IN_PAGE)       //2MB
#define HUGE_PAGESIZE     (SUPER_PAGESIZE*NUM_PTE_IN_PAGE) //1GB
#define PAGE_NUMBER(addr) ((unsigned long)(addr) >> PAGESIZE_SHIFT)
#define SUPER_PAGE_NUMBER(addr) (PAGE_NUMBER(addr)/NUM_PTE_IN_PAGE)
#define HUGE_PAGE_NUMBER(addr) (SUPER_PAGE_NUMBER(addr)/NUM_PTE_IN_PAGE)

#define ATP_MODE_BARE    (0UL<<60U)
#define ATP_MODE_SV39    (8UL<<60U)
#define ATP_MODE_SV48    (9UL<<60U)
#define ATP_MODE_SV57    (10UL<<60U)
#define ATP_MODE_SV64    (11UL<<60U)
#define ATP_ASID(asid)   ((asid)<<44U)

extern void SetupPageTables(void);
extern void EnableMMU(void);
