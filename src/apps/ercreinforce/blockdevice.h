#ifndef ERCREINFORCE_BLOCKDEVICE_H_
#define ERCREINFORCE_BLOCKDEVICE_H_ 1
/**
 * ...
 * @author Deano Calver
 */

class BlockDevice {
	
	virtual int getBlockCount() = 0;
	virtual int getBlockSize() = 0;
	
	// single block operations
	// async read and write
	virtual const uint8_t* readBlock( int blockaddr ) const = 0;
	virtual void writeBlock( int blockaddr, const uint8_t* data ) = 0;

	// mark this block isn't used (aka TRIM)
	virtual void unuseBlock( int blockAddr ) = 0;
	
};

#endif