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

void guest_entry(void);

static uint8_t guest_stack[PAGE_SIZE] __aligned(PAGE_SIZE);
//static uint8_t host_stack[PAGE_SIZE] __aligned(PAGE_SIZE);

struct pcpu;
extern struct pcpu __pcpu[];

struct vmcs {
    uint32_t revision_id;
    uint32_t abort_code;
    char data[PAGE_SIZE - sizeof(uint32_t) * 2];
};

static struct vmx_host_state host_state __aligned(0x10);
//static struct vmx_guest_state guest_state;

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
    __asm__ volatile("vmclear %1; setna %0;"
            : "=r"(error) : "m"(*(uint64_t *)&pa_region): "memory");

    return error;
}

static int vmptrld(struct vmcs* region)
{
    uint8_t error;
    uint64_t pa_region;

    pa_region = vtophys(region);
    __asm__ volatile("vmptrld %1; setna %0"
            : "=r"(error) : "m"(*(uint64_t *)&pa_region): "memory");

    return error;
}

static int vmwrite(uint64_t field, uint64_t value)
{
    uint8_t error;
    __asm__ volatile("vmwrite %2, %1; setna %0;"
            : "=r"(error) : "r"(field), "r"(value) : "memory");
    return error;
}

static int vmread(uint64_t field, uint64_t *ptr)
{
    uint8_t error;
    __asm__ volatile("vmread %1, %2; setna %0;"
            : "=r"(error) : "r"(field), "m"(*ptr): "memory");
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

static int vmcs_dump(void)
{
    int err = 0;

    uint64_t val;

    err |= vmread(GUEST_ES_SELECTOR, &val);
    printf("lhy: es selector:    %lx\n", val);

    err |= vmread(GUEST_CS_SELECTOR, &val);
    printf("lhy: cs selector:    %lx\n", val);

    err |= vmread(GUEST_SS_SELECTOR, &val);
    printf("lhy: ss selector:    %lx\n", val);

    err |= vmread(GUEST_DS_SELECTOR, &val);
    printf("lhy: ds selector:    %lx\n", val);

    err |= vmread(GUEST_FS_SELECTOR, &val);
    printf("lhy: fs selector:    %lx\n", val);

    err |= vmread(GUEST_GS_SELECTOR, &val);
    printf("lhy: gs selector:    %lx\n", val);

    err |= vmread(GUEST_LDTR_SELECTOR, &val);
    printf("lhy: ldtr selector:  %lx\n", val);

    err |= vmread(GUEST_TR_SELECTOR, &val);
    printf("lhy: tr selector:    %lx\n", val);

    if(err)
    {
        printf("failed the vmread. [guest selector]\n");
        return err;
    }

    err |= vmread(GUEST_CR0, &val);
    printf("lhy: cr0:       %lx\n", val);

    err |= vmread(GUEST_CR3, &val);
    printf("lhy: cr3:       %lx\n", val);

    err |= vmread(GUEST_CR4, &val);
    printf("lhy: cr4:       %lx\n", val);

    err |= vmread(GUEST_RFLAGS, &val);
    printf("lhy: rflags:    %lx\n", val);

    err |= vmread(GUEST_RIP, &val);
    printf("lhy: rip:       %lx\n", val);

    err |= vmread(GUEST_RSP, &val);
    printf("lhy: RSP:       %lx\n", val);

    err |= vmread(GUEST_ES_BASE, &val);
    printf("lhy: ES_BASE    %lx\n", val);

    err |= vmread(GUEST_CS_BASE, &val);
    printf("lhy: CS_BASE    %lx\n", val);

    err |= vmread(GUEST_SS_BASE, &val);
    printf("lhy: SS_BASE    %lx\n", val);

    err |= vmread(GUEST_DS_BASE, &val);
    printf("lhy: DS_BASE    %lx\n", val);

    err |= vmread(GUEST_FS_BASE, &val);
    printf("lhy: FS_BASE    %lx\n", val);

    err |= vmread(GUEST_GS_BASE, &val);
    printf("lhy: GS_BASE    %lx\n", val);

    err |= vmread(GUEST_LDTR_BASE, &val);
    printf("lhy: LDTR_BASE  %lx\n", val);

    err |= vmread(GUEST_TR_BASE, &val);
    printf("lhy: TR_BASE    %lx\n", val);

    if(err)
    {
        printf("lhy: TODO\n");
        return err;
    }


    err |= vmread(GUEST_GDTR_BASE, &val);
    printf("lhy: gdt base   %lx\n", val);

    err |= vmread(GUEST_IDTR_BASE, &val);
    printf("lhy: idt base   %lx\n", val);

    if(err)
    {
        printf("lhy: TODO\n");
        return err;
    }

    err |= vmread(HOST_RSP, &val);
    printf("lhy: host rsp   %lx\n", val);

    err |= vmread(HOST_RIP, &val);
    printf("lhy: host rip   %lx\n", val);

    if(err)
    {
        printf("lhy: host regs\n");
        return err;
    }

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

	uint16_t es = __read_es() & 0xf8;
	uint16_t cs = __read_cs() & 0xf8;
	uint16_t ss = __read_ss() & 0xf8;
	uint16_t ds = __read_ds() & 0xf8;
	uint16_t fs = __read_fs() & 0xf8;
	uint16_t gs = __read_gs() & 0xf8;
	uint16_t ldtr = __read_ldt();
	uint16_t tr = __read_tr() & 0xf8;

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

extern int vmx_entry_guest(struct vmx_host_state* hstate);
int vmx_exit_guest(void);

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
        return err;
    }

    return err;
}

static int setup_vmcs_host_field(uint64_t rip, uint64_t rsp)
{
    int err = 0;

    uint64_t cr0 = __read_cr0();
    uint64_t cr3 = __read_cr3();
    uint64_t cr4 = __read_cr4();

    printf("lhy: cr0    %lx\n", cr0);
    printf("lhy: cr3    %lx\n", cr3);
    printf("lhy: cr4    %lx\n", cr4);

    err |= vmwrite(HOST_CR0, cr0);
    err |= vmwrite(HOST_CR3, cr3);
    err |= vmwrite(HOST_CR4, cr4);

    if(err)
    {
        printf("lhy: failed. [host control registers]\n");
        return err;
    }

    printf("lhy: %lx %lx\n", rip, rsp);
    err |= vmwrite(HOST_RIP, rip);
    err |= vmwrite(HOST_RSP, rsp);

    if(err)
    {
        printf("lhy: failed. [host rip and rsp]\n");
        return err;
    }


	uint16_t es = __read_es();
	uint16_t cs = __read_cs();
	uint16_t ss = __read_ss();
	uint16_t ds = __read_ds();
	uint16_t fs = __read_fs();
	uint16_t gs = __read_gs();
	uint16_t tr = __read_tr();

    err |= vmwrite(HOST_ES_SELECTOR, es);
    err |= vmwrite(HOST_CS_SELECTOR, cs);
    err |= vmwrite(HOST_SS_SELECTOR, ss);
    err |= vmwrite(HOST_DS_SELECTOR, ds);
    err |= vmwrite(HOST_FS_SELECTOR, fs);
    err |= vmwrite(HOST_GS_SELECTOR, gs);
    err |= vmwrite(HOST_TR_SELECTOR, tr);

    if(err)
    {
        printf("lhy: failed. [host selectors]\n");
        return err;
    }

    uint64_t fs_base = rdmsr(MSR_FSBASE);
    uint64_t gs_base = (uint64_t)&__pcpu[curcpu];
    err |= vmwrite(HOST_FS_BASE, fs_base);
    err |= vmwrite(HOST_GS_BASE, gs_base);

    uint64_t tr_base = (uint64_t)PCPU_GET(tssp);
    err |= vmwrite(HOST_TR_BASE, tr_base);

    struct region_descriptor gdt, idt;

    __store_gdt(&gdt);
    __store_idt(&idt);

    err |= vmwrite(HOST_GDTR_BASE, gdt.rd_base);
    err |= vmwrite(HOST_IDTR_BASE, idt.rd_base);

    if(err)
    {
        printf("lhy: failed. [host base]\n");
        return err;
    }

    err |= vmwrite(HOST_IA32_SYSENTER_CS, rdmsr(MSR_SYSENTER_CS_MSR));
    err |= vmwrite(HOST_IA32_SYSENTER_ESP, rdmsr(MSR_SYSENTER_ESP_MSR));
    err |= vmwrite(HOST_IA32_SYSENTER_EIP, rdmsr(MSR_SYSENTER_EIP_MSR));
    if(err)
    {
        printf("lhy: failed. [host msrs]\n");
        return err;
    }

    return err;
}

static void adjust_control_value(uint32_t msr, uint32_t offset, uint32_t* value)
{
    uint64_t msr_val = rdmsr(msr + offset);
    printf("lhy: msr_val 0x%lx\n", msr_val);
    *value &= (uint32_t)(msr_val >> 32);
    *value |= (uint32_t)msr_val;
}

static int setup_vmcs_vm_execution_control_fields(void)
{
    int err = 0;

    uint64_t vmx_basic = rdmsr(MSR_VMX_BASIC);
    uint32_t msr_true_offset = 0;
    if(vmx_basic & VMX_BASIC_TRUE_CTLS)
    {
        msr_true_offset = 0xc;
    }

    // 24.6 vm execution control fields
    uint32_t pin_based_vm_exec_ctrls = 0;
    adjust_control_value(MSR_VMX_PINBASED_CTLS, msr_true_offset, &pin_based_vm_exec_ctrls);

    uint32_t cpu_based_vm_exec_ctrls = CPU_BASED_HLT_EXITING;
        //CPU_BASED_HLT_EXITING | CPU_BASED_ACTIVATE_SECONDARY_CONTROLS;
    adjust_control_value(MSR_VMX_PROCBASED_CTLS, msr_true_offset,
            &cpu_based_vm_exec_ctrls);

    // XXX: below code, rdmsr(MSR_VMX_PROCBASED_CTLS2) in adjust_control_value,
    //      cause exception and panic the kernel. why..
    //uint32_t second_cpu_based_vm_exec_ctrols = 0;
    //adjust_control_value(MSR_VMX_PROCBASED_CTLS2, msr_true_offset,
    //        &second_cpu_based_vm_exec_ctrols);

    // 24.7 vm-exit control fields
    uint32_t vm_exit_ctrls = VM_EXIT_HOST_ADDR_SPACE_SIZE;
    adjust_control_value(MSR_VMX_EXIT_CTLS, msr_true_offset, &vm_exit_ctrls);

    // 24.8 vm-entry control fields
    uint32_t vm_entry_ctrls = VM_ENTRY_IA32E_MODE;
    adjust_control_value(MSR_VMX_ENTRY_CTLS, msr_true_offset, &vm_entry_ctrls);

    err |= vmwrite(PIN_BASED_VM_EXEC_CONTROL, pin_based_vm_exec_ctrls);
    err |= vmwrite(CPU_BASED_VM_EXEC_CONTROL, cpu_based_vm_exec_ctrls);
    //err |= vmwrite(SECONDARY_VM_EXEC_CONTROL, second_cpu_based_vm_exec_ctrols);
    err |= vmwrite(VM_EXIT_CONTROLS, vm_exit_ctrls);
    err |= vmwrite(VM_ENTRY_CONTROLS, vm_entry_ctrls);

    if(err)
    {
        printf("lhy: failed. [vm {pin,cpu,2nd,entry,exit} control]\n");
        return err;
    }

    //24.8.2 vm-entry controls for MSRs
    err |= vmwrite(VM_ENTRY_MSR_LOAD_COUNT, 0);
    err |= vmwrite(VM_ENTRY_MSR_LOAD_ADDR, 0);

    //24.8.3
    err |= vmwrite(VM_ENTRY_INTR_INFO_FIELD, 0);
    err |= vmwrite(VM_ENTRY_EXCEP_ERROR_CODE, 0);
    err |= vmwrite(VM_ENTRY_INTR_INFO_FIELD, 0);

    if(err)
    {
        printf("lhy: failed [vm-entry fields]\n");
        return err;
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
    uintptr_t guest_rip = (uintptr_t)guest_entry;
    uintptr_t guest_rsp = (uintptr_t)guest_stack + PAGE_SIZE - 8; // - 0x10;
    err = setup_vmcs_guest_field(guest_rip, guest_rsp);
    if(err)
    {
        printf("lhy: setup failed the vmcs guest filed\n");
        return err;
    }

    uintptr_t host_rip = (uintptr_t)vmx_exit_guest;
    uintptr_t host_rsp = (uintptr_t)&host_state;
    err = setup_vmcs_host_field(host_rip, host_rsp);
    if(err)
    {
        printf("lhy: failed [setup_vmcs_host field]\n");
        return err;
    }

    // 24.{6,7,8,9}
    err = setup_vmcs_vm_execution_control_fields();
    if(err)
    {
        printf("lhy: failed. [setup_vmcs_vm_execution_control_fields]\n");
        return err;
    }

    printf("lhy: vmcs setup successed\n");
    err = vmcs_dump();
    if(err)
    {
        printf("lhy: failed. [vmcs_dump]\n");
        return err;
    }

    return 0;
}

int vmx_vcpu_run(void)
{
    int err = 0;

    //err = vmx_entry_guest(&host_state);

    __cli();
    __asm__ volatile(
            "movq %%r15, 0x00(%1);"
            "movq %%r14, 0x08(%1);"
            "movq %%r13, 0x10(%1);"
            "movq %%r12, 0x16(%1);"
            "movq %%rbp, 0x20(%1);"
            "movq %%rsp, 0x28(%1);"
            "movq %%rbx, 0x30(%1);"
            "movq %%rdi, %%rsp;"
            "vmlaunch;"
            "movl $1, %0;"
            "jmp .launch_failed;"
            ".globl vmx_exit_guest;"
            "vmx_exit_guest:;"
            "movq %%rsp, %%rdi;"
            "movq 0x00(%%rdi), %%r15;"
            "movq 0x08(%%rdi), %%r14;"
            "movq 0x10(%%rdi), %%r13;"
            "movq 0x18(%%rdi), %%r12;"
            "movq 0x20(%%rdi), %%rbp;"
            "movq 0x28(%%rdi), %%rsp;"
            "movq 0x30(%%rdi), %%rbx;"
            "movl $0, %0;"
            ".a:;"
            "jmp .a;"
            ".launch_failed:;"
            : "=r"(err)
            : "r"(&host_state)
            : "memory", "rdi", "r15", "r14", "r13", "r12", "rbp", "rsp", "rbx"
            );


    if(err)
    {
        printf("lhy: failed. [vmx_entry_guest]\n");
        printf("lhy: host_state\n");
        printf("lhy: r15 %lx\n", host_state.r15);
        printf("lhy: r14 %lx\n", host_state.r14);
        printf("lhy: r13 %lx\n", host_state.r13);
        printf("lhy: r12 %lx\n", host_state.r12);
        printf("lhy: rbp %lx\n", host_state.rbp);
        printf("lhy: rsp %lx\n", host_state.rsp);
        printf("lhy: rbx %lx\n", host_state.rbx);
        return err;
    }

    __sti();

    // TODO: read the exit reason.

    return 0;
}

static void vmx_shutdown(void *junk)
{
    vmxoff();
    printf("lhy: vmxoff [%d]\n", curcpu);
}

void debug_print(uint64_t val){
    printf("lhy: debug_print %lx\n", val);
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
