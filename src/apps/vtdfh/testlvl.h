#ifndef YOLK_TESTLVL_H_
#define YOLK_TESTLVL_H_ 1

class TestLvl {
public:
	TestLvl( SceneWorldPtr _world );
	~TestLvl();
private:
	SceneWorldPtr 	world;
	ThingPtr  		land;
};

#endif