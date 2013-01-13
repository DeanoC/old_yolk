#ifndef ERCREINFORCE_BASE_STORAGEDEVICE_H_
#define ERCREINFORCE_BASE_STORAGEDEVICE_H_ 1

#include "storagedevice.h"

class StorageDeviceBase : public StorageDevice {
public:
	virtual int getSize() override { return size; }
	virtual int getUniqueAddress() override { return uniqueAddress; }
	virtual Usage getUsage() override { return usage; }
	virtual State getState() override { return state; }
	
	StorageDeviceBase( int _size ) {
		size = _size;
		uniqueAddress = usedUAs++;
		usage = Usage::Spare;
		state = State::Online;
	}
	
protected:
	int size;
	int uniqueAddress;
	Usage usage;
	State state;
	
	static int usedUAs;
};

#endif
