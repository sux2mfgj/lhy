.text
.globl guest_entry

guest_entry:
    hlt
    hlt
    hlt

.globl vmx_entry_guest
// int vmx_entry_guest(struct vmx_host_state* hstate);
// %rdi: struct vmx_host_state*
vmx_entry_guest:
    movq %r15, 0(%rdi)
    movq %r14, 8(%rdi)
    movq %r13, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %rbp, 32(%rdi)
    movq %rsp, 40(%rdi)
    movq %rbx, 48(%rdi)
    pushq %rdi

    vmlaunch
    setbe %dil
    movq %rax, %rax
    movb %dil, %al

    ret

.globl
// int vmx_exit_guest(void);
vmx_exit_guest:
    popq %rdi
    movq 0(%rdi), %r15
    movq 8(%rdi), %r14
    movq 16(%rdi), %r13
    movq 24(%rdi), %r12
    movq 32(%rdi), %rbp
    movq 40(%rdi), %rsp
    movq 48(%rdi), %rbx

    movq $0, %rax

    ret

