#include "dwm/dwm.h"
#include "core/clock.h"
#include "dwm/mmu.h"
#include "dwm/sandboxmemorymanager.h"
#include "dwm/trustedregion.h"
#include "dwm/ieethreadcontext.h"
#include "dwm/isolatedexecengine.h"
#include "apis.h"

void VmExit( const IEEThreadContext* threadCtx ) {
	threadCtx->owner->terminate();
	// never return
	while( true ) {
		Core::Clock::sleep( 0.001f );
	}
}
void Vm_exit( const IEEThreadContext* threadCtx ) {
	threadCtx->owner->terminate();
	LOG(INFO) << "**************_exit called from DWM *************\n";
	// never return
	while( true ) {
		Core::Clock::sleep( 0.001f );
	}
}

UNTRUSTED_UINTPTR_T VmAlloc( const IEEThreadContext* threadCtx, uint32_t size ) {
	uintptr_t tAddr = (uintptr_t) threadCtx->owner->sandboxAllocate( size );
	return TRUSTED_PTR_TO_UNTRUSTED( tAddr );
}
void VmFree( const IEEThreadContext* threadCtx, UNTRUSTED_UINTPTR_T unAddr ) {
	threadCtx->owner->sandboxFree( (void*)UNTRUSTED_PTR_TO_TRUSTED(unAddr) );	
}

UNTRUSTED_UINTPTR_T VmGetSp( const IEEThreadContext* threadCtx ) {
	return TRUSTED_PTR_TO_UNTRUSTED(threadCtx->untrusted_stack);
}

uint32_t VmGetTid( const IEEThreadContext* threadCtx) {
	LOG(INFO) << "TODO: VmGetTid\n";
	return 0;
}

struct MMapParams {
	int32_t size;
	int32_t flags;
	int32_t fd;
	int32_t page_offset;
};

UNTRUSTED_UINTPTR_T VmMMap( const IEEThreadContext* threadCtx, UNTRUSTED_UINTPTR_T unParamPtr ) {
	const MMapParams* params = (const MMapParams*) UNTRUSTED_PTR_TO_TRUSTED( unParamPtr );
	return VmAlloc( threadCtx, params->size );
}

int VmMProtect( const IEEThreadContext* threadCtx, UNTRUSTED_UINTPTR_T addr, size_t len, int prot ) {
	LOG(INFO) << "TODO: VmMProtect\n";
	return 0;
}

#if PLATFORM == WINDOWS
__declspec( thread ) UNTRUSTED_UINTPTR_T tlsStorage;
#else
__thread UNTRUSTED_UINTPTR_T tlsStorage;
#endif

UNTRUSTED_UINTPTR_T VmGetTls() {
	return tlsStorage;
}

void VmSetTls( UNTRUSTED_UINTPTR_T val ) {
	tlsStorage = val;	
}

struct VmStat {
    uint64_t        dev;

    uint64_t        ino;
    uint32_t        mode;
    uint32_t        nlink;

    uint64_t        uid;
    uint64_t        gid;

    uint64_t        rdev;

    int64_t         size;
    uint64_t        blksize;
    uint64_t        blocks;

    uint64_t        atime;
    uint64_t        atime_nsec;

    uint64_t        mtime;
    uint64_t        mtime_nsec;

    uint64_t        ctime;
    uint64_t        ctime_nsec;

    uint64_t        ino2;
};

int VmFStat( const IEEThreadContext* threadCtx, int fd, UNTRUSTED_UINTPTR_T unBuf ) {
	VmStat* stat = (VmStat*) UNTRUSTED_PTR_TO_TRUSTED( unBuf );
	memset( stat, 0, sizeof( VmStat ) );

#if PLATFORM != WINDOWS
	// stdout
	if( fd == STDOUT_FILENO || fd == STDIN_FILENO || fd == STDERR_FILENO ) {
		struct stat sds;
		fstat( fd, &sds );
		stat->dev = sds.st_dev;
		stat->ino = sds.st_ino;
		stat->mode = sds.st_mode;
		stat->nlink = sds.st_nlink;
		stat->uid = sds.st_uid;
		stat->gid = sds.st_gid;
		stat->rdev = sds.st_rdev;
		stat->size = sds.st_size;
		stat->blksize = sds.st_blksize;
		stat->blocks = sds.st_blocks;
		stat->ino2 = sds.st_ino;
	} else 
#endif	
	{
		LOG(INFO) << "TODO: VmFStat " << fd << " \n";
	}
	return 0;
}

uint32_t VmWrite( const IEEThreadContext* threadCtx, int fd, UNTRUSTED_UINTPTR_T unBuf, uint32_t num ) {
	char* buf = (char*) UNTRUSTED_PTR_TO_TRUSTED( unBuf );

#if PLATFORM != WINDOWS
	if( fd == STDOUT_FILENO || fd == STDERR_FILENO ) {
		write( fd, buf, num );
	} else 
#endif
	{
		LOG(INFO) << "TODO: VmWrite : ";
		for( uint32_t i = 0; i < num; ++i ) {
			LOG(INFO) << buf[i];
		}
		LOG(INFO) << "\n";
	}
	return num;
}

struct VmTimeVal {
	int64_t      	tv_sec;
	uint64_t 		tv_usec;
};

int VmGetTimeOfDay( const IEEThreadContext* threadCtx, UNTRUSTED_UINTPTR_T unTv ) {
	struct timeval tv;
	struct VmTimeVal *rtv = (VmTimeVal*) UNTRUSTED_PTR_TO_TRUSTED(unTv);

#if PLATFORM == WINDOWS
	TODO_ASSERT( false );
#else
	int r = gettimeofday( &tv, nullptr );
	rtv->tv_sec = tv.tv_sec;
	rtv->tv_usec = tv.tv_usec;

	return r;
#endif
}
struct VmTimeSpec {
  int64_t  tv_sec;
  long    tv_nsec;
};

void VmNanoSleep( const IEEThreadContext* threadCtx, UNTRUSTED_UINTPTR_T unTs ) {
	struct VmTimeSpec *rts = (VmTimeSpec*) UNTRUSTED_PTR_TO_TRUSTED(unTs);

#if PLATFORM == WINDOWS
	Core::Clock::sleep( (float)rts->tv_sec + (rts->tv_nsec * 1e-9f) ); 
#else
	struct timespec ts;
	ts.tv_sec = rts->tv_sec;
	ts.tv_nsec = rts->tv_nsec;
	nanosleep( &ts, nullptr );
#endif
}
void InstallVmApiFuncs( TrustedRegion* trustedRegion ) {

	trustedRegion->addFunctionTrampoline( "VmExit", (void*) VmExit );
	trustedRegion->addFunctionTrampoline( "_exit", (void*) Vm_exit );
	trustedRegion->addFunctionTrampoline( "VmAlloc", (void*) VmAlloc );
	trustedRegion->addFunctionTrampoline( "VmFree", (void*) VmFree );
	trustedRegion->addFunctionTrampoline( "__get_sp", (void*) VmGetSp );
	trustedRegion->addFunctionTrampoline( "gettid", (void*) VmGetTid );
	trustedRegion->addFunctionTrampoline( "VmMMap", (void*) VmMMap );
	trustedRegion->addFunctionTrampoline( "mprotect", (void*) VmMProtect );

	trustedRegion->addFunctionTrampoline( "write", (void*) VmWrite );
	trustedRegion->addFunctionTrampoline( "fstat", (void*) VmFStat );
	trustedRegion->addFunctionTrampoline( "gettimeofday", (void*) VmGetTimeOfDay );
	trustedRegion->addFunctionTrampoline( "nanosleep", (void*) VmNanoSleep );

	trustedRegion->addFunctionTrampoline( "__set_tls", (void*) VmSetTls );
	trustedRegion->addFunctionTrampoline( "__get_tls", (void*) VmGetTls );
}
