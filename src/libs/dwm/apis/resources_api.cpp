#include "dwm/dwm.h"
#include "dwm/trustedregion.h"
#include "dwm/ieethreadcontext.h"
#include "dwm/isolatedexecengine.h"
#include "dwm/trustedregion.h"
#include "dwm/world.h"

#include "core/resourceman.h"
#include "core/coreresources.h"

#include "apis.h"
#include "../../../vt/src/libs/area/apis/resources_api.h"

ResHandle ResOpenText( _VT_PARAMS1( _VT_PTR( const char*, name ) ) ) {
	using namespace Core;
	const char* lfname = (const char*)UNTRUSTED_PTR_TO_TRUSTED( name );

	Core::TextResourceHandlePtr res = Core::TextResourceHandle::load( lfname, 0, RMRF_DONTFLUSH );
	uint32_t handle = threadCtx->owner->getTrustedRegion()->trustedAddressToHandle( (uintptr_t) res );
	return handle;	
}

ResHandle ResOpenBinProperty( _VT_PARAMS1( _VT_PTR( const char*, name ) ) ) {
	using namespace Core;
	const char* lfname = (const char*)UNTRUSTED_PTR_TO_TRUSTED( name );

	Core::BinPropertyResourceHandlePtr res = Core::BinPropertyResourceHandle::load( lfname, 0, RMRF_DONTFLUSH );
	uint32_t handle = threadCtx->owner->getTrustedRegion()->trustedAddressToHandle( (uintptr_t) res );
	return handle;	
}

uint32_t ResClose( _VT_PARAMS1( ResHandle handle ) ) {
	using namespace Core;
	ResourceHandleBase* res = (Core::ResourceHandleBase*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( handle );
	if( res == nullptr ) {
		return RES_ERROR;
	}
	ResourceMan::get()->baseCloseResource( res );
	return RES_OK;
}


uint32_t ResSize( _VT_PARAMS1( ResHandle handle ) ) {
	using namespace Core;
	uint32_t size = ~0;
	ResourceHandleBase* res = (Core::ResourceHandleBase*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( handle );
	switch( res->getType() ) {
		case TextType: {
			TextResourceHandlePtr tres = (TextResourceHandlePtr) res;
			auto textr = tres->acquire(); // dont flush flag ensure size can't change until we release our resource handle
			size = strlen( textr->text.get() );
		} 	break;
		case BinPropertyType: {
			BinPropertyResourceHandlePtr bres = (BinPropertyResourceHandlePtr) res;
			auto bpr = bres->acquire(); // dont flush flag ensure size can't change until we release our resource handle
			size = bpr->totalSize;
		}	break;
		default:
			LOG(INFO) << "Invalid resource type\n";
			return RES_ERROR;
	}
	return size;
}

struct VtBinProperty {
	uint8_t				flagsAndType;
	uint8_t					count;
	UNTRUSTED_UINTPTR_T 	name;
	UNTRUSTED_UINTPTR_T		data;
};



uint32_t ResCopyTo( _VT_PARAMS2( ResHandle handle, _VT_PTR( char*, vtDest ) ) ) {
	using namespace Core;
	uint32_t size = ResSize( threadCtx, handle );
	if( size == ~0 ) {
		LOG(INFO) << "Invalid resource size\n";
		return RES_ERROR;
	}
	uint8_t* dest = (uint8_t*) UNTRUSTED_PTR_TO_TRUSTED(vtDest);

	bool memChk = true;
	// check buffer has size 0's (has been memset'ed)
	for( uint32_t i = 0;i < size; ++i ) {
		if( dest[i] != 0 ) {
			memChk = false;
			break;
		}
	}
	if( memChk == false ) {
		LOG(INFO) << "Memory was not clear before called to ResCopyTo\n";
		return RES_ERROR;
	}

	ResourceHandleBase* res = (Core::ResourceHandleBase*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( handle );
	switch( res->getType() ) {
		case TextType: {
			TextResourceHandlePtr tres = (TextResourceHandlePtr) res;
			auto textr = tres->acquire(); // dont flush flag ensure size can't change until we release our resource handle
			memcpy( dest, textr->text.get(), size );
		} 	break;
		case BinPropertyType: {
			BinPropertyResourceHandlePtr bres = (BinPropertyResourceHandlePtr) res;
			auto bpr = bres->acquire(); // dont flush flag ensure size can't change until we release our resource handle
			auto trustedBPropBase = (uint8_t*)bpr->entries.get();
			memcpy( dest, trustedBPropBase, bpr->totalSize );
			VtBinProperty* vtBin = (VtBinProperty*) dest;
			// can't just memcopy as we have to fixup pointer + potentially accoutn for size change
			for( int i = 0;i < bpr->numEntries; ++i ) {
				vtBin[i].flagsAndType = bpr->entries[i].flagsAndType;
				vtBin[i].count = bpr->entries[i].count;
				auto nameOffset = ((uint8_t*)bpr->entries[i].name.p - trustedBPropBase);
				auto dataOffset = ((uint8_t*)bpr->entries[i].data.p - trustedBPropBase);
				vtBin[i].name = TRUSTED_PTR_TO_UNTRUSTED( nameOffset + dest );
				vtBin[i].data = TRUSTED_PTR_TO_UNTRUSTED( dataOffset + dest );
			}
		} 	break;
		default:
			LOG(INFO) << "Invalid resource type\n";
			return RES_ERROR;
	}

	return RES_OK;
}
uint32_t ResNumEntries( _VT_PARAMS1( ResHandle handle ) ) {
	using namespace Core;

	ResourceHandleBase* res = (Core::ResourceHandleBase*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( handle );
	switch( res->getType() ) {
		case TextType:
			return 1;
		case BinPropertyType: {
			BinPropertyResourceHandlePtr bres = (BinPropertyResourceHandlePtr) res;
			auto bpr = bres->acquire(); // dont flush flag ensure size can't change until we release our resource handle
			return bpr->numEntries;
		}
		default:
			LOG(INFO) << "Invalid resource type\n";
			return RES_ERROR;
	}	

	return RES_OK;
}

void InstallResourcesApiFuncs( TrustedRegion* trustedRegion ) {
	trustedRegion->addFunctionTrampoline( "ResOpenText", (void*) ResOpenText );
	trustedRegion->addFunctionTrampoline( "ResOpenBinProperty", (void*) ResOpenBinProperty );

	trustedRegion->addFunctionTrampoline( "ResClose", (void*) ResClose );

	trustedRegion->addFunctionTrampoline( "ResSize", (void*) ResSize );
	trustedRegion->addFunctionTrampoline( "ResCopyTo", (void*) ResCopyTo );
	trustedRegion->addFunctionTrampoline( "ResNumEntries", (void*) ResNumEntries );


}