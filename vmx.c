#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/smp.h>
#include <sys/pcpu.h>
#include <sys/malloc.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/cpufunc.h>
#include <machine/specialreg.h>

#include "vmx.h"
#include "x64.h"

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

int setup_vmcs_guest_field(uint64_t rip, uint64_t rsp);

//static int setup_vmcs_guest_field(uint64_t rip, uint64_t rsp)
int setup_vmcs_guest_field(uint64_t rip, uint64_t rsp)
{
    int err = 0;
    return err;

    // 24.4.1 Guest Register State
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
    //uint64_t rflags = __read_rflags();
    //err |= vmwrite(GUEST_RFLAGS, rflags);
    //err |= vmwrite(GUEST_RIP, rip);
    //err |= vmwrite(GUEST_RSP, rsp);

    //if(err)
    //{
    //    printf("lhy: error occured while writing rsp, rip or rflags\n");
    //    return err;
    //}

	uint16_t es = __read_es();
	uint16_t cs = __read_cs();
	uint16_t ss = __read_ss();
	uint16_t ds = __read_ds();
	uint16_t fs = __read_fs();
	uint16_t gs = __read_gs();
	uint16_t ldtr = __read_ldt();
	uint16_t tr = __read_tr();

    // configuration of guest state areas.
    err |= vmwrite(GUEST_ES_SELECTOR, es);
    err |= vmwrite(GUEST_CS_SELECTOR, cs);
    err |= vmwrite(GUEST_SS_SELECTOR, ss);
    err |= vmwrite(GUEST_DS_SELECTOR, ds);
    err |= vmwrite(GUEST_FS_SELECTOR, fs);
    err |= vmwrite(GUEST_GS_SELECTOR, gs);
    err |= vmwrite(GUEST_LDTR_SELECTOR, ldtr);
    err |= vmwrite(GUEST_TR_SELECTOR, tr);

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
