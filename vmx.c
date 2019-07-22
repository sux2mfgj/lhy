#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/smp.h>
#include <sys/pcpu.h>
#include <sys/malloc.h>
#include <sys/proc.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/cpufunc.h>
#include <machine/specialreg.h>

#include "vmx.h"
#include "x64.h"

struct pcpu;
extern struct pcpu __pcpu[];

struct vmcs {
    uint32_t revision_id;
    uint32_t abort_code;
    char data[PAGE_SIZE - sizeof(uint32_t) * 2];
};

static uint8_t vmxon_region[4][PAGE_SIZE] __aligned(PAGE_SIZE);
static struct vmcs* guest_vmcs;

static int vmxon(char *region)
{
    uint8_t error;
    uint64_t pa_region;

    pa_region = vtophys(region);

    //__asm__ volatile("vmxon %1; setna %0" : "=r"(error) : "m"(pa_region));
    __asm__ volatile("vmxon %1; setna %0" : "=r"(error) : "m"(*(uint64_t*)&pa_region));

    return error;
}

static void vmxoff(void)
{
    __asm__ volatile("vmxoff");
}

static int vmclear(struct vmcs* region)
{
    uint8_t error;
    uint64_t pa_region;

    pa_region = vtophys(region);

    //__asm__ volatile("vmclear %1; setna %0" : "=r"(error) : "m"(pa_region));
    __asm__ volatile("vmclear %1; setna %0" : "=r"(error) : "m"(*(uint64_t *)&pa_region));

    return error;
}

static int vmptrld(struct vmcs* region)
{
    uint8_t error;
    uint64_t pa_region;

    pa_region = vtophys(region);
    __asm__ volatile("vmptrld %1; setna %0": "=r"(error) : "m"(*(uint64_t *)&pa_region));

    return error;
}

static int vmwrite(uint64_t field, uint64_t value)
{
    uint8_t error;
    __asm__ volatile("vmwrite %2, %1; setna %0" : "=r"(error) : "r"(field), "r"(value));
    return error;
}

static void vmx_setup(void* junk)
{
    int error = 0;

    //uint64_t feature_control;
    uint32_t revision_id;

    //feature_control = rdmsr(MSR_IA32_FEATURE_CONTROL);
    //if((feature_control & IA32_FEATURE_CONTROL_LOCK) == 0)
    //{
    //    feature_control |= IA32_FEATURE_CONTROL_LOCK;
    //}

    //if((feature_control & IA32_FEATURE_CONTROL_VMX_EN) == 0)
    //{
    //    feature_control |= IA32_FEATURE_CONTROL_VMX_EN;
    //}

    //wrmsr(MSR_IA32_FEATURE_CONTROL, feature_control);

    load_cr4(rcr4() | CR4_VMXE);

    revision_id = rdmsr(MSR_VMX_BASIC) & 0xffffffff;

    *(uint32_t *)vmxon_region[curcpu] = revision_id;

    error = vmxon(vmxon_region[curcpu]);
    if(error)
    {
        printf("lhy: vmxon failed\n");
    }
    else
    {
        printf("lhy: vmxon success [%d]\n", curcpu);
    }
}

int vmx_init(void)
{
    int err = 0;

    //uint64_t vmx_basic;
    //vmx_basic = rdmsr(MSR_VMX_BASIC);

    smp_rendezvous(NULL, vmx_setup, NULL, NULL);

    return err;
}

static int setup_vmcs_guest_register_state(uint64_t rip, uint64_t rsp)
{
    int err = 0;

    uint64_t cr0 = __read_cr0();
    uint64_t cr3 = __read_cr3();
    uint64_t cr4 = __read_cr4();
    uint64_t dr7 = __read_dr7();

    err |= vmwrite(GUEST_CR0, cr0);
    err |= vmwrite(GUEST_CR3, cr3);
    err |= vmwrite(GUEST_CR4, cr4);
    err |= vmwrite(GUEST_DR7, dr7);

    if(err)
    {
        printf("lhy: failed to write control register to vmcs\n");
        return err;
    }

    // rsp, rip and rflags
    uint64_t rflags = __read_rflags();
    err |= vmwrite(GUEST_RFLAGS, rflags);
    err |= vmwrite(GUEST_RIP, rip);
    err |= vmwrite(GUEST_RSP, rsp);

    if(err)
    {
        printf("lhy: error occured while writing rsp, rip or rflags\n");
        return err;
    }

	uint16_t es = __read_es();
	uint16_t cs = __read_cs();
	uint16_t ss = __read_ss();
	uint16_t ds = __read_ds();
	uint16_t fs = __read_fs();
	uint16_t gs = __read_gs();
	uint16_t ldtr = __read_ldt();
	uint16_t tr = __read_tr();

    err |= vmwrite(GUEST_ES_SELECTOR, es);
    err |= vmwrite(GUEST_CS_SELECTOR, cs);
    err |= vmwrite(GUEST_SS_SELECTOR, ss);
    err |= vmwrite(GUEST_DS_SELECTOR, ds);
    err |= vmwrite(GUEST_FS_SELECTOR, fs);
    err |= vmwrite(GUEST_GS_SELECTOR, gs);
    err |= vmwrite(GUEST_LDTR_SELECTOR, ldtr);
    err |= vmwrite(GUEST_TR_SELECTOR, tr);

    if(err)
    {
        printf("lhy: failed. [selectors]\n");
        return err;
    }


    err |= vmwrite(GUEST_ES_BASE, 0);
    err |= vmwrite(GUEST_CS_BASE, 0);
    err |= vmwrite(GUEST_SS_BASE, 0);
    err |= vmwrite(GUEST_DS_BASE, 0);

    uint64_t fs_base = rdmsr(MSR_FSBASE);
    uint64_t gs_base = (uint64_t)&__pcpu[curcpu];
    err |= vmwrite(GUEST_FS_BASE, fs_base);
    err |= vmwrite(GUEST_GS_BASE, gs_base);

    err |= vmwrite(GUEST_LDTR_BASE, 0);
    uint64_t tr_base = (uint64_t)PCPU_GET(tssp);
    err |= vmwrite(GUEST_TR_BASE, tr_base);

    if(err)
    {
        printf("lhy: failed. [base]\n");
        return err;
    }

    uint32_t cs_limit = __load_segment_limit(cs);
    uint32_t ss_limit = __load_segment_limit(ss);
    uint32_t ds_limit = __load_segment_limit(ds);
    uint32_t es_limit = __load_segment_limit(es);
    uint32_t fs_limit = __load_segment_limit(fs);
    uint32_t gs_limit = __load_segment_limit(gs);
    uint32_t ldt_limit = __load_segment_limit(ldtr);
    uint32_t tr_limit = __load_segment_limit(tr);

    err |= vmwrite(GUEST_CS_LIMIT, cs_limit);
    err |= vmwrite(GUEST_SS_LIMIT, ss_limit);
    err |= vmwrite(GUEST_DS_LIMIT, ds_limit);
    err |= vmwrite(GUEST_ES_LIMIT, es_limit);
    err |= vmwrite(GUEST_FS_LIMIT, fs_limit);
    err |= vmwrite(GUEST_GS_LIMIT, gs_limit);
    err |= vmwrite(GUEST_LDTR_LIMIT, ldt_limit);
    err |= vmwrite(GUEST_TR_LIMIT, tr_limit);

    if(err)
    {
        printf("lhy: failed. [limit]\n");
        return err;
    }

    uint32_t cs_ar = __load_access_right(cs);
    uint32_t ss_ar = __load_access_right(ss);
    uint32_t ds_ar = __load_access_right(ds);
    uint32_t es_ar = __load_access_right(es);
    uint32_t fs_ar = __load_access_right(fs);
    uint32_t gs_ar = __load_access_right(gs);
    uint32_t ldt_ar = __load_access_right(ldtr);
    uint32_t tr_ar = __load_access_right(tr);

    err |= vmwrite(GUEST_CS_AR, cs_ar);
    err |= vmwrite(GUEST_SS_AR, ss_ar);
    err |= vmwrite(GUEST_DS_AR, ds_ar);
    err |= vmwrite(GUEST_ES_AR, es_ar);
    err |= vmwrite(GUEST_FS_AR, fs_ar);
    err |= vmwrite(GUEST_GS_AR, gs_ar);
    err |= vmwrite(GUEST_LDTR_AR, ldt_ar);
    err |= vmwrite(GUEST_TR_AR, tr_ar);

    if(err)
    {
        printf("lhy: failed. [access right]\n");
        return err;
    }


    struct region_descriptor gdt, idt;

    __store_gdt(&gdt);
    __store_idt(&idt);

    err |= vmwrite(GUEST_GDTR_BASE, gdt.rd_base);
    err |= vmwrite(GUEST_IDTR_BASE, idt.rd_base);

    if(err)
    {
        printf("lhy: failed. [gdt and idt]\n");
        return err;
    }

    err |= vmwrite(GUEST_IA32_DEBUGCTL, rdmsr(MSR_DEBUGCTLMSR));
    err |= vmwrite(GUEST_IA32_SYSENTER_CS, rdmsr(MSR_SYSENTER_CS_MSR));
    err |= vmwrite(GUEST_IA32_SYSENTER_ESP, rdmsr(MSR_SYSENTER_ESP_MSR));
    err |= vmwrite(GUEST_IA32_SYSENTER_EIP, rdmsr(MSR_SYSENTER_EIP_MSR));

    if(err)
    {
        printf("lhy: failed. [msrs]\n");
        return err;
    }

    return err;
}

static int setup_vmcs_guest_non_register_state(void)
{
    int err = 0;

    err |= vmwrite(GUEST_ACTIVITY_STATE, 0); // 0 means the guest is active.
    err |= vmwrite(GUEST_INTERRUPTIBILITY_INFO, 0); // clear. no interrupt occured.
    err |= vmwrite(GUEST_PENDING_DBG_EXCPT, 0);
	err |= vmwrite(VMCS_LINK_POINTER, ~0ULL);

    return err;
}

static int setup_vmcs_guest_field(uint64_t rip, uint64_t rsp)
{
    int err = 0;

    // 24.4.1 Guest Register State
    err = setup_vmcs_guest_register_state(rip, rsp);
    if(err)
    {
        printf("lhy: failed. [guest_register_state]\n");
        return err;
    }

    // 24.4.2 Guest Non-Register State
    err = setup_vmcs_guest_non_register_state();
    if(err)
    {
        printf("lhy: failed. [guest non-register state]\n");
    }

    return err;
}

static MALLOC_DEFINE(M_LHY, "lhy", "a lightweight hypervisor");
int vmx_vm_init(void)
{
    int err = 0;
    uint32_t revision_id;

    guest_vmcs = malloc(sizeof(struct vmcs), M_LHY, M_ZERO);
    if((uintptr_t)guest_vmcs & PAGE_MASK)
    {
        printf("lhy: vmcs area should be aligned page size");
        return -1;
    }

    revision_id = rdmsr(MSR_VMX_BASIC) & 0xffffffff;
    guest_vmcs->revision_id = revision_id;

    err = vmclear(guest_vmcs);
    if(err)
    {
        printf("lhy: vmclear failed\n");
        return err;
    }

    err = vmptrld(guest_vmcs);
    if(err)
    {
        printf("lhy: vmptrld failed\n");
        return err;
    }

    // TODO
    uintptr_t rip = 0;
    uintptr_t rsp = 0;
    err = setup_vmcs_guest_field(rip, rsp);
    if(err)
    {
        printf("lhy: setup failed the vmcs guest filed\n");
    }

    //TODO
    return err;
}

static void vmx_shutdown(void *junk)
{
    vmxoff();
    printf("lhy: vmxoff [%d]\n", curcpu);
}

int vmx_deinit(void)
{
    int err = 0;
    if(guest_vmcs != NULL)
    {
        err = vmclear(guest_vmcs);
        if(err) {}
        free(guest_vmcs, M_LHY);
        guest_vmcs = NULL;
        printf("lhy: guest_vmcs clear and free\n");
    }

    smp_rendezvous(NULL, vmx_shutdown, NULL, NULL);
    return err;
}
