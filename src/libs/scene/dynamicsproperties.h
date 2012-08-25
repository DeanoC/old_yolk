#ifndef YOLK_SCENE_DYNAMICS_PROPERTIES_H_
#define YOLK_SCENE_DYNAMICS_PROPERTIES_H_ 1

#define DYNAMICS_TYPE 				"dynamics_type"
#define DYNAMICS_SHAPE 				"dynamics_shape"
#define DYNAMICS_COLMARGIN 			"dynamics_colmargin"
#define DYNAMICS_MASSMETHOD 		"dynamics_massmethod"
#define DYNAMICS_MASS 				"dynamics_mass"
#define DYNAMICS_DENSITY 			"dynamics_density"
#define DYNAMICS_FRICTION 			"dynamics_friction"
#define DYNAMICS_RESTITUTION 		"dynamics_restitution"
#define DYNAMICS_LINEARDAMPING 		"dynamics_lineardamping"
#define DYNAMICS_ANGULARDAMPING 	"dynamics_angulardamping"

#define DYNAMICS_TYPE_STATIC		0
#define DYNAMICS_TYPE_DYNAMICS		1

#define DYNAMICS_SHAPE_SPHERE		0
#define DYNAMICS_SHAPE_BOX			1
#define DYNAMICS_SHAPE_CONVEXHULL	2
#define DYNAMICS_SHAPE_MESH			3

#define DYNAMICS_SPHERE_RADIUS		"dynamics_sphere_radius"
#define DYNAMICS_BOX_EXTENTS		"dynamics_box_extents"
// convex hull TODO
#define DYNAMICS_MAX_MESH_VERTICES	(65535)
#define DYNAMICS_MAX_MESH_INDICES	(65535)
#define DYNAMICS_MESH_VERTICES		"dynamics_mesh_vertices"
#define DYNAMICS_MESH_INDICES		"dynamics_mesh_indices"

#define DYNAMICS_MASSMETHOD_DENSITY 0
#define DYNAMICS_MASSMETHOD_MASS	1

#endif