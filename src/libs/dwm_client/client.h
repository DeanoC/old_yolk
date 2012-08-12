#ifndef YOLK_DWM_CLIENT_CLIENT_H_
#define YOLK_DWM_CLIENT_CLIENT_H_ 1

#include "dwm_client/clientworld.h"

class DwmClient {
public:
	DwmClient();
	~DwmClient();

	void start();
	void run();
	void end();

	ClientWorldPtr getClientWorld() const { return world; }
protected:
	ClientWorldPtr						world;
};


#endif