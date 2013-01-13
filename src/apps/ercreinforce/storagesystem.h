#ifndef ERCREINFORCE_STORAGESYSTEM_H_
#define ERCREINFORCE_STORAGESYSTEM_H_ 1

class StorageDevice;

class StorageSystem {
public:
	virtual int getDeviceCount() = 0;
	virtual std::shared_ptr<StorageDevice> getDevice( int _index ) = 0;

	virtual void replaceDevice( int _index, std::shared_ptr<StorageDevice>& _newdevice ) = 0;
	
};

#endif