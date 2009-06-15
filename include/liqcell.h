

#ifndef liqcell_H
#define liqcell_H

#include "liqbase.h"
//#include "liqcell_easyrun.h"
//#include "liqcell_prop.h"


//#########################################################################
//#########################################################################
//######################################################################### cell type definition
//#########################################################################
//#########################################################################

typedef
struct liqcell
{
	unsigned int usagecount;
	//struct liqcell *linkcontent;			// this gives me a shiver..
	struct liqcell *linkparent;
	struct liqcell *linkprev;
	struct liqcell *linknext;
	struct liqcell *linkchild;
	//int childcount;

	int    kind;						// see cellkind_ enumeration


										// 1=prop    		- a none visual property
										// 2=visual 		- a user interface element
										// 4=widget         - a core widget component, the "base"

	char * name;
	char * classname;
	char * context;
	void * data;						// YOU are in charge of allocation, perhaps I Should enforce rigidity

	// todo: VVV maybe fold these up into a single uint bitmask VVV

	int deleted;						// marked as deleted (recoverable), will decide on this later
	int visible;
	int enabled;
	int selected;


	int x;								// these are our dimensions according to the parent
	int y;
	int w;
	int h;

	int kineticx;						// this is how fast we are travelling relative to our parent
	int kineticy;						// this will be moved shortly

	int overlapx;						// amount of overlap (just to keep the algo in place)
	int overlapy;


	int innerw;							// these are our total required dimensions according to our contents
	int innerh;							// all children are expected to exist within this area

	// another
	struct liqcell *content;				// contained cell :)

	liqsketch *sketch;					// one of each of the media types are required.  i need to be able to write paint or draw directly :)
	liqimage  *image;
	liqfont   *font;
	int dirty;							// the dirty flag is important, and should really be automatic
	int tag;
	int dirtyhold;
	char *caption;

	unsigned int unused[8];

}	liqcell;

#define cellkind_prop      1
#define cellkind_visual    2
#define cellkind_widget    4
#define cellkind_shown     8


int liqcell_iskind(liqcell *self,int cellkind);
/*
	// old style iter code
	liqcell *c=self->linkchild;
	while(c)
	{
		// do action
		c=c->linknext;
	}

	// newer OO method, aim to not touch the ->members directly
	liqcell *c=liqcell_getlinkchild(self);
	while(c)
	{
		// do action
		c=liqcell_getlinknext(c);
	}
 */

//#########################################################################
//#########################################################################
//######################################################################### cell construction and reference counting
//#########################################################################
//#########################################################################
liqcell * liqcell_new();
liqcell * liqcell_hold(liqcell *self);
void    liqcell_release(liqcell *self);
void    liqcell_free(liqcell *self);

//######################################################################### standard constructors

liqcell*  liqcell_quickcreatewidget(char *name,char *classname,int innerw,int innerh);
liqcell*  liqcell_quickcreatevis(char *name,char *classname,int x,int y,int w,int h);
liqcell*  liqcell_quickcreatedata(char *name,char *classname,void *data);
liqcell*  liqcell_quickcreatenameclass(char *name,char *classname);
//liqcell * liqcell_quickcreatefull(char *name,char *classname,char *context,void *data);

//######################################################################### children and tree management

liqcell*  liqcell_child_append(liqcell *self,liqcell *child);
liqcell*  liqcell_child_insert(liqcell *self,liqcell *child);
liqcell*  liqcell_child_insertsorted(liqcell *self, liqcell * ch);
liqcell*  liqcell_child_insertsortedbyname(liqcell *self, liqcell * ch,int sortpositive);
int liqcell_child_remove(liqcell *self,liqcell *child);
int liqcell_child_removeall(liqcell *self);
int liqcell_child_removeallvisual(liqcell *self);

liqcell*  liqcell_child_lookup(liqcell *self,char *name); // use dotted branches
liqcell*  liqcell_child_lookup_simple(liqcell *self,char *name);	// ignore dotted branches
liqcell*  liqcell_child_lookup_nameclass(liqcell *self,char *name,char *classname);



liqcell *	liqcell_getlinkparent(liqcell *self);
liqcell *	liqcell_getlinkprev(liqcell *self);
liqcell *	liqcell_getlinknext(liqcell *self);
liqcell *	liqcell_getlinkchild(liqcell *self);


//######################################################################### searching

//liqcell*  liqcell_findfirst(liqcell *self,char *query);
liqcell*  liqcell_findnext(liqcell *self,char *query);



liqcell*  liqcell_local_lookup(liqcell *self,char *name);
liqcell*  liqcell_local_lookup_nameclass(liqcell *self,char *name,char *classname);
liqcell*  liqcell_global_lookup(liqcell *self,char *name);
liqcell*  liqcell_global_lookup_nameclass(liqcell *self,char *name,char *classname);

void *  liqcell_handlerfind(liqcell *self,char *handlername);
liqcell*  liqcell_handleradd( liqcell *self,char *handlername, void *handler);
// add a handler but pass in some context data
// the context is passed into the handler through an additional 3rd parameter
liqcell*  liqcell_handleradd_withcontext( liqcell *self,char *handlername, void *handler,void *context);
int 	liqcell_handlerrun( liqcell *self,char *handlername,void *args);



// example handler prototype
// context will be NULL unless explicitely set at the time of configuring the handler
//int 	handler(liqcell *self,void *eventargs,void *context);


char*  liqcell_local_lookup_getname(liqcell *self,char *name);
char*  liqcell_local_lookup_getcaption(liqcell *self,char *name);


//######################################################################### standard control properties

void 	liqcell_setname(liqcell *self,char *name);				// symbolic identifier
char *	liqcell_getname(liqcell *self);



void 	liqcell_setcaption(liqcell *self,char *caption);		// easy translatable label
char *	liqcell_getcaption(liqcell *self);



void 	liqcell_setclassname(liqcell *self,char *classname);	// class name used to construct
char *	liqcell_getclassname(liqcell *self);



void 	liqcell_setcontext(liqcell *self,char *context);		// variation used (if applicable)
char *	liqcell_getcontext(liqcell *self);





void 	liqcell_setcontent(liqcell *self,liqcell *content);
liqcell *	liqcell_getcontent(liqcell *content);

int     liqcell_getqualifiedname(liqcell *self, char *buff, int buffmax);

void 	liqcell_setdata(liqcell *self,void *data);
void *	liqcell_getdata(liqcell *self);


void 	liqcell_settag(liqcell *self,void *tag);
void *	liqcell_gettag(liqcell *self);


//liqcell *	liqcell_getlinkcontent(liqcell *self);



//######################################################################### ui/interaction
void 	liqcell_setvisible(liqcell *self,int arg);				// set the visible indicator
int    	liqcell_getvisible(liqcell *self);

void 	liqcell_setenabled(liqcell *self,int arg);				// set the enabled indicator
int 	liqcell_getenabled(liqcell *self);

void 	liqcell_setselected(liqcell *self,int arg);				// set the selected indicator
int 	liqcell_getselected(liqcell *self);

void 	liqcell_setdirty(liqcell *self,int dirty);				// set the dirty flag :)  this cascades through parents as well
int    	liqcell_getdirty(liqcell *self);

void 	liqcell_setdirtyhold(liqcell *self,int dirtyhold);		// hold off on telling parent (useful if there are multiple changes due)
int    	liqcell_getdirtyhold(liqcell *self);


void 	liqcell_setshown(liqcell *self,int arg);				// set the shown indicator flag
int    	liqcell_getshown(liqcell *self);

int     liqcell_getflagvisual(liqcell *self);
int     liqcell_getflagwidget(liqcell *self);
//######################################################################### style attributes

void 	liqcell_setsketch(liqcell *self,liqsketch *sketch);
liqsketch*liqcell_getsketch(liqcell *self);


void 	liqcell_setimage(liqcell *self,liqimage *image);
liqimage *liqcell_getimage(liqcell *self);

void 	liqcell_setfont(liqcell *self,liqfont *font);
liqfont *liqcell_getfont(liqcell *self);


void    liqcell_sketch_autoload(liqcell *self);

//######################################################################### rectangle boundary handlers

void 	liqcell_setpos(liqcell *self,int x,int y);
void 	liqcell_setsize(liqcell *self,int w,int h);
void 	liqcell_adjustpos(liqcell *self,int dx,int dy);
void 	liqcell_adjustsize(liqcell *self,int dw,int dh);
void 	liqcell_setkinetic(liqcell *self,int kx,int ky);
void 	liqcell_setrect(liqcell *self,int x,int y,int w,int h);

int 	liqcell_movetowardsrect(liqcell *self,int x,int y,int w,int h, float fraction);	// return 0 if at target coords already, 1 if not yet there


void liqcell_setrect_autoscale(liqcell *self,int x,int y,int w,int h,float sx,float sy);

int    	liqcell_getx(liqcell *self);
int    	liqcell_gety(liqcell *self);
int    	liqcell_getw(liqcell *self);
int    	liqcell_geth(liqcell *self);
int    	liqcell_getinnerw(liqcell *self);
int    	liqcell_getinnerh(liqcell *self);
int    	liqcell_getkineticx(liqcell *self);
int    	liqcell_getkineticy(liqcell *self);

void 	liqcell_forceinboundparent(liqcell *self);

//######################################################################### Misc Functions

void 	liqcell_zorder_totop(liqcell *self);  // moves the cell to the top of the zorder, NULL function at present
liqcell * liqcell_getbasewidget(liqcell *self); // called from within an event steps backwards until it finds the base widget this item was created by

//######################################################################### Arrangement and Layout tools


int     liqcell_child_countvisible(liqcell *self);

int 	liqcell_child_arrange_easytile(liqcell *self);			// make sure all contents are bound within the area
int 	liqcell_child_arrange_easyrow(liqcell *self);			// split into Left|Centre|Right boxes (newspaper panels)
int 	liqcell_child_arrange_easycol(liqcell *self);			// split into Top|Middle|Bottom boxes (sections)


int 	liqcell_child_arrange_autoflow(liqcell *self);			// flowing document: like text does;
int 	liqcell_child_arrange_nooverlap(liqcell *self,liqcell *currentselection);
int 	liqcell_child_arrange_makegrid(liqcell *self,int viscolcount,int visrowcount);



//######################################################################### special handlers




#endif
