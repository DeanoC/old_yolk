#define  _CRT_SECURE_NO_WARNINGS

#include <lwserver.h>
#include <lwhandler.h>
#include <lwrender.h>
#include <lwcustobj.h>
#include <lwhost.h>
#include <stdlib.h>
#include <string.h>

static double vert[][ 3 ] = {
-110.0,		100.0,	0.0,    	
-110.0,		0.0,	0.0,    	
 -50.0,		0.0,	0.0,    	
 -50.0,		100.0,	0.0,    	
  00.0,		0.0,	0.0,    	
  50.0,		100.0,	0.0,    	
  70.0,		100.0,	0.0,    	
  70.0,		0.0,	0.0,    	
 130.0,		0.0,	0.0,    	
};

static int edge[][ 2 ] = {
   0, 1,  
   1, 2, 
   3, 4,  
   4, 5,  
   6, 7,  
   7, 8
};

enum { 
	ID_LEVELNAME = 0x8001, 
	ID_LEVELDESC
};

typedef struct {
   LWItemID		id;
   GlobalFunc*	global;
   char			levelName[256];
   char			levelDesc[1024];
} MyObject;

/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.  We don't
really have any instance data, but we need to return something, and
this at least shows what create() would normally be doing.
====================================================================== */

static LWInstance Create( void *priv, LWItemID item, LWError *err )
{
   MyObject *inst;

   if ( inst = (MyObject*) malloc( sizeof( MyObject ))) {
      inst->id = item;
	  inst->levelName[0] = 0;
	  inst->levelDesc[0] = 0;   }
   else
      *err = "Couldn't allocate 4 bytes!";

   return inst;
}


/*
======================================================================
Destroy()

Handler callback.  Free resources allocated by Create().
====================================================================== */
static void Destroy( LWInstance ins ) {
   if( ins ) free( ins );
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.
====================================================================== */
static LWError Copy( LWInstance a, LWInstance b ) {
	MyObject *to = (MyObject*) a;
	MyObject *from = (MyObject*) b;
	LWItemID id;

	id = to->id;
	*to = *from;
	to->id = id;

	return NULL;
}


/*
======================================================================
Load(), Save(), Describe(), UseItems(), ChangeID(), Init(), Cleanup(),
NewTime(), Flags()

We're a pretty simple plug-in, so we don't need to do anything in
these callbacks, but they're here if we want to fill them in later.
====================================================================== */

static const char * Describe( LWInstance inst ) { 
	return "CyberHackerSpace level desciptor"; 
}

static const LWItemID * UseItems( LWInstance inst ) { 
	return nullptr; 
}

static void ChangeID( LWInstance inst, const LWItemID *ids ) { 
}

static LWError Init( LWInstance inst, int mode ) { 
	return nullptr; 
}

static void Cleanup( LWInstance inst ) { 
}

static LWError NewTime( LWInstance inst, LWFrame fr, LWTime t ) { 
	return nullptr; 
}

static unsigned int Flags( LWInstance inst ) { 
	return 0; 
}


#define LVLNAME_ID	LWID_('L','V','N','M')
#define LVLDESC_ID	LWID_('L','V','D','C')
// in ID_ order
static LWBlockIdent BlockIDs[] = {
	{ LVLNAME_ID, "LevelName" },
	{ LVLDESC_ID, "LevelDesc" },
	{ 0, 0 }
};

#define GET_BLOCKID( id) BlockIDs[ ((id)-0x8001) ]
/* Serialise to LWS and back */
static LWError Load( LWInstance ins, const LWLoadState *ls ) { 
	MyObject* inst = (MyObject*) ins;
	LWID ident = 0;
	while( (ident = LWLOAD_FIND( ls, BlockIDs )) ) {
		switch( ident ) {
		case LVLNAME_ID: LWLOAD_STR(ls, inst->levelName,sizeof(inst->levelName)); break;
		case LVLDESC_ID: LWLOAD_STR(ls, inst->levelDesc,sizeof(inst->levelDesc)); break;
		}
		LWLOAD_END(ls);
	} 
	return NULL; 
}
static LWError Save( LWInstance ins, const LWSaveState *ss ) { 
	MyObject* inst = (MyObject*) ins;

	LWSAVE_BEGIN( ss, &GET_BLOCKID(ID_LEVELNAME), 1 );
		LWSAVE_STR(ss,inst->levelName);
	LWSAVE_END( ss );
	LWSAVE_BEGIN( ss, &GET_BLOCKID(ID_LEVELDESC), 1 );
		LWSAVE_STR(ss,inst->levelDesc);
	LWSAVE_END( ss );

	return NULL; 
}

/*
======================================================================
Evaluate()

Handler callback.  This is called each time the custom object needs to
be redrawn.  We just draw our edges, letting Layout choose the color
based on selection state and user preference.
====================================================================== */

static void Evaluate( LWInstance ins, const LWCustomObjAccess *access )
{
	MyObject* inst = (MyObject*) ins;
	int i;

	for ( i = 0; i < 6; i++ )
		access->line( access->dispData,
			vert[ edge[ i ][ 0 ]], vert[ edge[ i ][ 1 ]], LWCSYS_OBJECT );
}



/* Interface panel
*/
static void * XPanelGet( LWInstance ins, unsigned int vid ) {
	MyObject* inst = (MyObject*) ins;
	switch ( vid ) {
		case ID_LEVELNAME:    return inst->levelName;
		case ID_LEVELDESC:    return inst->levelDesc;
		default:           return NULL;
	}
}

static LWXPRefreshCode XPanelSet( LWInstance ins, unsigned int vid, void *value )
{
	MyObject* inst = (MyObject*) ins;
	switch ( vid ) {
		case ID_LEVELNAME: {
			const char* str = (const char*)value;
			strcpy( inst->levelName, str );

		} break;
		case ID_LEVELDESC:{
			const char* str = (const char*)value;
			strcpy( inst->levelDesc, str );
		} break;

		default:
			return LWXPRC_NONE;
	}
	return LWXPRC_DFLT;
}

static LWXPanelID XPanel( GlobalFunc *global, LWInstance ins ) {
	MyObject* inst = (MyObject*) ins;
	static LWXPanelControl ctrl_list[] = {
		{ ID_LEVELNAME,    "Level Name",    "string",  },
		{ ID_LEVELDESC,    "Level Description",    "string",  },      
		{0}
	};
	static LWXPanelDataDesc data_descrip[] = {
		{ ID_LEVELNAME,   "Level Name",   "string" },
		{ ID_LEVELDESC,   "Level Description",   "string" },
     	{0}
	};

	static LWXPanelHint hint[] = {
		XpLABEL(0,"cyberhackerspace Level plugin"),
		XpEND
	};

	LWXPanelFuncs *lwxpf = NULL;
	LWXPanelID     panID = NULL;
	lwxpf = (LWXPanelFuncs*)(*global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
	if ( lwxpf ) {
		panID = (*lwxpf->create)( LWXP_VIEW, ctrl_list );  
		if ( panID ) {
			(*lwxpf->hint)    ( panID, 0, hint );
			(*lwxpf->describe)( panID, data_descrip, (LWXPanelGetFunc*) &XPanelGet, (LWXPanelSetFunc*) &XPanelSet );
			(*lwxpf->viewInst)( panID, inst );
			(*lwxpf->setData) ( panID, 0, inst);
		}
	}
	return panID;
}
/*
======================================================================
LevelHandler()

Handler activation function.  Check the version, get some globals, and
fill in the callback fields of the handler structure.
====================================================================== */
extern "C" {
int LevelHandler( int version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData) {
   if ( version != LWCUSTOMOBJ_VERSION )
      return AFUNC_BADVERSION;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->copy    = Copy;
   local->inst->descln  = Describe;

   if ( local->item ) {
      local->item->useItems = UseItems;
      local->item->changeID = ChangeID;
   }

   if ( local->rend ) {
      local->rend->init    = Init;
      local->rend->cleanup = Cleanup;
      local->rend->newTime = NewTime;
   }

   local->evaluate = Evaluate;
   local->flags    = Flags;

   return AFUNC_OK;
}

/*
======================================================================
LevelInterface()
Interface activation function.
====================================================================== */
int LevelInterface( int version, GlobalFunc *global, LWInterface *local, void *serverData )
{
	MyObject *inst = ( MyObject * ) local->inst;
 
	if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;
 
   local->panel   = XPanel( global, inst );
   local->options = NULL;
   local->command = NULL;

   inst->global = global;

   return AFUNC_OK;
}

} // end extern "C"