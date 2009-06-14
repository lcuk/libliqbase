#ifndef LIQX11INFO_H
#define LIQX11INFO_H



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>



#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <X11/extensions/XShm.h>


#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "liqcanvas.h"
#include "liqx11overlay.h"

typedef struct liqx11info
{
	Display *		mydisplay;
	Window 			mywindow;
	GC 				mygc;
	int 			myscreen;
	
	Atom    		my_WM_DELETE_WINDOW;
	
	liqx11overlay   myoverlaycore;
	liqx11overlay *	myoverlay;
	
	
	int             myinnotifyflag;
	int             myispressedflag;
	int             myisvisibleflag;
	int             myisfocusflag;
	
}
	liqx11info;




int liqx11info_init(liqx11info *myx11info, int pixelwidth,int pixelheight,int fullscreen);
int liqx11info_close(liqx11info *myx11info);



int liqx11info_refreshdisplay(liqx11info *myx11info);
int liqx11info_eventgetcount(liqx11info *myx11info);
int liqx11info_eventgetnext(liqx11info *myx11info,XEvent *event);


int liqx11info_get_next_liqevent(liqx11info *myx11info,LIQEVENT *ev,int *dirtyflagptr);





#endif
