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

#include "liqbase.h"

#include "liqx11overlay.h"



//#############################################################
extern int 			XShmQueryExtension(Display*);
extern int 			XShmGetEventBase(Display*);
extern XvImage  *	XvShmCreateImage(Display*, XvPortID, int, char*, int, int, XShmSegmentInfo*);


#define 			GUID_YUV12_PLANAR 0x32315659	// original
//#############################################################




int liqx11overlay_init(liqx11overlay *self, Display *dpy, int screen, Window window, GC gc)
{
	liqapp_log("x11overlay init begin");
	
	//################################################# initialize our host

	self->dpy    = dpy;
	self->screen = screen;
	self->window = window;
	self->gc     = gc;
	
	
	self->yuv_image=NULL;
	self->yuv_shminfo.shmid = 0;
	self->yuv_shminfo.shmaddr = NULL;
	self->yuv_shminfo.readOnly = False;
	self->yuv_shminfo_attached=0;



	
	//################################################# find out our dimensions


	XWindowAttributes attrs;
	XGetWindowAttributes(
								 self->dpy,
								 self->window,
								 &attrs );

	self->yuv_width  = attrs.width;
	self->yuv_height = attrs.height;

// Sat Aug 22 00:52:46 2009 lcuk : lowres flag added, if set half the resolution of the overlay
if( liqapp_pref_checkexists("lowres") )
{
	self->yuv_width  = attrs.width/2;
	self->yuv_height = attrs.height/2;
}

	liqapp_log("x11overlay dims wh(%i,%i)",self->yuv_width,self->yuv_height);

if(self->yuv_width==480 && self->yuv_height==800)
{
	//self->yuv_width  = 480;
	//self->yuv_height = 480;

	//liqapp_log("x11overlay dims wh(%i,%i)   <<<<ROTATION HACK",self->yuv_width,self->yuv_height);
	
	// 20090605_001351 lcuk : removed this hack, n810 might blow but it doesnt show correctly anyway
}
	//################################################# get adapter info and find our port
	int             ret;
	unsigned int	p_num_adaptors;
	XvAdaptorInfo*  ai;
	

	ret = XvQueryAdaptors(self->dpy, DefaultRootWindow(self->dpy), &p_num_adaptors, &ai);  
	if (ret != Success) 
	{
		{ return liqapp_errorandfail(-1,"canvas XvQueryAdaptors failed"); }
	}

	if(p_num_adaptors==0) 
	{
		{ return liqapp_errorandfail(-1,"canvas XvQueryAdaptors returned no adapters"); }
	}
    
    liqapp_log("x11overlay Canvas Xv p_num_adaptors=%i",p_num_adaptors);
	self->xv_port = ai[0].base_id;
	
	XvFreeAdaptorInfo(ai);
	if (self->xv_port == -1)
	{
		{ return liqapp_errorandfail(-1,"canvas No XV Port on default adapter"); }	
	}

	//################################################# setup shared memory access
	
	liqapp_log("x11overlay XShm init begin");
	
	if (!XShmQueryExtension(self->dpy))
	{
		{ return liqapp_errorandfail(-1,"canvas XShmQueryExtension failed"); }
	}


	//YUV12 and IV420 are same but U&V are swapped
	self->yuv_image = XvShmCreateImage(self->dpy, self->xv_port, GUID_YUV12_PLANAR, 0, self->yuv_width, self->yuv_height, &self->yuv_shminfo);
	//yuv_image = XvShmCreateImage(dpy, xv_port, GUID_IV420_PLANAR, 0, yuv_width, yuv_height, &yuv_shminfo);


	self->yuv_shminfo.shmid = shmget(IPC_PRIVATE, self->yuv_image->data_size, IPC_CREAT | 0777);
	self->yuv_shminfo.shmaddr = self->yuv_image->data = shmat(self->yuv_shminfo.shmid, 0, 0);
	self->yuv_shminfo.readOnly = False;



	//################################################# setup the surface and liqcliprect
	liqapp_log("x11overlay creating canvas.surface");
	canvas.surface=liqimage_new();
	if (!canvas.surface) 
	{
		{ liqapp_errorandfail(-1,"canvas liqimage_create failed"); }
		return -1;
	}
	liqimage_pagedefinefromXVImage(canvas.surface,self->yuv_image,canvas.dpix,canvas.dpiy);





	liqapp_log("x11overlay creating canvas.cr");

	canvas.cr=liqcliprect_newfromimage(canvas.surface);
	if (!canvas.cr) 
	{
		{ liqapp_errorandfail(-1,"canvas liqcliprect_create failed"); }
		return -1;
	}
	
	liqcliprect_drawclear(canvas.cr,0,128,128);


if(canvas.fullscreen)
{
	canvas.pixelwidth  = self->yuv_width;
	canvas.pixelheight = self->yuv_height;
}


	return 0;
	
}



int liqx11overlay_close(liqx11overlay *self)
{

	liqapp_log("x11overlay closing");

	
	if(canvas.cr){ liqcliprect_release(canvas.cr);   canvas.cr=NULL; }



	if(canvas.surface){ liqimage_release(canvas.surface); canvas.surface=NULL; }
	
	
	

	liqx11overlay_hide(self);
	
	if(self->yuv_image)
	{
		self->yuv_image->data = NULL;
		// release this
		//XDestroyImage(self->yuv_image);
		XFree(self->yuv_image);
	}
	if(self->yuv_shminfo.shmaddr)
	{
		shmdt(self->yuv_shminfo.shmaddr);
	}
	if(self->yuv_shminfo.shmid)
	{
		//shmctl(self->yuv_shminfo.shmid, IPC_RMID, 0);
		self->yuv_shminfo.shmid=0;
	}

	return 0;
}




int liqx11overlay_show(liqx11overlay *self)
{
    //return 0;    
	liqapp_log("x11overlay show begin");
	
	if(self->yuv_shminfo_attached) return 0;
	
	liqapp_log("x11overlay showing");
	
	if (!XShmAttach(self->dpy, &self->yuv_shminfo)) 
	{
		{ return liqapp_errorandfail(-1,"canvas XShmAttach failed"); }
	}
	
	liqapp_log("x11overlay shown");
	self->yuv_shminfo_attached=1;
	return 0;
}





int liqx11overlay_hide(liqx11overlay *self)
{
	
    //return 0;    
	liqapp_log("x11overlay hide begin");
	
	if(!self->yuv_shminfo_attached) return 0;

	liqapp_log("x11overlay hiding");

	XShmDetach(self->dpy,&self->yuv_shminfo);

	self->yuv_shminfo_attached=0;

	return 0;
}




int liqx11overlay_refreshdisplay(liqx11overlay *self)
{
	//liqapp_log("refresh..");
    
    
    //return 0;    
    
	
	//liqapp_log("x11overlay refresh begin wh(%i,%i) ",self->yuv_image->width, self->yuv_image->height);
	if(!self->yuv_shminfo_attached) return 0;
	
    //liqcliprect_drawcolorcube( liqcanvas_getcliprect(), 0,0, self->yuv_image->width, self->yuv_image->height,128);

	int res=0;
	
//	if(self->yuv_image->width==480 && self->yuv_image->height==480)
	{
		
		//liqapp_log("x11overlay refresh shortcut");
		
//		res=XvShmPutImage( self->dpy, self->xv_port, self->window, self->gc, self->yuv_image,
//			0, 0, self->yuv_image->width, self->yuv_image->height,
//			0, 0, 480,480, True);//False);//True);
		// set the true flag :)
		//XSync(dpy,False);

		//liqapp_log("x11overlay refresh shortcut done");
//		return 0;
				
	}
    
	
	
	//liqapp_log("x11overlay refresh getting geom");

	int		_x, _y;
	unsigned int _w, _h, _b, _d;
	Window	_dw;
	res=XGetGeometry(self->dpy, self->window, &_dw, &_x, &_y, &_w, &_h, &_b, &_d);
	if(res==0)
	{
		{ return liqapp_errorandfail(-1,"liqx11overlay XGetGeometry failed"); }
	}
	
	// this raises an X11 event (xev.type == 65)  // XShmCompletionEvent
	// when the data has been sent to the display and you are safe to draw again
	
	
	//liqapp_log("x11refresh: _x_y(%i,%i) _w_h(%i,%i) b(%i) wh(%i,%i) ",_x,_y,_w,_h,_b,  self->yuv_image->width, self->yuv_image->height );
	
	
	{	
		res=XvShmPutImage( self->dpy, self->xv_port, self->window, self->gc, self->yuv_image,
			0, 0, self->yuv_image->width, self->yuv_image->height,
			0, 0, _w, _h, True);//False);//True);
		// set the true flag :)
		//XSync(dpy,False);

		
	}



	if(res!=Success)
	{
		{ return liqapp_errorandfail(-1,"x11overlay XvShmPutImage failed"); }
	}
	return 0;
}



//########################################################################
//######################################################################## colorcube optimised 256pixel square
//######################################################################## unused now, but is a nice example of even shading



int liqx11overlay_drawcolorcube(liqx11overlay *self,int x,int y,char grey)
{
	if(x<0)return -1;
	if(y<0)return -1;
	if(x+256>self->yuv_image->width)return -1;
	if(y+266>self->yuv_image->height)return -1;
	//xsurface_drawrect_grey(canvas.surface,x,y,256,256,grey);

int i;
int j;

int uo = self->yuv_image->width * self->yuv_image->height;
int vo = uo + (uo >> 2);
int uvw = self->yuv_image->width>>1;

	x>>=1;
	y>>=1;

unsigned int *planeu = (unsigned int *)&self->yuv_image->data[ uo+(y*uvw)+x ];
unsigned int *planev = (unsigned int *)&self->yuv_image->data[ vo+(y*uvw)+x ];
unsigned int planeskip = (uvw>>2)-32;
unsigned int adjustu = 0x08080808;
unsigned int adjustv = 0x02020202;
unsigned int nu;// = 0x00020406;
unsigned int nv = 0x00000000;
	for(i=0;i<128;i++)
	{
		nu = 0x01030507;
		//nu = 0x00020406;
		for(j=0;j<32;j++)
		{
			// 32 longs make up a chroma cube side
			*planeu++ = nu;
			*planev++ = nv;
			nu+=adjustu;
		}
		nv+=adjustv;
		planeu+=planeskip;
		planev+=planeskip;
	}
	return 0;
}
