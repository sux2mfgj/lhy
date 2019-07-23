#ifndef __VMX_H
#define __VMX_H

int vmx_init(void);
int vmx_deinit(void);
int vmx_vm_init(void);

enum vmcs_field {
    GUEST_ES_SELECTOR           = 0x00000800,
    GUEST_CS_SELECTOR           = 0x00000802,
    GUEST_SS_SELECTOR           = 0x00000804,
    GUEST_DS_SELECTOR           = 0x00000806,
    GUEST_FS_SELECTOR           = 0x00000808,
    GUEST_GS_SELECTOR           = 0x0000080a,
    GUEST_LDTR_SELECTOR         = 0x0000080c,
    GUEST_TR_SELECTOR           = 0x0000080e,

    GUEST_CR0                   = 0x00006800,
    GUEST_CR3                   = 0x00006802,
    GUEST_CR4                   = 0x00006804,
    GUEST_DR7                   = 0x0000681a,

    GUEST_RFLAGS                = 0x00006820,
    GUEST_RSP                   = 0x0000681c,
    GUEST_RIP                   = 0x0000681e,

    GUEST_ES_BASE               = 0x00006806,
	GUEST_CS_BASE               = 0x00006808,
	GUEST_SS_BASE               = 0x0000680a,
	GUEST_DS_BASE               = 0x0000680c,
	GUEST_FS_BASE               = 0x0000680e,
	GUEST_GS_BASE               = 0x00006810,
	GUEST_LDTR_BASE             = 0x00006812,
	GUEST_TR_BASE               = 0x00006814,

    GUEST_ES_LIMIT              = 0x00004800,
	GUEST_CS_LIMIT              = 0x00004802,
	GUEST_SS_LIMIT              = 0x00004804,
	GUEST_DS_LIMIT              = 0x00004806,
	GUEST_FS_LIMIT              = 0x00004808,
	GUEST_GS_LIMIT              = 0x0000480a,
	GUEST_LDTR_LIMIT            = 0x0000480c,
	GUEST_TR_LIMIT              = 0x0000480e,
	GUEST_GDTR_LIMIT            = 0x00004810,
	GUEST_IDTR_LIMIT            = 0x00004812,

    GUEST_ES_AR                 = 0x00004814,
	GUEST_CS_AR                 = 0x00004816,
	GUEST_SS_AR                 = 0x00004818,
	GUEST_DS_AR                 = 0x0000481a,
	GUEST_FS_AR                 = 0x0000481c,
	GUEST_GS_AR                 = 0x0000481e,
	GUEST_LDTR_AR               = 0x00004820,
	GUEST_TR_AR                 = 0x00004822,

    GUEST_GDTR_BASE             = 0x00006816,
	GUEST_IDTR_BASE             = 0x00006818,

    GUEST_IA32_DEBUGCTL 		= 0x00002802,
	GUEST_IA32_DEBUGCTL_HIGH 	= 0x00002803,

    GUEST_IA32_SYSENTER_CS 		= 0x0000482A,
    GUEST_IA32_SYSENTER_ESP 	= 0x00006824,
    GUEST_IA32_SYSENTER_EIP 	= 0x00006826,

    GUEST_INTERRUPTIBILITY_INFO = 0x00004824,
	GUEST_ACTIVITY_STATE        = 0X00004826,
    GUEST_PENDING_DBG_EXCPT     = 0x00006822,
    VMCS_LINK_POINTER           = 0x00002800,
	VMCS_LINK_POINTER_HIGH      = 0x00002801,

    HOST_CR0                    = 0x00006c00,
	HOST_CR3                    = 0x00006c02,
	HOST_CR4                    = 0x00006c04,
	HOST_FS_BASE                = 0x00006c06,
	HOST_GS_BASE                = 0x00006c08,
	HOST_TR_BASE                = 0x00006c0a,
	HOST_GDTR_BASE              = 0x00006c0c,
	HOST_IDTR_BASE              = 0x00006c0e,
	HOST_RSP                    = 0x00006c14,
	HOST_RIP                    = 0x00006c16,
    HOST_ES_SELECTOR            = 0x00000c00,
	HOST_CS_SELECTOR            = 0x00000c02,
	HOST_SS_SELECTOR            = 0x00000c04,
	HOST_DS_SELECTOR            = 0x00000c06,
	HOST_FS_SELECTOR            = 0x00000c08,
	HOST_GS_SELECTOR            = 0x00000c0a,
	HOST_TR_SELECTOR            = 0x00000c0c,

    HOST_IA32_SYSENTER_CS       = 0x00004c00,
    HOST_IA32_SYSENTER_ESP      = 0x00006c10,
	HOST_IA32_SYSENTER_EIP      = 0x00006c12,

    PIN_BASED_VM_EXEC_CONTROL   = 0x00004000,
    CPU_BASED_VM_EXEC_CONTROL   = 0x00004002,
    SECONDARY_VM_EXEC_CONTROL   = 0x0000401e,
    VM_EXIT_CONTROLS            = 0x0000400c,
    VM_ENTRY_CONTROLS           = 0x00004012,

    VM_ENTRY_MSR_LOAD_ADDR      = 0x0000200a,
	VM_ENTRY_MSR_LOAD_ADDR_HIGH = 0x0000200b,
    VM_ENTRY_MSR_LOAD_COUNT     = 0x00004014,
    VM_ENTRY_INTR_INFO_FIELD    = 0x00004016,
    VM_ENTRY_EXCEP_ERROR_CODE   = 0x00004018,
	VM_ENTRY_INSTRUCTION_LEN    = 0x0000401a,

};

// VMX_BASIC fileds
#define VMX_BASIC_TRUE_CTLS		(1ULL << 55)


// processor-based vm-execution controls
#define CPU_BASED_HLT_EXITING                   0x00000080
#define CPU_BASED_ACTIVATE_SECONDARY_CONTROLS   0x80000000

// vm-exit controls
#define VM_EXIT_HOST_ADDR_SPACE_SIZE            0x00000200

//vm-entry controls
#define VM_ENTRY_IA32E_MODE                     0x00000200


#endif
