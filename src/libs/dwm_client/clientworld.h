#ifndef _YOLK_DWM_CLIENT_CLIENT_WORLD_H_
#define _YOLK_DWM_CLIENT_CLIENT_WORLD_H_ 1

#include "dwm/world.h"
#include "dwm_client/inputframe.h"
#include "core/boundedbuffer.h"

namespace Scene {
	class Camera;
}

class ClientWorld : public World {
public:
	ClientWorld();

	virtual ApiSetupFunction getApiSetupFunction() override { return &InstallApiFuncs; } 

	void queueInputFrame( const InputFrame& frame );
	bool dequeueInputFrame( InputFrame* frame ); 	//!< return false if no input to dequeue

	static void InstallApiFuncs( class TrustedRegion* trustedRegion );
protected:
	Core::BoundedBuffer<InputFrame>		inputQueue;
};

typedef std::shared_ptr<ClientWorld> ClientWorldPtr;

#endif