#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/smp.h>
#include <sys/pcpu.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/cpufunc.h>
#include <machine/specialreg.h>

#include "vmx.h"

static uint8_t vmxon_region[4][PAGE_SIZE] __aligned(PAGE_SIZE);

static int vmxon(char *region)
{
    uint8_t error;
    uint64_t pa_region;

    pa_region = vtophys(region);

    //__asm__ volatile("vmxon %1; setna %0" : "=r"(error) : "m"(pa_region));
    __asm__ volatile("vmxon %1; setna %0" : "=r"(error) : "m"(*(uint64_t*)&pa_region));

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
        printf("vmxon failed\n");
    }
    else
    {
        printf("vmxon success\n");
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
