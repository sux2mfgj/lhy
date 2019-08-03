#include <machine/asmacros.h>
#include <machine/asm.h>
#include <machine/specialreg.h>

.text
.globl guest_entry

guest_entry:
    hlt
    hlt
    hlt

    /*
.globl vmx_entry_guest
// int vmx_entry_guest(struct vmx_host_state* hstate);
// %rdi: struct vmx_host_state*
vmx_entry_guest:
    //push %rbp
    cli
    movq %r15, 0(%rdi)
    movq %r14, 8(%rdi)
    movq %r13, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %rbp, 32(%rdi)
    movq %rsp, 40(%rdi)
    movq %rbx, 48(%rdi)

    //call debug_print

    //movq %rdi, %rsp
    vmlaunch

    //setbe %dil
    //movq %rax, %rax
    //movb %dil, %al

    movq $1, %rax
    popq %rbp

    retq


.globl vmx_exit_guest
// int vmx_exit_guest(void);
vmx_exit_guest:
    movq %rsp, %rdi

    movq 0(%rdi), %r15
    movq 8(%rdi), %r14
    movq 16(%rdi), %r13
    movq 24(%rdi), %r12
    movq 32(%rdi), %rbp
    movq 40(%rdi), %rsp
    movq 48(%rdi), %rbx

    //sti

    //call debug_print
//.a:
    //jmp .a
    movq $0, %rax

    //popq %rbp

    retq
    */
