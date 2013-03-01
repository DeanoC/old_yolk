#ifndef YOLK_VTDFH_ENERGY_CRYSTAL_H_
#define YOLK_VTDFH_ENERGY_CRYSTAL_H_ 1

class EnergyCrystal {
public:
	EnergyCrystal( SceneWorldPtr _world, Core::TransformNode* startNode );
	~EnergyCrystal();
	
protected:
	SceneWorldPtr 	world;
	ThingPtr  		crystal;
};

typedef std::shared_ptr<EnergyCrystal> EnergyCrystalPtr;

#endif