#ifndef __X86_H
#define __X86_H

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/smp.h>
#include <sys/pcpu.h>
#include <sys/malloc.h>
#include <sys/proc.h>

#include <machine/cpufunc.h>
#include <machine/segments.h>

uint64_t __read_cr0(void);
uint64_t __read_cr3(void);
uint64_t __read_cr4(void);

uint64_t __read_dr7(void);

uint64_t __read_rflags(void);

uint16_t __read_es(void);
uint16_t __read_cs(void);
uint16_t __read_ss(void);
uint16_t __read_ds(void);
uint16_t __read_fs(void);
uint16_t __read_gs(void);
uint16_t __read_ldt(void);
uint16_t __read_tr(void);

uint32_t __load_segment_limit(uint32_t selector);
uint32_t __load_access_right(uint32_t selector);

void __store_gdt(struct region_descriptor* addr);
void __store_idt(struct region_descriptor* addr);

void __sti(void);
void __cli(void);

#endif //__X86_H
