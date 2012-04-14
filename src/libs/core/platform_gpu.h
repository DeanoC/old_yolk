
#if COMPILER == CUDA_COMPILER && PLATFORM == GPU
#undef assert
#define CORE_ASSERT(x) if(!(x))printf(#x);
#define assert(x) CORE_ASSERT(x)
#endif

// these allow sharing between cpu and gpu
#if COMPILER == CUDA_COMPILER
#	define CALL				__device__
#	define READONLY_MEM		__constant__
#	define LOCAL_MEM		__device__
#endif

#	define CORE_ALIGN(x)  __align__(x)

