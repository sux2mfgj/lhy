#include "x64.h"

uint64_t __read_cr0(void)
{
    uint64_t cr0;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    return cr0;
}

uint64_t __read_cr3(void)
{
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0": "=r"(cr3));
    return cr3;
}

uint64_t __read_cr4(void)
{
    uint64_t cr4;
    __asm__ volatile("mov %%cr4, %0": "=r"(cr4));
    return cr4;
}

uint64_t __read_dr7(void)
{
    uint64_t dr7;
    __asm__ volatile("mov %%dr7, %0" : "=r"(dr7));
    return dr7;
}

uint64_t __read_rflags(void)
{
    uint64_t rflags;
    __asm__ volatile(
            "pushfq;"
            "popq %0;"
            : "=r"(rflags));
    return rflags;
}

uint16_t __read_es(void)
{
	uint16_t es;
	__asm__ volatile("mov %%es, %0" : "=r"(es));
	return es;
}

uint16_t __read_cs(void)
{
	uint16_t cs;
	__asm__ volatile("mov %%cs, %0" : "=r"(cs));
	return cs;
}

uint16_t __read_ss(void)
{
	uint16_t ss;
	__asm__ volatile("mov %%ss, %0" : "=r"(ss));
	return ss;
}

uint16_t __read_ds(void)
{
	uint16_t ds;
	__asm__ volatile("mov %%ds, %0" : "=r"(ds));
	return ds;
}

uint16_t __read_fs(void)
{
	uint16_t fs;
	__asm__ volatile("mov %%fs, %0" : "=r"(fs));
	return fs;
}

uint16_t __read_gs(void)
{
	uint16_t gs;
	__asm__ volatile("mov %%gs, %0" : "=r"(gs));
	return gs;
}

uint16_t __read_ldt(void)
{
	uint16_t ldt;
	__asm__ volatile("sldt %0" : "=r"(ldt));
	return ldt;
}

uint16_t __read_tr(void)
{
	uint16_t tr;
	__asm__ volatile("str %0" : "=r"(tr));
	return tr;
}

uint32_t __load_segment_limit(uint32_t selector)
{
	uint32_t limit;
	__asm__ volatile("lsl %1, %0" : "=r"(limit) : "r"(selector));
	return limit;
}

uint32_t __load_access_right(uint32_t selector)
{
	uint32_t access_right;
	__asm__ volatile("lar %1, %0" : "=r"(access_right) : "r"(selector));
    return access_right;
}

