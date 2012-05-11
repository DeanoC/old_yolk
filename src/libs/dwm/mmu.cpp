/*
*/
#include "core/core.h"
#include "mmu.h"

#if PLATFORM_OS == MS_WINDOWS
#	if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32

typedef LONG (NTAPI *NTSETLDT)(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
typedef LONG (NTAPI *NTSETINFO)(HANDLE, DWORD, const VOID*, ULONG);
typedef LONG (WINAPI *NTQUERY)(HANDLE, DWORD, VOID*, DWORD, DWORD*);
static NTQUERY query_information_process;
static NTSETINFO set_information_process;
static NTSETLDT set_ldt_entries;
typedef struct {
  DWORD byte_offset;
  DWORD size;
  struct LdtEntry entries[1];
} LdtInfo;
#	endif
#endif

MMU::MMU() {
#if PLATFORM_OS == MS_WINDOWS
	// determine page size from OS
	SYSTEM_INFO sys_info;
	GetSystemInfo( &sys_info );
	// win32 has both a HW page size (usually 4K) and a granularity (usually 64K)
	// that the virtual allocates, for now we ignore that fact and pretend everything
	// is working in 4K pages. This might be wasteful and interact with MapViewOfFile
	pageSize = sys_info.dwPageSize;
#	if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
	// need to talk to Nt native API for ldt manipulation!
	HMODULE hmod = GetModuleHandleA("ntdll.dll");
	query_information_process = (NTQUERY)(GetProcAddress(hmod, "NtQueryInformationProcess"));
	set_information_process = (NTSETINFO)(GetProcAddress(hmod, "ZwSetInformationProcess"));
	set_ldt_entries = (NTSETLDT)(GetProcAddress(hmod, "NtSetLdtEntries"));
	if (NULL == set_ldt_entries) {
		// fallback just in case NtSet isn't defined, from user space they are the same function
		set_ldt_entries = (NTSETLDT)(GetProcAddress(hmod, "ZwSetLdtEntries"));
	}
	uint32_t err;

	LdtInfo info;
	memset(&info, 0, sizeof(LdtInfo));
	info.byte_offset = (LDT_ENTRIES-1) << 3;
	info.size = sizeof(LdtEntry);
	info.entries[0].base_00to15 = 0;
	info.entries[0].base_16to23 = 0;
	info.entries[0].base_24to31 = 0;
	info.entries[0].limit_00to15 = 0;
	info.entries[0].limit_16to19 = 0;
	info.entries[0].type = 0x10;
	info.entries[0].descriptor_privilege = 3;
	info.entries[0].present = 0;
	info.entries[0].available = 0;
	info.entries[0].code_64_bit = 0;
	info.entries[0].op_size_32 = 1;
	info.entries[0].granularity = 1;

	// grew the internal ldt table to max
	err = (*set_information_process)(GetCurrentProcess(), 10, (void*)&info, sizeof(LdtInfo) );

	ldtTable[0].present = 1; // ignore the first entry as windows does
	for (auto index = 1; index < LDT_ENTRIES; ++index) {
		DWORD len;
		info.byte_offset = index << 3;
		/* TODO(sehr): change parameters to allow only one call to query_info... */
		err = query_information_process(GetCurrentProcess(), 10, (void*)&info, sizeof(LdtInfo), &len);
		memcpy( &ldtTable[index], info.entries, info.size );
	}
#	endif

#else
	pageSize = getpagesize();

#	if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32
	modify_ldt(0, ldtTable, sizeof( LdtEntry ) * LDT_ENTRIES );
#	endif

#endif
}

void MMU::allocPagesWithFlags( void** hintOut, size_t numBytes, unsigned int flags ) {
	*hintOut = (void*)( (uintptr_t)*hintOut & ~(pageSize-1) );
	numBytes = Core::alignTo( numBytes, pageSize );

#if PLATFORM_OS == LINUX 
	int mapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
	if( *hintOut != 0 ) {
		mapFlags |= MAP_FIXED;
	}
	auto addr = mmap( *hintOut, size, PROT_NONE,  flags, -1, (off_t) 0);
	if( addr == MAP_FAILED ){
		addr = nullptr;
	}
	*hintOut = addr;
#elif PLATFORM_OS == MS_WINDOWS
    auto addr = VirtualAlloc( *hintOut, numBytes, flags, PAGE_NOACCESS);
    *hintOut = addr;
#endif
}


void MMU::allocPages( void** hintOut, size_t numBytes ) {
#if PLATFORM_OS == LINUX 
	int mapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
	if( *hintOut != 0 ) {
		mapFlags |= MAP_FIXED;
	}
	allocPagesWithFlags( hintOut, numBytes, mapFlags );
#elif PLATFORM_OS == MS_WINDOWS
	allocPagesWithFlags( hintOut, numBytes, MEM_RESERVE | MEM_COMMIT );
#endif
}

void MMU::reservePages( void** hintOut, size_t numBytes ) {
#if PLATFORM_OS == LINUX 
	int mapFlags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
	if( *hintOut != 0 ) {
		mapFlags |= MAP_FIXED;
	}
	allocPagesWithFlags( hintOut, numBytes, mapFlags );
#elif PLATFORM_OS == MS_WINDOWS
	allocPagesWithFlags( hintOut, numBytes, MEM_RESERVE );
#endif
}

void MMU::commitPages( void** hintOut, size_t numBytes ) {
#if PLATFORM_OS == LINUX 
	int mapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
	if( *hintOut != 0 ) {
		mapFlags |= MAP_FIXED;
	}
	allocPagesWithFlags( hintOut, numBytes, mapFlags );
#elif PLATFORM_OS == MS_WINDOWS
	allocPagesWithFlags( hintOut, numBytes, MEM_COMMIT );
#endif
}


void MMU::freePages( void* pages, size_t numBytes ) {
	pages = (void*)( (uintptr_t)pages & ~(pageSize-1) );
#if PLATFORM_OS == LINUX
	numBytes = Core::alignTo( numBytes, pageSize );
	mmunmap( pages, numBytes );
#elif PLATFORM_OS == MS_WINDOWS
	VirtualFree( pages, 0, MEM_RELEASE );
#endif
}

void MMU::protectPages( void* pages, size_t numBytes, unsigned int settings ) {
	pages = (void*)( (uintptr_t)pages & ~(pageSize-1) );
	numBytes = Core::alignTo( numBytes, pageSize );
#if PLATFORM_OS == LINUX 
	mprotect( pages, numBytes, settings );
#elif PLATFORM_OS == MS_WINDOWS
	uint32_t winSettings = 0;
	if( settings & PROT_EXEC ) {
		if( settings & PROT_WRITE ) {
			// on windows write always means reads aswell
			winSettings = PAGE_EXECUTE_READWRITE;
		} else if( settings & PROT_READ ) {
			winSettings = PAGE_EXECUTE_READ;
		}
		if( winSettings == 0 ) {
			winSettings = PAGE_EXECUTE;
		}
	} else {
		if( settings & PROT_WRITE ) {
			// on windows write always means reads aswell
			winSettings = PAGE_READWRITE;
		} else if( settings & PROT_READ ) {
			winSettings = PAGE_READONLY;
		}
	}
	if( winSettings == 0 ) {
		winSettings = PAGE_NOACCESS;
	}
	DWORD old;
	VirtualProtect( pages, numBytes, winSettings, &old );
#endif
}

#if CPU_FAMILY == CPU_X86 && CPU_BIT_SIZE == 32

uint16_t MMU::allocSelector( bool data, void* pages, size_t numBytes, unsigned int settings ) {
	numBytes = Core::alignTo( numBytes, pageSize );
	uint16_t retval = LDT_ENTRIES + 1;
	// find a free entry.
	for (auto i = 0; i < LDT_ENTRIES; ++i) {
		if (!ldtTable[i].present) {
			retval = i;
			break;
		}
	}
	// if no free space found, error out
	if( retval == LDT_ENTRIES + 1 ){
		return 0xFFFF;
	}

	// our segments are always in pages not bytes
	auto limit = (numBytes / pageSize) - 1;
	assert( numBytes >= pageSize );	// min 1 page per segment
	assert( limit < 0xFFFFF ); 	// max 2^20 pages per segment

#if PLATFORM_OS == LINUX
	// linux requires us to use a user_desc ldt field to change selectors, this is a 
	// restricted form of the full ldt table, also its returns the full ldt entries
	struct user_desc ud;
	ud.entry_number = retval;
	if( data ) {
		ud.contents = MODIFY_LDT_CONTENTS_DATA;
	} else {
		ud.contents = MODIFY_LDT_CONTENTS_CODE;
	}
	ud.seg_32bit = 1;
	ud.seg_not_present = 0;
	ud.useable = 1;
	// user_dec only controls the write flag, so force the caller to set those flags
	// page protection is then used to refine them further
	assert( setting & (PROT_READ | PROT_EXEC) );
	if( setting & PROT_WRITE ) {
		ud.read_exec_only = false;
	} else {
		ud.read_exec_only = true;	
	}

	ud.limit = limit;
	ud.limit_in_pages = true;
	// send the ldt page into the hw MMU
  	modify_ldt( 1, &ud, sizeof(ud) );

  	// re-update the ldt table
	modify_ldt( 0, ldtTable, sizeof( LdtEntry ) * (retval+1) );

	// for changing the priviledge level (3 = 0x7) is encoded in the
	// bottom bits. TBH not really sure why/what here but its what nacl
	// does so erm. I'll do it too even though I don't support changing
	// exisiting selectors, so I think its not needed?
  	return (retval << 3) | 0x7;
#elif PLATFORM_OS == MS_WINDOWS
	struct LdtEntry ldt;

	if( settings & PROT_WRITE ) {
		ldt.type = 0x12; // R/W
	} else {
		if( data ) {
			ldt.type = 0x10; // R
		} else {
			ldt.type = 0x1a; // R/X
		}
	}
	ldt.descriptor_privilege = 3;
	ldt.present = 1;
	ldt.available = 1;
	ldt.code_64_bit = 0;
	ldt.op_size_32 = 1;

	ldt.base_00to15 = ((unsigned long) pages) & 0xffff;
	ldt.base_16to23 = (((unsigned long) pages) >> 16) & 0xff;
	ldt.base_24to31 = (((unsigned long) pages) >> 24) & 0xff;
	ldt.limit_00to15 = limit & 0xffff;
	ldt.limit_16to19 = (limit >> 16) & 0xf;
	ldt.granularity = true; // paegs

	uint32_t err;
	if (NULL != set_ldt_entries) {
		union {
			struct LdtEntry ldt;
			DWORD dwords[2];
		} u;
		u.ldt = ldt;
		err = (*set_ldt_entries)((retval << 3) | 0x7,
	                    u.dwords[0],
	                    u.dwords[1],
	                    0,
	                    0,
	                    0);
	}

	if ((NULL == set_ldt_entries) || (0 != retval)) {
		LdtInfo info;
		info.byte_offset = retval << 3;
		info.size = sizeof(struct LdtEntry);
		info.entries[0] = ldt;
		err = (*set_information_process)((HANDLE)-1, 10, (void*)&info, 16);
	}
	memcpy( &ldtTable[retval], &ldt, sizeof( LdtEntry ) );

	return (retval << 3) | 0x7;
#endif
}

void MMU::freeSelector( uint16_t selector ) {

#if PLATFORM_OS == LINUX
	struct user_desc ud;
	ud.entry_number = selector >> 3;
	ud.seg_not_present = 1;
	ud.base_addr = 0;
	ud.limit = 0;
	ud.limit_in_pages = 0;
	ud.read_exec_only = 0;
	ud.seg_32bit = 0;
	ud.useable = 0;
	ud.contents = MODIFY_LDT_CONTENTS_DATA;
	modify_ldt( 1, &ud, sizeof(ud) );
#elif PLATFORM_OS == MS_WINDOWS
	int retval;
	union {
		struct LdtEntry entry;
		DWORD dwords[2];
	} u;
	retval = 0;
	u.entry.base_00to15 = 0;
	u.entry.base_16to23 = 0;
	u.entry.base_24to31 = 0;
	u.entry.limit_00to15 = 0;
	u.entry.limit_16to19 = 0;
	u.entry.type = 0x10;
	u.entry.descriptor_privilege = 3;
	u.entry.present = 0;
	u.entry.available = 0;
	u.entry.code_64_bit = 0;
	u.entry.op_size_32 = 1;
	u.entry.granularity = 1;

	if (NULL != set_ldt_entries) {
		retval = (*set_ldt_entries)(selector, u.dwords[0], u.dwords[1], 0, 0, 0);
	}

	if ((NULL == set_ldt_entries) || (0 != retval)) {
		LdtInfo info;
		info.byte_offset = selector & ~0x7;
		info.size = sizeof(LdtEntry);
		info.entries[0] = u.entry;
		retval = (*set_information_process)((HANDLE)-1, 10, (void*)&info, 16);
	}
	memcpy( &ldtTable[(selector >> 3)], &u.entry, sizeof( LdtEntry ) );

#endif

}

#endif
