#ifndef YOLK_TESTLVL_H_
#define YOLK_TESTLVL_H_ 1

class TestLvl : public Updatable {
public:
	TestLvl( SceneWorldPtr _world );
	~TestLvl();
	
	virtual void update( float timeS ) override;
private:
	SceneWorldPtr 	world;
	ThingPtr  		land;
};

#endif