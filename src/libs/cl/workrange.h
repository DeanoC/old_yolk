//!-----------------------------------------------------
//!
//! \file workrange.h
//! Contains the cl NDRange helper object
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_WORKRANGE_H)
#define WIERD_CL_WORKRANGE_H

namespace Cl {
	template< int DIMS >
	struct WorkRange {
		WorkRange( size_t globRange[DIMS] ) {
			for( int i = 0;i < DIMS; ++i ) {
				store[i] = globRange[i];
			}
			globalRange = &store[0];
			localRange = nullptr;
			globalOffset = nullptr;
		}
		WorkRange( size_t globRange[DIMS], size_t locRange[DIMS], size_t globOff[DIMS] ) {
			for( int i = 0;i < DIMS; ++i ) {
				store[i] = globRange[i];
				store[i+DIMS] = locRange[i];
				store[i+(DIMS*2)] = globOff[i];
			}
			globalRange = &store[0];
			localRange = &store[DIMS];
			globalOffset = &store[2*DIMS];
		}
		WorkRange( size_t globRange[DIMS], size_t locRange[DIMS] ) {
			for( int i = 0;i < DIMS; ++i ) {
				store[i] = globRange[i];
				store[i+DIMS] = locRange[i];
			}
			globalRange = &store[0];
			localRange = &store[DIMS];
			globalOffset = nullptr;
		}

		size_t* globalRange;
		size_t* localRange;
		size_t* globalOffset;
		size_t	store[ DIMS * 3];
	};
	template<>
	struct WorkRange<1> {
		WorkRange( size_t globRange ) {
			store[0] = globRange;
			globalRange = &store[0];
			localRange = nullptr;
			globalOffset = nullptr;
		}

		WorkRange( size_t globRange, size_t locRange, size_t globOff ) {
			store[0] = globRange;
			store[1] = locRange;
			store[2] = globOff;
			globalRange = &store[0];
			localRange = &store[1];
			globalOffset = &store[2];
		}
		size_t* globalRange;
		size_t* localRange;
		size_t* globalOffset;
		size_t	store[3];
	};
}

#endif // WIERD_CL_WORKRANGE_H