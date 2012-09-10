#define  _CRT_SECURE_NO_WARNINGS

#include <lwserver.h>
#include <lwhandler.h>
#include <lwrender.h>
#include <lwcustobj.h>
#include <lwhost.h>
#include <stdlib.h>
#include <string.h>
#define NUM_ARRAY_ELEMENTS( array ) ( sizeof( (array) ) / sizeof( (array)[0] ) )

namespace {

static double vert[][ 3 ] = {
  30.0,		100.0,	0.0,    	
 -30.0,		100.0,	0.0,    	

  30.0,		10.0,	0.0,    	
  -30.0,	10.0,	0.0,    	

  0.0,		100.0,	0.0,    	
  0.0,		10.0,	0.0,    	

};

static int edge[] = {
   0, 1,  
   2, 3, 
   4, 5,  
};

enum { 
	ID_ITEMTYPE = 0x8001, 
};

typedef struct {
   LWItemID		id;
   GlobalFunc*	global;
   int			type;
} ItemObject;

/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.  We don't
really have any instance data, but we need to return something, and
this at least shows what create() would normally be doing.
====================================================================== */

static LWInstance Create( void *priv, LWItemID item, LWError *err )
{
   ItemObject *inst;

   if ( inst = (ItemObject*) malloc( sizeof( ItemObject ))) {
      inst->id = item;
	  inst->type = 0;
   }
   else
      *err = "Couldn't allocate memory!";

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
	ItemObject *to = (ItemObject*) a;
	ItemObject *from = (ItemObject*) b;
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
	return "VTDFH Item"; 
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

#define ITEM_SECTION LWID_('I','T','E','M')
static LWBlockIdent SectionIDs[] = {
	{ ITEM_SECTION, "Item" },
	{ 0, 0 }
};

#define ITEM_TYPE	LWID_('I','T','T','P')
// in ID_ order
static LWBlockIdent BlockIDs[] = {
	{ ITEM_TYPE, "ItemType" },
	{ 0, 0 }
};

#define GET_BLOCKID( id) BlockIDs[ ((id)-0x8001) ]
/* Serialise to LWS and back */
static LWError Load( LWInstance ins, const LWLoadState *ls ) { 
	ItemObject* inst = (ItemObject*) ins;
	LWID ident = 0;
	// section up data, to avoid LW cross from one instance to the next!
	if( LWLOAD_FIND(ls, &SectionIDs[0]) == ITEM_SECTION ) {
		while( ident = LWLOAD_FIND( ls, BlockIDs ) ) {
			switch( ident ) {
			case ITEM_TYPE: LWLOAD_I4(ls, &inst->type, true); break;
			}
			LWLOAD_END(ls);
		};
		LWLOAD_END(ls);
	}

	return NULL; 
}
static LWError Save( LWInstance ins, const LWSaveState *ss ) { 
	ItemObject* inst = (ItemObject*) ins;
	// we have to insert a root block, otherwise lightwave will read over
    // the end of the block into the next instance!
    LWSAVE_BEGIN(ss, &SectionIDs[0], false);
		LWSAVE_BEGIN( ss, &GET_BLOCKID(ID_ITEMTYPE), true );
			LWSAVE_I4(ss,&inst->type, true );
		LWSAVE_END( ss );
	LWSAVE_END(ss);

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
	ItemObject* inst = (ItemObject*) ins;
	int i;

	for ( i = 0; i < NUM_ARRAY_ELEMENTS(edge)/2; i++ ) {
		access->line( access->dispData, vert[ edge[ (i*2) + 0 ]], vert[ edge[ (i*2) + 1 ]], LWCSYS_OBJECT );
	}

	double txtpos[3] = { 0.0, 20.0, 0.0 };
	access->text( access->dispData, txtpos, "Item", LWJUST_CENTER, LWCSYS_OBJECT );
}



/* Interface panel
*/
static void * XPanelGet( LWInstance ins, unsigned int vid ) {
	ItemObject* inst = (ItemObject*) ins;
	switch ( vid ) {
		case ID_ITEMTYPE:    return &inst->type;
		default:           return NULL;
	}
}

static LWXPRefreshCode XPanelSet( LWInstance ins, unsigned int vid, void *value )
{
	ItemObject* inst = (ItemObject*) ins;
	switch ( vid ) {
		case ID_ITEMTYPE: {
			const int* v = (const int*)value;
			inst->type = *v;
		
		} break;
		default:
			return LWXPRC_NONE;
	}
	return LWXPRC_DFLT;
}

static LWXPanelID XPanel( GlobalFunc *global, LWInstance ins ) {
	ItemObject* inst = (ItemObject*) ins;
	static LWXPanelControl ctrl_list[] = {
		{ ID_ITEMTYPE,    "Item Type",    "integer",  },
      
		{0}
	};
	static LWXPanelDataDesc data_descrip[] = {
		{ ID_ITEMTYPE,   "Item Type",   "integer" },
     	{0}
	};

	static LWXPanelHint hint[] = {
		XpLABEL(0,"vtdfh Item plugin"),
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

} // end private namespace
/*
======================================================================
EnemyHandler()

Handler activation function.  Check the version, get some globals, and
fill in the callback fields of the handler structure.
====================================================================== */
extern "C" {
int ItemHandler( int version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData) {
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
EnemyInterface()
Interface activation function.
====================================================================== */
int ItemInterface( int version, GlobalFunc *global, LWInterface *local, void *serverData )
{
	ItemObject *inst = ( ItemObject * ) local->inst;
 
	if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;
 
   local->panel   = XPanel( global, inst );
   local->options = NULL;
   local->command = NULL;

   inst->global = global;

   return AFUNC_OK;
}

} // end extern "C"