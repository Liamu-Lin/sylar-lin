.text
.globl swap_context
.type swap_context, @function

//swap_context(FiberContext* old_ctx, FiberContext* new_ctx)
swap_context:
    // save current context
    movq %rbx, 0x00(%rdi)   // save general purpose registers
    movq %rbp, 0x08(%rdi)
    movq %rsp, 0x10(%rdi)
    movq %r12, 0x18(%rdi)
    movq %r13, 0x20(%rdi)
    movq %r14, 0x28(%rdi)
    movq %r15, 0x30(%rdi)
    pushfq                  // save rflags
    popq %rax
    movq %rax, 0x40(%rdi)
    movq (%rsp), %rax       // save return address
    movq %rax, 0x48(%rdi)
    
    // load new context
    movq 0x00(%rsi), %rbx   // restore general purpose registers
    movq 0x08(%rsi), %rbp
    movq 0x10(%rsi), %rsp
    movq 0x18(%rsi), %r12
    movq 0x20(%rsi), %r13
    movq 0x28(%rsi), %r14
    movq 0x30(%rsi), %r15
    movq 0x38(%rsi), %rdi   // restore rdi, used when the fiber is first switched to
    movq 0x40(%rsi), %rax   // restore rflags
    pushq %rax
    popfq
    leaq 8(%rsp), %rsp      // overwrite return address
    pushq 0x48(%rsi)

    ret     //ret addr is already on stack