#ifndef ERCREINFORCE_MEMORYBLOCKSTORAGEDEVICE_H_
#define ERCREINFORCE_MEMORYBLOCKSTORAGEDEVICE_H_ 1

#include "basestoragedevice.h"
#include "blockdevice.h"
/**
 * ...
 * @author Deano Calver
 */

class MemoryStorageDevice : public StorageDeviceBase, public BlockDevice {
public:
	
	MemoryStorageDevice( int _size, int _blockSize ) :
		StorageDeviceBase( _size ) {
		buffer = (uint8_t*) malloc( size );
		blockSize = _blockSize;
		blockCount = size / blockSize; // remainder will be unused and wasted
	}
	~MemoryStorageDevice() {
		free( buffer );
	}
	uint8_t* getBuffer() { return buffer; }

	// implement BlockDevice interfce
	virtual int getBlockCount() override { return blockCount; }
	virtual int getBlockSize() override { return blockSize; }

	const uint8_t* readBlock( int blockAddr ) const override {
		return buffer + (blockAddr * blockSize);
	}

	void unuseBlock( int blockAddr ) override {
		std::cerr << "TODO unuseBlock\n";
	}

	void writeBlock( int blockaddr, const uint8_t* data ) override {
		memcpy( buffer + (blockaddr * blockSize), data , blockSize );
	}
	
private:
	uint8_t*	buffer;
	int			blockSize;
	int			blockCount;
};

#endif