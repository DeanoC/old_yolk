/*
 * /Z/Projects/Cyberspace/yolk_repo/src/libs/dwm/mmu.h
 *
 * (c) Copyright Cloud Pixies Ltd.
 */
#pragma once

#ifndef DWM_MMU_H_
#define DWM_MMU_H_ 1

#include "core/singleton.h"

#if PLATFORM_OS == MS_WINDOWS
#else
#	include <sys/mman.h>
#endif

#if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
struct LdtEntry {
  uint16_t limit_00to15;
  uint16_t base_00to15;

  unsigned int base_16to23 : 8;

  unsigned int type : 5;
  unsigned int descriptor_privilege : 2;
  unsigned int present : 1;

  unsigned int limit_16to19 : 4;
  unsigned int available : 1;
  unsigned int code_64_bit : 1;
  unsigned int op_size_32 : 1;
  unsigned int granularity : 1;

  unsigned int base_24to31 : 8;
};

/* to make LDT_ENTRIES available */
#	if PLATFORM_OS == MS_WINDOWS
# 		define LDT_ENTRIES 8192
#	elif PLATFORM_OS == APPLE_MAC
#		define LDT_ENTRIES 8192
#	elif PLATFORM_OS == LINUX
//#		include <asm/ldt.h>
#		include <sys/mman.h>
//		extern int modify_ldt(int func, void* ptr, unsigned long bytecount);
#	endif

#endif // 32 bit x86

class MMU : public Core::Singleton<MMU> {
public:
	friend class Core::Singleton<MMU>;
#	if PLATFORM_OS == MS_WINDOWS
	static const unsigned int PAGE_NONE = 0x0;   // Page can't be accessed.
	static const unsigned int PAGE_READ = 0x1;   // Page can be read.
	static const unsigned int PAGE_WRITE = 0x2;   // Page can be written.
	static const unsigned int PAGE_EXEC  = 0x4;   // Page can be executed.
#else
	static const unsigned int PAGE_NONE = PROT_NONE;   // Page can't be accessed.
	static const unsigned int PAGE_READ = PROT_READ;   // Page can be read.
	static const unsigned int PAGE_WRITE = PROT_WRITE;   // Page can be written.
	static const unsigned int PAGE_EXEC  = PROT_EXEC;   // Page can be executed.
#endif

	// how big each page is
	size_t getPageSize() const { return pageSize; }

	// return a valid but random page address, to improve security
//	uintptr_t getRandomPageAddress();

	// allocate pages to contain numBytes (round up), start of range return in hintOut
	// if the variable pointed at by hintOut is not null on entry, system will try and
	// place the pages at that address. However its only a hint and my be unable to.
	// pages are allocated with no access
	void allocPages( void** hintOut, size_t numBytes );

	// reserve without actually commiting a range of pages ala allocPages
	void reservePages( void** hintOut, size_t numBytes );

	// commit all or a subset of previously reserved pages
	void commitPages( void** hintOut, size_t numBytes );

	// free numBytes (rounded up) pages starting at pages address will uncommit as required
	void freePages( void* pages, size_t numBytes );

	// change the protection setting for the pages to settings if not PROT_NONE also commits
	// the range
	void protectPages( void* pages, size_t numBytes, unsigned int settings );

// 32bit x86 has selectors for segmented memory model
#if 0
//CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
	// selectors can be code or data and are page sized and aligned (numBytes rounded up)
	// the settings are a limited subset of page level, its either RWX or RX, R will map to RX 
	// so beware! Use page protection for finer control
	uint16_t allocSelector( bool data, void* pages, size_t numBytes, unsigned int settings );
	void freeSelector( uint16_t selector );
#endif
private:
	MMU();
	void allocPagesWithFlags( void** hintOut, size_t numBytes, unsigned int flags );

	size_t pageSize;

#if 0
//CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
	std::array<LdtEntry, LDT_ENTRIES> ldtTable;
#endif
	
};

#endif