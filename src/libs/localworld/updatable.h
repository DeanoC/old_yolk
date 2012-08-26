#ifndef YOLK_LOCALWORLD_UPDATABLE_H_
#define YOLK_LOCALWORLD_UPDATABLE_H_ 1

class Updatable {
public:
	virtual void update( float timeS ) = 0;
};

typedef std::shared_ptr<Updatable> UpdatablePtr;
#endif