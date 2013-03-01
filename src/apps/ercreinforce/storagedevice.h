#ifndef ERCREFINFORCE_STORAGEDEVICE_H_
#define ERCREFINFORCE_STORAGEDEVICE_H_ 1

#include <vector>

enum class Usage {
	Data,
	Redundency,
	Spare,
};

enum class State {
	Online,
	Offline,
	Faulty,
	Rebuilding,
};

typedef std::vector<int> StorageGroup;

struct NGrp {
	std::vector<StorageGroup> 		groups;
	std::vector<int> 				set;
};

class StorageDevice {
public:
	virtual int getSize() = 0;
	virtual int getUniqueAddress() = 0;
	virtual Usage getUsage() = 0;
	virtual State getState() = 0;
};

#endif