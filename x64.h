#ifndef __X86_H
#define __X86_H

#include <sys/types.h>

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

#endif //__X86_H
