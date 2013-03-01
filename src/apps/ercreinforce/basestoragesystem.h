#ifndef ERCREINFORCE_BASESTORAGESYSTEM_H_
#define ERCREINFORCE_BASESTORAGESYSTEM_H_ 1

#include <memory>
#include <vector>
#include "storagesystem.h"
#include "storagedevice.h"

class BaseStorageSystem : public StorageSystem {
public:
	virtual int getDeviceCount() override { return devices.size(); }

	virtual std::shared_ptr<StorageDevice> getDevice( int _index ) override { return devices.at( _index ); }

	virtual void replaceDevice( int _index, std::shared_ptr<StorageDevice>& _newdevice ) override {
		devices.at(_index) = _newdevice;
	}

	virtual void pushDevice( std::shared_ptr<StorageDevice>& _newdevice ) {
		devices.push_back( _newdevice );
	}

protected:
	std::vector< std::shared_ptr<StorageDevice> > devices;

};

#endif