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
};

#endif
