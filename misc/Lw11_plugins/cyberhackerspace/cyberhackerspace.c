/*
======================================================================
barn.c

A custom null in the shape of a barn.

Ernie Wright  16 Mar 00

This isn't entirely useless.  You sometimes want to double-check
programming assumptions about the effects of animation parameters on
the orientation of objects.  Creating this custom null is a convenient
way to get a simple oriented object into the scene.

From F.S. Hill, COMPUTER GRAPHICS, MacMillan, ISBN 0-02-354860-6.
====================================================================== */
#define  _CRT_SECURE_NO_WARNINGS

#include <lwserver.h>
#include <lwcustobj.h>

extern int LevelHandler( int version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData);
extern int LevelInterface( int version, GlobalFunc *global, LWInterface *local, void *serverData );
extern int MarkerHandler( int version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData);
extern int MarkerInterface( int version, GlobalFunc *global, LWInterface *local, void *serverData );


ServerRecord ServerDesc[] = {
   { LWCUSTOMOBJ_HCLASS, "CyberHackerSpace_Level", LevelHandler },
   { LWCUSTOMOBJ_ICLASS, "CyberHackerSpace_Level", LevelInterface },
   { LWCUSTOMOBJ_HCLASS, "CyberHackerSpace_Marker", MarkerHandler },
   { LWCUSTOMOBJ_ICLASS, "CyberHackerSpace_Marker", MarkerInterface },
   { NULL }
};
