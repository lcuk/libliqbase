#ifndef LIQX11OVERLAY_H
#define LIQX11OVERLAY_H



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

typedef struct liqx11overlay
{

	Display				*dpy;
	int					screen;
	Window				window;
	GC 				    gc;
	
	
	int                 yuv_width;
	int                 yuv_height;

	XShmSegmentInfo		yuv_shminfo;
	XvImage *			yuv_image;
	int 				yuv_shminfo_attached;
	int				    xv_port;
} liqx11overlay;








int liqx11overlay_init(liqx11overlay *self, Display *dpy, int screen, Window window, GC gc,   int attrswidth,int attrsheight);
int liqx11overlay_show(liqx11overlay *self);
int liqx11overlay_hide(liqx11overlay *self);
int liqx11overlay_close(liqx11overlay *self);
int liqx11overlay_refreshdisplay(liqx11overlay *self);
int liqx11overlay_drawcolorcube(liqx11overlay *self,int x,int y,char grey);








#endif
