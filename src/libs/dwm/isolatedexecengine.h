/**
 @file	Z:\Projects\Cyberspace\yolk_repo\src\libs\dwm\isolatedexecengine.h

 Declares the isolatedexecengine class.
 */
 
#ifndef DWM_ISOLATEDEXECENGINE_H_
#define DWM_ISOLATEDEXECENGINE_H_

class SandboxMemoryManager;
class TrustedRegion;
class World;
namespace llvm { class RuntimeDyld; }

class IsolatedExecEngine {
public:
	static const unsigned int KiB = 1024;
	static const unsigned int MiB = 1024 * KiB;
	static const unsigned int GiB = 1024 * MiB;

	IsolatedExecEngine( uint32_t sandboxSize, uint32_t sandboxStackSize, uint32_t sandboxTrustedRegionSize, World* _world );
	~IsolatedExecEngine();

	void addLibrary( const std::string& elfstr );
	void process( const std::string& elfstr, void (*apiInstall)( TrustedRegion* ) );

	void* sandboxAllocate( size_t size );
	void sandboxFree( void* ptr );

	TrustedRegion* getTrustedRegion() { return trustedRegion; }

	void terminate();

	World*					world;
private:
	Core::thread*			untrustedThread;

	void* sandboxMem;
	void* reservedMem;
	SandboxMemoryManager*	mmgr;
	llvm::RuntimeDyld*		dyld;
	TrustedRegion*			trustedRegion;
};

#endif