/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 * Customized to york and rpg.sh by deano@cloudpixies.com
 */
        .text
        .globl SwitchToUntrustedAVXLinux
        .type SwitchToUntrustedAVXLinux, @function
SwitchToUntrustedAVXLinux:
        /* if Windows, 1st param is already in %rcx, not %rdi */
        /* elif Linux/OSX, 1st param is already in %rdi. */
        mov     %rdi, %rcx
.globl SwitchToUntrustedAVX
.type SwitchToUntrustedAVX, @function
SwitchToUntrustedAVX:
	/* rcx = IEEThreadContext* */
        movq    0(%rcx), 			%rbx
        movq    8(%rcx), 			%r12
        movq    16(%rcx), 			%r13 
        movq    24(%rcx), 			%r14
        movq    %rsp,                           64(%rcx) /* save trusted stack */
        movq    32(%rcx), 			%rsp  	/* rsp -- switch stack */
        movq    32(%rcx), 			%rbp	/* rbp == rsp initially */

        /** Zero all unused registers. */
        fninit
        fldcw   40(%rcx)				/* fcw */
        xorl    %edx, 				%edx
        movl    %edx, 				%eax
        movl    %edx, 				%esi
        movq    %rdx, 				%r8
        movq    %rdx, 				%r9
        movq    %rdx, 				%r10
        movq    %rdx, 				%r11
        vxorps   %ymm0, %ymm0, 		%ymm0
        vxorps   %ymm1, %ymm1, 		%ymm1
        vxorps   %ymm2, %ymm2, 		%ymm2
        vxorps   %ymm3, %ymm3, 		%ymm3
        vxorps   %ymm4, %ymm4, 		%ymm4
        vxorps   %ymm5, %ymm5, 		%ymm5
        vxorps   %ymm6, %ymm6, 		%ymm6
        vxorps   %ymm7, %ymm7, 		%ymm7
        vxorps   %ymm8, %ymm8, 		%ymm8
        vxorps   %ymm9, %ymm9, 		%ymm9
        vxorps   %ymm10, %ymm10, 	%ymm10
        vxorps   %ymm11, %ymm11, 	%ymm11
        vxorps   %ymm12, %ymm12, 	%ymm12
        vxorps   %ymm13, %ymm13, 	%ymm13
        vxorps   %ymm14, %ymm14, 	%ymm14
        vxorps   %ymm15, %ymm15, 	%ymm15

        movq	48(%rcx), 			%r15	/* set the untrusted memory base register */
        leal    8(%rsp), 			%edi	/* move from untrusted stack to linux x86-64 ABI registers */
        movq    56(%rcx), 			%rcx 	/* two instructions to hide ThreadContext and set untrusted rip */
        jmp     *%rcx

.globl SwitchToUntrustedSSELinux
.type SwitchToUntrustedSSELinux, @function
SwitchToUntrustedSSELinux:
        /* if Windows, 1st param is already in %rcx */
        /* elif Linux/OSX, 1st param is in %rdi. */
        mov     %rdi, %rcx
.globl SwitchToUntrustedSSE
.type SwitchToUntrustedSSE, @function
SwitchToUntrustedSSE:
        movq    0(%rcx), 			%rbx
        movq    8(%rcx), 			%r12
        movq    16(%rcx), 			%r13 
        movq    24(%rcx), 			%r14

        movq    %rsp,                           64(%rcx) /* save trusted stack */
        movq    32(%rcx), 			%rsp  	/* rsp -- switch stack */
        movq    32(%rcx), 			%rbp	/* rbp == rsp initially */

        /* Zero all unused registers. */
        fninit
        fldcw   40(%rcx)					/* fcw */
        xorl    %edx, 				%edx
        movl    %edx, 				%eax
        movl    %edx, 				%esi
        movq    %rdx, 				%r8
        movq    %rdx, 				%r9
        movq    %rdx, 				%r10
        movq    %rdx, 				%r11
        xorps   %xmm0, 		 		%xmm0
        xorps   %xmm1, 		 		%xmm1
        xorps   %xmm2, 		 		%xmm2
        xorps   %xmm3, 		 		%xmm3
        xorps   %xmm4, 		 		%xmm4
        xorps   %xmm5, 		 		%xmm5
        xorps   %xmm6, 		 		%xmm6
        xorps   %xmm7, 		 		%xmm7
        xorps   %xmm8, 		 		%xmm8
        xorps   %xmm9, 		 		%xmm9
        xorps   %xmm10,			 	%xmm10
        xorps   %xmm11,			 	%xmm11
        xorps   %xmm12,			 	%xmm12
        xorps   %xmm13,			 	%xmm13
        xorps   %xmm14,			 	%xmm14
        xorps   %xmm15,			 	%xmm15

        movq	48(%rcx), 			%r15	/* set the untrusted memory base register */
        leal    8(%rsp), 			%edi	/* move from untrusted stack to linux x86-64 ABI registers */
        movq    56(%rcx), 			%rcx 	/* two instructions to hide ThreadContext and set untrusted rip */
        jmp     *%rcx

/* synthesise  instructions (movq threadCtx, %rax; mov %rax, %r15; movq fptr, %rax) at the label */
.globl SwitchToTrustedWin
.type SwitchToTrustedWin, @function
SwitchToTrustedWin:
	movq	%rsp, 				32(%r15)	/* save the untrusted stack		*/
	movq	64(%r15), 			%rsp		/* restore the trusted stack		*/
	pushq					%rbp	/* save the untrusted ebp 			*/
	subq	$32,				%rsp		/* windows shadow stack space		*/
	/* untrusted code uses Linux x86-64 call convention
	   so we convert it to windows x86-64 here
	   system calls only support 3 interger/pointer parameters */
	movq	%rdx,				%r9		/* 3rd linux param = 4th win param */
	movq	%rsi, 				%r8		/* 2nd linux param = 3rd win param */
	movq	%rdi,				%rdx		/* 1st linux param = 2nd win param */
	movq	%r15,				%rcx		/* 1st param IEEThreadContext  */
	call 					*%rax		/* call into win64 abi function return in rax */
	addq	$32,				%rsp
	popq					%rbp		/* restore the untrusted ebp		*/
	movq	32(%r15), 			%rsp		/* restore the untrusted stack		*/
	movq	48(%r15), 			%r15		/* r15 back to untrusted base addr 	*/

	ret 										/* return to the untrusted code 	*/
.globl SwitchToTrustedWinEnd
.type SwitchToTrustedWinEnd, @function
SwitchToTrustedWinEnd:

.globl SwitchToTrustedLinux
.type SwitchToTrustedLinux, @function
SwitchToTrustedLinux:
        /* synthesise instructions here (movq threadCtx, %rcx; movq funcptr, %rax) */
        pushq   %r12                                     
        movq    %rsp,                           32(%rcx)        /* save the untrusted stack                             */
        movq    64(%rcx),                       %rsp            /* restore the trusted stack                            */
        pushq                                   %r15            /* save r15                                             */

        /* system calls only support 3 interger/pointer parameters                                                      */
        movq    %rcx,                           %r12            /* r12 is callee saved and restored                     */
        movq    %rdx,                           %rcx            /* 3rd untrusted param = 4th trusted param              */
        movq    %rsi,                           %rdx            /* 2nd untrusted param = 3rd trusted param              */
        movq    %rdi,                           %rsi            /* 1st untrusted param = 2nd trusted param              */
        movq    %r12,                           %rdi            /* IEEThreadContext = 1st trusted param                 */

        call                                    *%rax           /* call into linux64 abi function return in rax,rbx     */

        popq                                    %r15            /* restore r15                                          */
        movq    32(%r12),                       %rsp            /* restore the untrusted stack                          */
        popq    %r12
        ret                                                     /* return to the untrusted code                         */
.globl SwitchToTrustedLinuxEnd
.type SwitchToTrustedLinuxEnd, @function
SwitchToTrustedLinuxEnd: