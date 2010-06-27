
#ifndef liqcell_EASYRUN_H
#define liqcell_EASYRUN_H

#ifdef __cplusplus
extern "C" {
#endif


#include "liqbase.h"

#include "liqcell.h"

#include "vgraph.h"

int liqcell_easyrun(liqcell *self);

liqcell *liqcell_easyrunstack_topself();        // return the topmost cell in the very first window ("the universe")

typedef struct liqcelleventargs
{
	// the event context passed to objects per paint event
	liqcell *self;
}
	liqcelleventargs;


typedef struct liqcellfiltereventargs
{
	// the event context passed to objects per filter event
	int   filterinuse;		// 0 no filter in effect, 1 filter in use
	char *searchterm;		// normal search pattern, may include a tokensized breakdown of the search below
	char *searchtags;		// list of tags to include
	
	int   resultoutof;		// number of possible units
	int   resultshown;		// number actually remaining visible
}
	liqcellfiltereventargs;



typedef struct liqcellclickeventargs
{
	// the event context passed to objects per paint event
	liqcell *newdialogtoopen;
	int newdialogshowmode;		// 0 zoom to selection, 1 slide in out
}
	liqcellclickeventargs;


typedef struct liqcellpainteventargs
{
	// the event context passed to objects per paint event

	//liqcliprect *cr;
	vgraph *graph;
	int         ox;				// where you should consider top left: 0,0
	int         oy;
	int         mx;				// your actual available dimensions in pixel sizes
	int         my;				// this should have followed a setting on the cell itself and the users preferences
	int         runfast;		// set to 1 to indicate in a runfast session, the render engine will return asap to give you another frame
}
	liqcellpainteventargs;


typedef struct liqcellkeyeventargs
{
	int  keycode;
	char keystring[16];
	int  ispress;
	unsigned int keymodifierstate; // holds the x11 ShiftMask ControlMask Mod5Mask etc
}
	liqcellkeyeventargs;



typedef struct liqcellmouseeventargs
{
	// the event context passed to objects per stroke
	// privately constructed and managed for you
	// to use in a floating section, just adjust by the deltas

	//liqcliprect *cr;
	vgraph *graph;





	liqstroke  *stroke;

	int mcnt;		// count of items

	int msx;		// start item
	int msy;
	int msz;
	unsigned long 	 mst;

	int mex;		// end item
	int mey;
	int mez;
	unsigned long 	 met;

	int mdx;		// item deltas since last invocation
	int mdy;
	int mdz;
	unsigned long 	 mdt;

	liqcell *hit;

	int         ox;				// where you should consider top left: 0,0
	int         oy;

	int 		multiok;		// if set, then multi params are operational
	
	int         multisx;		// starting point used for thumb
	int         multisy;
	
	int         multix;			// best guess estimate for the 2nd finger
	int         multiy;
	int         multiw;
	int         multih;
	
	int         multifx;		// first initial report obtained for the 2nd finger
	int         multify;		// important to know we are now larger than we were at the start

}
	liqcellmouseeventargs;


liqcell * liqcell_easyrun_getactivecontrol();

int liqcell_easyrun_mouseeventargs_multitouchprepare(liqcell *self, liqcellmouseeventargs *args,liqcell *context);

#ifdef __cplusplus
}
#endif

#endif
