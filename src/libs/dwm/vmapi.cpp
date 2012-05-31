#include "dwm.h"

#include "mmu.h"
#include "sandboxmemorymanager.h"
#include "trustedregion.h"
#include "ieethreadcontext.h"
#include "isolatedexecengine.h"

// TODO debug versus release(SFI) type magic

#define UNTRUSTED_UINTPTR_T uint64_t
#define TRUSTED_PTR_TO_UNTRUSTED(x) (UNTRUSTED_UINTPTR_T)(x - threadCtx->membase)
#define UNTRUSTED_PTR_TO_TRUSTED(x) ((uintptr_t)x + threadCtx->membase)
//namespace {

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

__thread UNTRUSTED_UINTPTR_T tlsStorage;

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
	// stdout
	if( fd == STDOUT_FILENO ) {
		struct stat sds;
		fstat( STDOUT_FILENO, &sds );
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
	} else {
		LOG(INFO) << "TODO: VmFStat " << fd << " \n";
	}
	return 0;
}

uint32_t VmWrite( const IEEThreadContext* threadCtx, int fd, UNTRUSTED_UINTPTR_T unBuf, uint32_t num ) {
	char* buf = (char*) UNTRUSTED_PTR_TO_TRUSTED( unBuf );

	if( fd == STDOUT_FILENO ) {
		write( fd, buf, num );
	} else {
		LOG(INFO) << "TODO: VmWrite : ";
		for( uint32_t i = 0; i < num; ++i ) {
			LOG(INFO) << buf[i];
		}
		LOG(INFO) << "\n";
	}
	return num;
}




//} // end anon namespace

void InstallVmApiFuncs( TrustedRegion* trustedRegion ) {
	trustedRegion->addFunctionTrampoline( "VmAlloc", (void*) VmAlloc );
	trustedRegion->addFunctionTrampoline( "VmFree", (void*) VmFree );
	trustedRegion->addFunctionTrampoline( "__get_sp", (void*) VmGetSp );
	trustedRegion->addFunctionTrampoline( "gettid", (void*) VmGetTid );
	trustedRegion->addFunctionTrampoline( "VmMMap", (void*) VmMMap );
	trustedRegion->addFunctionTrampoline( "mprotect", (void*) VmMProtect );

	trustedRegion->addFunctionTrampoline( "write", (void*) VmWrite );
	trustedRegion->addFunctionTrampoline( "fstat", (void*) VmFStat );

	trustedRegion->addFunctionTrampoline( "__set_tls", (void*) VmSetTls );
	trustedRegion->addFunctionTrampoline( "__get_tls", (void*) VmGetTls );
}
