/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * Customized to york and rpg.sh by deano@cloudpixies.com
 */
        .text
#if NACL_WINDOWS
        /* if Windows, 1st param is already in %rcx, not %rdi */
#elif NACL_LINUX || NACL_OSX
        /* elif Linux/OSX, 1st param is already in %rdi. */
        mov     %rdi, %rcx
#endif
        .globl SwitchToUntrustedSSE
        .type SwitchToUntrustedSSE, @function
SwitchToUntrustedSSE:
        movq    8(%rcx), %rbx
        movq    32(%rcx), %rbp
        movq    96(%rcx), %r12
        movq    104(%rcx), %r13 
        movq    112(%rcx), %r14
        movq    120(%rcx), %r15
        movq    56(%rcx), %rsp  	/* rsp -- switch stack */
        movq    144(%rcx), %rax  	/* syscall return */

        /*
         * %rdi is the first argument in the user calling convention.
         */
        leal    8(%rsp), %edi

        /*
         * Zero all unused registers.  The 32-bit instructions
         * are a byte shorter than their 64-bit counterparts
         * when the target register is one of the first eight,
         * and they implicitly zero the high halves.
         */
        xorl    %edx, %edx
        movl    %edx, %esi
        movq    %rdx, %r8
        movq    %rdx, %r9
        movq    %rdx, %r10
        movq    %rdx, %r11
        fninit
        fldcw   152(%rcx)	/* fcw */
        xorps   %xmm0, %xmm0
        xorps   %xmm1, %xmm1
        xorps   %xmm2, %xmm2
        xorps   %xmm3, %xmm3
        xorps   %xmm4, %xmm4
        xorps   %xmm5, %xmm5
        xorps   %xmm6, %xmm6
        xorps   %xmm7, %xmm7
        xorps   %xmm8, %xmm8
        xorps   %xmm9, %xmm9
        xorps   %xmm10, %xmm10
        xorps   %xmm11, %xmm11
        xorps   %xmm12, %xmm12
        xorps   %xmm13, %xmm13
        xorps   %xmm14, %xmm14
        xorps   %xmm15, %xmm15

        movq    136(%rcx), %rcx /* two instructions to hide ThreadContext and setting newrip */
        jmp     *%rcx

        .globl SwitchToUntrustedAVX
        .type SwitchToUntrustedAVX, @function
SwitchToUntrustedAVX:
        movq    8(%rcx), %rbx
        movq    32(%rcx), %rbp
        movq    96(%rcx), %r12
        movq    104(%rcx), %r13 
        movq    112(%rcx), %r14
        movq    120(%rcx), %r15
        movq    56(%rcx), %rsp  	/* rsp -- switch stack */
        movq    144(%rcx), %rax  	/* syscall return */

        /*
         * %rdi is the first argument in the user calling convention.
         */
        leal    8(%rsp), %edi

        /*
         * Zero all unused registers.  The 32-bit instructions
         * are a byte shorter than their 64-bit counterparts
         * when the target register is one of the first eight,
         * and they implicitly zero the high halves.
         */
        xorl    %edx, %edx
        movl    %edx, %esi
        movq    %rdx, %r8
        movq    %rdx, %r9
        movq    %rdx, %r10
        movq    %rdx, %r11
        fninit
        fldcw   152(%rcx)				/* fcw */
        vxorps   %ymm0, %ymm0, %ymm0
        vxorps   %ymm1, %ymm1, %ymm1
        vxorps   %ymm2, %ymm2, %ymm2
        vxorps   %ymm3, %ymm3, %ymm3
        vxorps   %ymm4, %ymm4, %ymm4
        vxorps   %ymm5, %ymm5, %ymm5
        vxorps   %ymm6, %ymm6, %ymm6
        vxorps   %ymm7, %ymm7, %ymm7
        vxorps   %ymm8, %ymm8, %ymm8
        vxorps   %ymm9, %ymm9, %ymm9
        vxorps   %ymm10, %ymm10, %ymm10
        vxorps   %ymm11, %ymm11, %ymm11
        vxorps   %ymm12, %ymm12, %ymm12
        vxorps   %ymm13, %ymm13, %ymm13
        vxorps   %ymm14, %ymm14, %ymm14
        vxorps   %ymm15, %ymm15, %ymm15

        movq    136(%rcx), %rcx /* two instructions to hide ThreadContext and setting newrip */
        jmp     *%rcx