

static void clearbss(void)
{
    unsigned long long *p;
    extern unsigned long long _bss_start[];
    extern unsigned long long _bss_end[];

    for (p = _bss_start; p < _bss_end; p++) {
        *p = 0LL;
    }
}

void main(void) {
    clearbss();
}
