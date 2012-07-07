//!-----------------------------------------------------
//!
//! \file cmdqueue.h
//! Contains the cl base class for compute memory
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_CMDQUEUE_H)
#define WIERD_CL_CMDQUEUE_H

#include "buffer.h"
#include "image.h"
#include "kernel.h"
#include "workrange.h"

namespace Cl {
	typedef cl_event Event;

	class CmdQueue {
	public:
		// entire buffer manipulation
		void read( const BufferPtr src, void* dest, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			cl_int _err;
			_err = clEnqueueReadBuffer( q, src->getName(), false, 0, src->getSize(), dest, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueReadBuffer", _err );
		}
		void write( const void* src, BufferPtr dest, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			cl_int _err;
			_err = clEnqueueWriteBuffer( q, dest->getName(), false, 0, dest->getSize(), src, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueWriteBuffer", _err );
		}
		void copy( const BufferPtr src, BufferPtr dest, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			CORE_ASSERT( dest->getSize() >= src->getSize() );
			cl_int _err;
			_err = clEnqueueCopyBuffer( q, src->getName(), dest->getName(), 0, 0, src->getSize(), numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueCopyBuffer", _err );
		}
		// entire image manipulation
		void read( const ImagePtr src, void* dest, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			size_t origin[3] = { 0 };
			size_t size[3] = { src->getWidth(), src->getHeight(), 1 };
			cl_int _err;
			_err = clEnqueueReadImage( q, src->getName(), false, origin, size, 0, 0, dest, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueReadImage", _err );
		}
		void write( const void* src, const ImagePtr dest, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			size_t origin[3] = { 0 };
			size_t size[3] = { dest->getWidth(), dest->getHeight(), 1 };
			cl_int _err;
			_err = clEnqueueWriteImage( q, dest->getName(), false, origin, size, 0, 0, src, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueWriteImage", _err );
		}
		void copy( const ImagePtr src, ImagePtr dest, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			CORE_ASSERT( dest->getWidth() >= src->getWidth() );
			CORE_ASSERT( dest->getHeight() >= src->getHeight() );
			size_t origin[3] = { 0 };
			size_t size[3] = { src->getWidth(), src->getHeight(), 1 };
			cl_int _err;
			_err = clEnqueueCopyImage( q, src->getName(), dest->getName(), origin, origin, size, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueCopyImage", _err );
		}
		void copy( const ImagePtr src, BufferPtr dest, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			size_t origin[3] = { 0 };
			size_t size[3] = { src->getWidth(), src->getHeight(), 1 };
			cl_int _err;
			_err = clEnqueueCopyImageToBuffer( q, src->getName(), dest->getName(), origin, size, 0, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueCopyImageToBuffer", _err );
		}
		void copy( const BufferPtr src, ImagePtr dest, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			size_t origin[3] = { 0 };
			size_t size[3] = { dest->getWidth(), dest->getHeight(), 1 };
			cl_int _err;
			_err = clEnqueueCopyBufferToImage( q, src->getName(), dest->getName(), 0, origin, size, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueCopyImage", _err );
		}

		void executeTask( KernelPtr kernel, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			cl_int _err;
			_err = clEnqueueTask( q, kernel->getName(), numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueTask", _err );
		}
	
		template<int DIMS>
		void execute( KernelPtr kernel, const WorkRange<DIMS>& range, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			cl_int _err;
			_err = clEnqueueNDRangeKernel( q, kernel->getName(), DIMS, range.globalOffset, range.globalRange, range.localRange, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueNDRangeKernel", _err );
		}

		void acquireGL( MemoryObject::Name objName, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			cl_int _err;
			_err = clEnqueueAcquireGLObjects( q, 1, &objName, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueAcquireGLObjects", _err );
		}
		void releaseGL( MemoryObject::Name objName, Event* eve = nullptr, int numToWait = 0, const Event* waitEvents = nullptr ) {
			cl_int _err;
			_err = clEnqueueReleaseGLObjects( q, 1, &objName, numToWait, waitEvents, eve );
			CL_CHECK("clEnqueueReleaseGLObjects", _err );
		}


		void flush() {
			cl_int _err;
			_err = clFlush( q );
			CL_CHECK("clFlush", _err );
		}

		void finish() {
			cl_int _err;
			_err = clFinish( q );
			CL_CHECK("clFinish", _err );
		}

		void marker( Event* eve = nullptr ) {
			cl_int _err;
			_err = clEnqueueMarker( q, eve );
			CL_CHECK("clEnqueueMarker", _err );
		}

		void barrier( Event* eve = nullptr ) {
			cl_int _err;
			_err = clEnqueueBarrier( q );
			CL_CHECK("clEnqueueBarrier", _err );
		}

		void waitForEvents( int numToWait, const Event* waitEvents ) {
			cl_int _err;
			_err = clEnqueueWaitForEvents( q, numToWait, waitEvents );
			CL_CHECK("clEnqueueWaitForEvents", _err );
		}

	protected:
		friend class Context;
		CmdQueue( cl_context _con, cl_device_id _dev ) {
			cl_int _err;
			q = clCreateCommandQueue( _con, _dev, 0, &_err );
			CL_CHECK("clCreateCommandQueue", _err );
		}

		cl_command_queue	q;
	};

	typedef std::shared_ptr<CmdQueue>	CmdQueuePtr;
}

#endif //WIERD_CL_CMDQUEUE_H
