#ifndef YOLK_VTDFH_INDIGO_WEAK_SPIDER_H_
#define YOLK_VTDFH_INDIGO_WEAK_SPIDER_H_ 1

#include "localworld/updatable.h"

class IndigoWeakSpider {
public:
	IndigoWeakSpider( SceneWorldPtr _world, Core::TransformNode* startNode );
	~IndigoWeakSpider();
	
protected:
	void update( float timeS );

	Updatable				updater;

	SceneWorldPtr 	world;
	ThingPtr  		spider;

	float speed;
	float angularSpeed;
};

typedef std::shared_ptr<IndigoWeakSpider> IndigoWeakSpiderPtr;
#endif