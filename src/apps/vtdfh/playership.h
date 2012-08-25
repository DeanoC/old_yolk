#ifndef YOLK_VTDFH_PLAYERSHIP_H_
#define YOLK_VTDFH_PLAYERSHIP_H_ 1

class PlayerShip {
public:
	PlayerShip( SceneWorldPtr _world );
	~PlayerShip();
private:
	SceneWorldPtr 	world;
	ThingPtr  		ship;
};

#endif