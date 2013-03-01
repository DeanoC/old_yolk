#define  _CRT_SECURE_NO_WARNINGS

#include <lwserver.h>
#include <lwhandler.h>
#include <lwrender.h>
#include <lwcustobj.h>
#include <lwhost.h>
#include <stdlib.h>
#include <string.h>

namespace {
enum { 
	ID_MARKERTYPE = 0x8001, 
};

enum MARKER_TYPES {
	GENERAL_MARKER = 0,
	STARTPOS_MARKER,
	MAX_MARKER_TYPES
};

const char* MarkerTypeStrings[ MAX_MARKER_TYPES + 1] = {
	"General Marker",
	"Player Start Marker",
	nullptr
};

typedef struct {
   LWItemID		id;
   GlobalFunc*	global;
   int			type;
} MarkerObject;

/*
======================================================================
Create()

Handler callback.  Allocate and initialize instance data.  We don't
really have any instance data, but we need to return something, and
this at least shows what create() would normally be doing.
====================================================================== */

static LWInstance Create( void *priv, LWItemID item, LWError *err )
{
   MarkerObject *inst;

   if ( inst = (MarkerObject*) malloc( sizeof( MarkerObject ))) {
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
	MarkerObject *to = (MarkerObject*) a;
	MarkerObject *from = (MarkerObject*) b;
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
	return "VTDFH Marker"; 
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
	return LWCOF_OVERLAY; 
}

#define MARK_SECTION LWID_('M','A','R','K')
static LWBlockIdent SectionIDs[] = {
	{ MARK_SECTION, "Marker" },
	{ 0, 0 }
};

#define MARK_TYPE	LWID_('M','K','T','Y')
// in ID_ order
static LWBlockIdent BlockIDs[] = {
	{ MARK_TYPE, "MarkerType" },
	{ 0, 0 }
};

#define GET_BLOCKID( id) BlockIDs[ ((id)-0x8001) ]
/* Serialise to LWS and back */
static LWError Load( LWInstance ins, const LWLoadState *ls ) { 
	MarkerObject* inst = (MarkerObject*) ins;
	LWID ident = 0;
	// section up data, to avoid LW cross from one instance to the next!
	if( LWLOAD_FIND(ls, &SectionIDs[0]) == MARK_SECTION ) {
		while( ident = LWLOAD_FIND( ls, BlockIDs ) ) {
			switch( ident ) {
			case MARK_TYPE: LWLOAD_I4(ls, &inst->type, true); break;
			}
			LWLOAD_END(ls);
		};
		LWLOAD_END(ls);
	}

	return NULL; 
}
static LWError Save( LWInstance ins, const LWSaveState *ss ) { 
	MarkerObject* inst = (MarkerObject*) ins;
	// we have to insert a root block, otherwise lightwave will read over
    // the end of the block into the next instance!
    LWSAVE_BEGIN(ss, &SectionIDs[0], false);
		LWSAVE_BEGIN( ss, &GET_BLOCKID(ID_MARKERTYPE), true );
			LWSAVE_I4(ss,&inst->type, true );
		LWSAVE_END( ss );
	LWSAVE_END(ss);

	return NULL; 
}
static void * XPanelGet( LWInstance ins, unsigned int vid ) {
	MarkerObject* inst = (MarkerObject*) ins;
	switch ( vid ) {
		case ID_MARKERTYPE:    return &inst->type;
		default:           return NULL;
	}
}

static LWXPRefreshCode XPanelSet( LWInstance ins, unsigned int vid, void *value ) {
	MarkerObject* inst = (MarkerObject*) ins;
	switch ( vid ) {
		case ID_MARKERTYPE: {
			const int* v = (const int*)value;
			inst->type = *v;
		} break;
		default:
			return LWXPRC_NONE;
	}
	return LWXPRC_DFLT;
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
	MarkerObject* inst = (MarkerObject*) ins;

	if( access->flags & LWCOFL_PICKING ) {
		double a[3] = { -5.0, -5.0, 0.0 };
		double b[3] = {  5.0, -5.0, 0.0 };
		double c[3] = {  5.0,  5.0, 0.0 };
		double d[3] = { -5.0,  5.0, 0.0 };

		access->quad( access->dispData, a, b, c, d, LWCSYS_OBJECT );
	} else {
		double txtpos[3] = { 0.0, 3.0, 0.0 };
		access->text( access->dispData, txtpos, MarkerTypeStrings[inst->type], LWJUST_CENTER, LWCSYS_OBJECT );
	}
}



/* Interface panel
*/
static LWXPanelID XPanel( GlobalFunc *global, LWInstance ins ) {
	MarkerObject* inst = (MarkerObject*) ins;
	static LWXPanelControl ctrl_list[] = {
		{ ID_MARKERTYPE,    "Type of Marker",    "iPopChoice",  },
      
		{0}
	};
	static LWXPanelDataDesc data_descrip[] = {
		{ ID_MARKERTYPE,   "Marker Type",   "integer" },
     	{0}
	};

	static LWXPanelHint hint[] = {
		XpLABEL(0,"vtdfh Marker plugin"),

		XpSTRLIST( ID_MARKERTYPE, MarkerTypeStrings ),

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
MarkerHandler()

Handler activation function.  Check the version, get some globals, and
fill in the callback fields of the handler structure.
====================================================================== */
extern "C" {
int MarkerHandler( int version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData) {
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
MarkerInterface()
Interface activation function.
====================================================================== */
int MarkerInterface( int version, GlobalFunc *global, LWInterface *local, void *serverData )
{
	MarkerObject *inst = ( MarkerObject * ) local->inst;
 
	if ( version != LWINTERFACE_VERSION )
      return AFUNC_BADVERSION;
 
   local->panel   = XPanel( global, inst );
   local->options = NULL;
   local->command = NULL;

   inst->global = global;

   return AFUNC_OK;
}

} // end extern "C"