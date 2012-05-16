/*
 * ieethreadcontext.h
 */
#pragma once
#ifndef YOLK_DWM_IEETHREADCONTEXT_H_
#define YOLK_DWM_IEETHREADCONTEXT_H_

class IsolatedExecEngine;
#if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
# error TODO
#elif CPU_FAMILY == CPU_X64
struct IEEThreadContext {
	uint64_t  	rbx;				//  0
	uint64_t  	r12;				//  8
	uint64_t  	r13;				// 16
	uint64_t  	r14;				// 24
	uint64_t  	untrusted_stack;	// 32
  	uint16_t    fcw;				// 40
  	uint16_t	padd[3];			// 42
	uint64_t  	membase;			// 48
	// the above are the untrusted initial registers
	uint64_t	untrusted_rip;		// 56
	uint64_t	trusted_stack;		// 64
	IsolatedExecEngine* owner;		// 72
};									// 80
#endif

#endif