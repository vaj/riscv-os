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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <elf.h>


void main(int argc, char** argv)
{
    Elf64_Ehdr elfheader;
    Elf64_Phdr *pheader;
    unsigned long paddr;
    unsigned long offset = 0UL;
    int fd;
    int i;

    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(fd);
    }
    if (read(fd, &elfheader, sizeof(elfheader)) < 0) {
        perror("read eheader");
        exit(-1);
    }
    if (argc >= 3) {
        paddr = (unsigned long)strtol(argv[2], NULL, 16);
    } else {
        paddr = elfheader.e_entry;
    }
    if (paddr) {
        offset = paddr - elfheader.e_entry;
    }
    if (lseek(fd, elfheader.e_phoff, SEEK_SET) < 0) {
        perror("lseek");
        exit(-1);
    }
    pheader = malloc(elfheader.e_phentsize * elfheader.e_phnum);
    if (read(fd, pheader, elfheader.e_phentsize * elfheader.e_phnum) < 0) {
        perror("read program header");
        exit(-1);
    }
    for (i = 0; i < elfheader.e_phnum; i++) {
        pheader[i].p_paddr = pheader[i].p_vaddr + offset;
    }
    if (lseek(fd, elfheader.e_phoff, SEEK_SET) < 0) {
        perror("lseek");
        exit(-1);
    }
    if (write(fd, pheader, elfheader.e_phentsize * elfheader.e_phnum) < 0) {
        perror("write program header");
        exit(-1);
    }
    printf("%s is located at physical address:0x%lx\n", argv[1], paddr);
    close(fd);
}
