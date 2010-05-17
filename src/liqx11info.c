#include <X11/Xlib.h>
#include <X11/Xutil.h>


#include "liqbase.h"

#include "liqx11info.h"

#include "liqx11overlay.h"


//############################################################# keyboard bits

#define ShiftMask               (1<<0)
#define ControlMask             (1<<2)
#define Mod5Mask                (1<<7)


#define XK_Shift_L                       0xffe1
#define XK_Control_L                     0xffe3
#define XK_ISO_Level3_Shift              0xfe03



#define liqx11_modifier_StickyMask (1<<15)


int liqx11_modifierprev=0;
int liqx11_modifier=0;


XImage *cover_image = NULL;
XImage *liqimage_convert_to_ximage(liqimage *self, Display *dis, int screen);

void cover_image_release(liqx11info *myx11info)
{

	if(cover_image)
	{
		XFree(cover_image);
		cover_image=NULL;
	}
	
}
	
void cover_image_rebuild(liqx11info *myx11info)
{

	cover_image_release(myx11info);
	
	cover_image = liqimage_convert_to_ximage( liqcanvas_getsurface(),  myx11info->mydisplay , myx11info->myscreen);
	
}
	
void cover_image_blit(liqx11info *myx11info)
{

	if(cover_image)
	{
		XPutImage (myx11info->mydisplay, myx11info->mywindow, myx11info->mygc, cover_image, 0, 0, 0, 0, liqcanvas_getwidth(), liqcanvas_getheight() );
		XFlush (myx11info->mydisplay);
	}
	
}

//############################################################# 

int x11_get_first_xvport(Display *display)
{
	
	
	
	//################################################# get adapter info and find our port
	int             ret;
	unsigned int	p_num_adaptors;
	XvAdaptorInfo*  ai;
	

	ret = XvQueryAdaptors(display, DefaultRootWindow(display), &p_num_adaptors, &ai);  
	if (ret != Success) 
	{
		{ return liqapp_errorandfail(-1,"x11_get_first_xvport XvQueryAdaptors failed"); }
	}

	if(p_num_adaptors==0) 
	{
		{ return liqapp_errorandfail(-1,"x11_get_first_xvport XvQueryAdaptors returned no adapters"); }
	}
    
    liqapp_log("x11_get_first_xvport Xv p_num_adaptors=%i  ai[0].base_id=%d",p_num_adaptors, ai[0].base_id);
	
	return ai[0].base_id;
	
}


static void x11_set_fullscreen_state(Display *display, Window window, int action)
{
    XEvent xev;
    /* init X event structure for _NET_WM_FULLSCREEN client msg */
    xev.xclient.type 		= ClientMessage;
    xev.xclient.serial 		= 0;
    xev.xclient.send_event 	= True;
    xev.xclient.message_type= XInternAtom(display, "_NET_WM_STATE", False);
    xev.xclient.window 		= window;
    xev.xclient.format 		= 32;
    xev.xclient.data.l[0]	= action;
    xev.xclient.data.l[1]	= XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[2]	= 0;
    xev.xclient.data.l[3]	= 0;
    xev.xclient.data.l[4]	= 0;
    /* finally send that damn thing */
  //  if( !XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev) )
    if( !XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask , &xev) )
	{
		{ liqapp_errorandfail(-1,"canvas x11 could not set fullscreen"); }
    }
    XSync(display, False);
}



	
int liqx11info_regrab_focus(liqx11info *myx11info)
{
	// 20090708_200813 lcuk : thank you danielwilms for pointing me in the right direction
	XSetInputFocus(myx11info->mydisplay, myx11info->mywindow, RevertToPointerRoot, CurrentTime);
	if(!XRaiseWindow(myx11info->mydisplay, myx11info->mywindow))
	{
		{ liqapp_errorandfail(-1,"x11info XRaiseWindow, could not raise"); }
	}
}




	

//############################################################# 



int liqx11info_init(liqx11info *myx11info, int pixelwidth,int pixelheight,int fullscreen)
{



//XEvent 			xev;
//KeySym 			mykey;


	
	
	liqapp_log("x11info starting up");
	
	
	canvas.keepalivealarmtime=10000;
	

	

	myx11info->myoverlay     = NULL;
	myx11info->mydisplay     = XOpenDisplay("");
	myx11info->myscreen      = DefaultScreen(myx11info->mydisplay);
	myx11info->mywindow      = 0;//NULL;
	
	
	myx11info->myinnotifyflag=0;
	myx11info->myispressedflag=0;
	myx11info->myisvisibleflag=0;
	myx11info->myisfocusflag=0;
	
	
	
	XVisualInfo				vinfo;

    // why do i need 16bit, i need xv, not 16b..
	if(       
       XMatchVisualInfo(myx11info->mydisplay, myx11info->myscreen, 24, TrueColor    , &vinfo) ||
       XMatchVisualInfo(myx11info->mydisplay, myx11info->myscreen, 24, DirectColor  , &vinfo) ||
       XMatchVisualInfo(myx11info->mydisplay, myx11info->myscreen, 16, TrueColor    , &vinfo) ||
       XMatchVisualInfo(myx11info->mydisplay, myx11info->myscreen, 16, DirectColor  , &vinfo) ||
       XMatchVisualInfo(myx11info->mydisplay, myx11info->myscreen, 15, TrueColor    , &vinfo) ||
       XMatchVisualInfo(myx11info->mydisplay, myx11info->myscreen, 15, DirectColor  , &vinfo)
       )
    {
        // ok
    }
    else
	{
		{ return liqapp_errorandfail(-1,"x11info cannot XMatchVisualInfo"); }
	}
	
	



XvPortID  xvport_num = x11_get_first_xvport(myx11info->mydisplay);
int       xvoverlaycolorkey=-1;

        Atom xv_colorkey = XInternAtom(myx11info->mydisplay, "XV_COLORKEY", 0);
        XvGetPortAttribute(myx11info->mydisplay, xvport_num, xv_colorkey, &xvoverlaycolorkey);
		
		
		
int       xvautopaintcolorkey=-1;

        Atom xv_autopaintcolorkey = XInternAtom(myx11info->mydisplay, "XV_AUTOPAINT_COLORKEY", 0);
        XvGetPortAttribute(myx11info->mydisplay, xvport_num, xv_autopaintcolorkey, &xvautopaintcolorkey);


	liqapp_log("xv_colorkey %d,%d",xvoverlaycolorkey,xvautopaintcolorkey);
	xvoverlaycolorkey=1;
	XvSetPortAttribute(myx11info->mydisplay, xvport_num, XInternAtom(myx11info->mydisplay, "XV_COLORKEY", True), xvoverlaycolorkey );


	xvautopaintcolorkey=1;
	XvSetPortAttribute(myx11info->mydisplay, xvport_num, XInternAtom(myx11info->mydisplay, "XV_AUTOPAINT_COLORKEY", True), xvautopaintcolorkey );
	


	//################################################# 
	
	
	liqapp_log("x11info preparing baselines and hint");

	XSizeHints 		myhint;
	unsigned long 	myforeground;
	unsigned long 	mybackground;
	
		mybackground  = BlackPixel(myx11info->mydisplay, myx11info->myscreen);
		myforeground  = WhitePixel(myx11info->mydisplay, myx11info->myscreen);
		
		liqapp_log("colors Back=%d fore=%d key=%d",mybackground,myforeground,xvoverlaycolorkey);
		
		mybackground = xvoverlaycolorkey;


	
		myhint.x      = 0;
		myhint.y      = 0;
		myhint.width  = pixelwidth;
		myhint.height = pixelheight;
		myhint.flags  = PPosition | PSize;
		
		liqapp_log("x11info creating window");
		
		myx11info->mywindow = XCreateSimpleWindow(myx11info->mydisplay,
			DefaultRootWindow(myx11info->mydisplay),
			myhint.x, myhint.y, myhint.width, myhint.height, 
			5, myforeground, mybackground);



	liqapp_log("x11info setting ClassHint (thanks qwerty12!)");
	
	char *tit = strdup(liqapp_gettitle());
	if(!tit)
	{
		{ liqapp_errorandfail(-1,"x11info ClassHint, could not alloc tit"); }
		
	}
	// 20090617_201322 lcuk : qwerty12 says i need a class hint as well.
	char *t=tit;
	while(t && *t)
	{
		if(*t=='-') *t='_';
		t++;
	}
	
		XClassHint classhint;
		classhint.res_name =tit;
		classhint.res_class=tit;
		
		XSetClassHint(myx11info->mydisplay, myx11info->mywindow, &classhint);
	

	


	liqapp_log("x11info setting window properties");


	
	//XSetStandardProperties(myx11info->mydisplay, myx11info->mywindow, liqapp_gettitle(),liqapp_gettitle(), None, NULL, 0, &myhint);
	XSetStandardProperties(myx11info->mydisplay, myx11info->mywindow, tit,tit, None, NULL, 0, &myhint);
	
	myx11info->mygc = XCreateGC(myx11info->mydisplay, myx11info->mywindow, 0, 0);

	free(tit);

	// ################### good x11 info
	// http://www.openmash.org/lxr/source/xlib/X11/X.h?c=tk8.3#L99
	// http://static.cray-cyber.org/Documentation/NEC_SX_R10_1/G1AE02E/CHAP10.HTML



 	//################################################# init normal x11 handler mask
	// set the actual event mask we normally want for this window
	XSelectInput(	myx11info->mydisplay,myx11info->mywindow,
				 
						StructureNotifyMask |
						
						//SubstructureNotifyMask |

						VisibilityChangeMask |


						EnterWindowMask |
						LeaveWindowMask |
						
						FocusChangeMask |

						ExposureMask |
						
						ButtonPressMask |
						ButtonMotionMask |
						ButtonReleaseMask |
						
						
						KeyPressMask |
						KeyReleaseMask
						
						);
	
	
 	//################################################# add the WM_delete_window atom
    myx11info->my_WM_DELETE_WINDOW = XInternAtom(myx11info->mydisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(myx11info->mydisplay, myx11info->mywindow, &myx11info->my_WM_DELETE_WINDOW, 1);

		
	
	//################################################# lets make sure we are top of the pile
	
	XMapRaised(		myx11info->mydisplay,myx11info->mywindow);
		
	
	if(fullscreen)
	{
		liqapp_log("x11info going fullscreen");
		x11_set_fullscreen_state(myx11info->mydisplay,myx11info->mywindow,1);
	}

	XEvent event;
	do 
	{
		XNextEvent(myx11info->mydisplay, &event);
		liqapp_log("looping %i  %i",event.type,MapNotify);
	}
	while (event.type != MapNotify || event.xmap.event != myx11info->mywindow);

	//################################################# put the overlay in place :)

	liqapp_log("x11info allocating overlay");
	
	myx11info->myoverlay = &myx11info->myoverlaycore;
	liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc,  canvas.pixelwidth,canvas.pixelheight);
	
	liqapp_log("x11info binding overlay to window");
	myx11info->myoverlay->window = myx11info->mywindow;

	liqapp_log("x11info showing overlay");
	liqx11overlay_show(myx11info->myoverlay);

	
	return 0;
}
	

	
	
	
	
	
	
	
	
int liqx11info_close(liqx11info *myx11info)
{
	
	liqapp_log("x11 shutting down");
	
	

	if(myx11info->myoverlay)
	{				
		//xev.xconfigure.width
		liqx11overlay_close(myx11info->myoverlay);
		myx11info->myoverlay = NULL;
	}
				
				
	XFreeGC(myx11info->mydisplay, myx11info->mygc);
	XUnmapWindow(myx11info->mydisplay,myx11info->mywindow);
	XDestroyWindow(myx11info->mydisplay, myx11info->mywindow);
	XCloseDisplay(myx11info->mydisplay);

	return 0;
}





int liqx11info_eventgetcount(liqx11info *myx11info)
{
	int evc=XEventsQueued(myx11info->mydisplay, QueuedAfterFlush);
	//liqapp_log("evc %i",evc);
	return evc;
}





int liqx11info_eventgetnext(liqx11info *myx11info,XEvent *event)
{
	return XNextEvent(myx11info->mydisplay, event);
}














int liqx11info_refreshdisplay(liqx11info *myx11info)
{
    
/*    
    XImage *liqimage_convert_to_ximage(liqimage *self, Display *dis, int screen);
 
 
	//void *actualx11display;	// this will always be live within an x11 framework
	//void *actualx11window; 
    
    XImage *img = liqimage_convert_to_ximage(canvas.surface, myx11info->mydisplay,myx11info->myscreen);
    if(img)
    {

		XPutImage(myx11info->mydisplay, myx11info->mywindow, myx11info->mygc, img, 0, 0, 0, 0, img->width, img->height);
		XFlush(myx11info->mydisplay);
        XDestroyImage(img);
    }
 */    
	return liqx11overlay_refreshdisplay(myx11info->myoverlay);
}







//############################################################# 
//############################################################# liqcanvas_nextevent_x11 
//############################################################# 
//############################################################# 










int liqx11info_minimize(liqx11info *myx11info)
{
	
						if((myx11info->myoverlay))
						{
							cover_image_rebuild(myx11info);
							cover_image_blit(myx11info);
							liqx11overlay_hide(myx11info->myoverlay);
							//liqx11overlay_refreshdisplay(myx11info->myoverlay);
						}	
}


int liqx11info_get_next_liqevent(liqx11info *myx11info,LIQEVENT *ev,int *dirtyflagptr)
{
	ev->type = 		LIQEVENT_TYPE_NONE;
	ev->ticks =     liqapp_GetTicks();
	ev->state =     LIQEVENT_STATE_NONE;
	
	XEvent xev={0};
unsigned long ticksnow=ev->ticks;
//  goto foo;
//	canvas.keepalivealarmtime=0;
//  ok we are here.  we are not sure if there are any events
	while( ((canvas.keepalivealarmtime>0) && (liqx11info_eventgetcount(myx11info)==0)) )
	{
		// we have been called with NO events pending.
		// we must sleep
		//liqapp_log("Sleeping...%u,%u dif=%u  kat=%u",ev->ticks,ticksnow,ticksnow-ev->ticks,canvas.keepalivealarmtime);
		liqapp_sleep(25);
		if(liqcanvas_eventcount()>0) break;
		
		if((dirtyflagptr) && (*dirtyflagptr))
		{
			//liqapp_log("dirty flag bailout!!");
			ev->type = LIQEVENT_TYPE_DIRTYFLAGSET;
			return 1;
		}
		
		ticksnow = liqapp_GetTicks();
		
		if(liqx11info_eventgetcount(myx11info)>0) break;

		if(ticksnow-ev->ticks > canvas.keepalivealarmtime)// && myx11info->myinnotifyflag)
		{
			liqapp_log("Waking!...%i,%i",ev->ticks,ticksnow);

			//// we exit and return 
			//ev->type = 		LIQEVENT_STATE_KEEPALIVE;
			//ev->ticks =     liqapp_GetTicks();
			//return 0;

			// hang on, got a better idea :)
			// lets just refresh and carry on looping :)
			// ... no need for the client to do anything
			// because we are holding from this loop for the entire duration,
			// if we have waited longer than the clients' real blanking timeout
			// then we can just let it drop through and out of this loop to let it blank itself
			// (which it was already doing, just too early)

			//liqcanvas_xv_refreshdisplay();
			
			liqx11info_refreshdisplay( myx11info );
			
			ev->type = 		LIQEVENT_TYPE_NONE;
			ev->ticks =     liqapp_GetTicks();		

		}
	}
	
	
	// pan out, visibility gone
	// THEN(1)pan to other app, focus gone too
	// THEN(2)pan back in, configurenotify should restore visibility
	
	
	
	//myx11info->myisvisibleflag = 1;
	
foo:

	//liqx11info_eventgetnext();

	//XNextEvent(dpy,&xev);
	
	
	liqx11info_eventgetnext( myx11info, &xev );
	
	//http://linux.die.net/man/3/xanyevent
	
	int keylen=0;
	liqapp_log("x11.NextEvent %i  dpy=%d window=%d root=%d subwindow=%d,   vis=%d foc=%d",
			   xev.type,xev.xany.display,xev.xany.window, xev.xcrossing.root,xev.xcrossing.subwindow,
			   myx11info->myisvisibleflag,myx11info->myisfocusflag);
	

	
	
			switch (xev.type)
			{					
	
	
				case ClientMessage:
					if (xev.xclient.data.l[0] == myx11info->my_WM_DELETE_WINDOW)
					{
						liqapp_log("x11.event.WM_DELETE_WINDOW");
						// deleting window
						// done=1;
						// Sat Aug 29 20:54:44 2009 lcuk : find much cleaner way to handle this
						exit(0);
						break;
					}
					liqapp_log("x11.event.ClientMessage");
					break;
					
						
				//############################################ Child lifetime events 
	
				case CreateNotify:
					liqapp_log("x11.event.CreateNotify");
					break;
				
				case DestroyNotify:
					liqapp_log("x11.event.DestroyNotify");
					break;			
				
				//############################################ Notify events: principle actors in arranging the window 
				
				case ConfigureNotify:
					// http://tronche.com/gui/x/xlib/events/window-state-change/configure.html
					// The X server can report ConfigureNotify events to clients wanting information about actual changes to a window's state,
					// such as size, position, border, and stacking order. 
					liqapp_log("x11.event.ConfigureNotify (%i,%i)-step(%i,%i) above=%d overrideredirect=%d",
							   xev.xconfigure.x,xev.xconfigure.y,xev.xconfigure.width,xev.xconfigure.height,
							   xev.xconfigure.above,xev.xconfigure.override_redirect);
					
					if( myx11info->myisvisibleflag==0 && myx11info->myisfocusflag==0)
					{
						break;
					}					
					if( myx11info->myisvisibleflag==0 && myx11info->myisfocusflag==1)
					{
						liqapp_log("x11.eep");
							if(myx11info->myoverlay)
							{				
								//liqapp_sleep(100);
								//xev.xconfigure.width
								liqx11overlay_close(myx11info->myoverlay);
								myx11info->myoverlay = NULL;
								//isbusyrendering=0;
							}
							if(!myx11info->myoverlay)
							{
							//	liqapp_sleep(100);
								myx11info->myoverlay = &myx11info->myoverlaycore;
								liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc,   canvas.pixelwidth,canvas.pixelheight);
								//isbusyrendering=0;
							}
							liqx11overlay_show(myx11info->myoverlay);
							myx11info->myisvisibleflag = 1;
							cover_image_release(myx11info);			
							ev->type = LIQEVENT_TYPE_RESIZE;
							break;
					}
					
					if( myx11info->myisvisibleflag==0 && myx11info->myisfocusflag==0)
					{
						myx11info->myisvisibleflag=1;
					}

				
					
					//if( !myx11info->myisvisibleflag ) break;
					
					if(xev.xconfigure.override_redirect) break;
					
					if(xev.xconfigure.window == myx11info->mywindow)
					{
						if(xev.xconfigure.event == myx11info->mywindow)
						{
							// perfect, its just us
						}
						else
							break;
					}
					else
						break;
					
					//liqapp_log("green shortcut"); break;
					
					//if( xev.xconfigure.above==0 || xev.xconfigure.above==myx11info->mywindow)
					//{
					//	myx11info->myisfocusflag=1;
					//}


					
					if( ( xev.xconfigure.width == liqcanvas_getwidth() ) &&
					    ( xev.xconfigure.height == liqcanvas_getheight() ) &&
						( myx11info->myoverlay ) 
					  )
					{
						//ev->type = LIQEVENT_TYPE_RESIZE;
						//break;
						// already at correct dimensions
					}
					else
					{
							
						if( myx11info->myisfocusflag )
						{
					
							if(myx11info->myoverlay)
							{				
								//liqapp_sleep(100);
								//xev.xconfigure.width
								liqx11overlay_close(myx11info->myoverlay);
								myx11info->myoverlay = NULL;
								//isbusyrendering=0;
							}
			
							if(!myx11info->myoverlay)
							{
							//	liqapp_sleep(100);
								myx11info->myoverlay = &myx11info->myoverlaycore;
								liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc,   canvas.pixelwidth,canvas.pixelheight);
								//isbusyrendering=0;
							}
						}
					}
					//dirty=1;
					
					
				if( myx11info->myisfocusflag )
				{
					
					liqx11overlay_show(myx11info->myoverlay);
					myx11info->myisvisibleflag = 1;
					cover_image_release(myx11info);
				}					
					ev->type = LIQEVENT_TYPE_RESIZE;
					// need to update the cr and stuff
	
					
					break;
				
				case ReparentNotify:
					liqapp_log("x11.event.ReparentNotify");
					break;
				
				case MapNotify:
					liqapp_log("x11.event.MapNotify");
					break;
				
				case MappingNotify:
					liqapp_log("x11.event.MappingNotify");
					break;				
				
				
				//############################################ Visibility handler
	
	
				case VisibilityNotify:
					liqapp_log("x11.event.VisibilityNotify state=%i",xev.xvisibility.state);
					myx11info->myisvisibleflag = (xev.xvisibility.state == 0) || (xev.xvisibility.state==1);
					if( myx11info->myisvisibleflag )
					{
					
						if((myx11info->myoverlay))
						{
							liqx11overlay_show(myx11info->myoverlay);
							liqx11overlay_refreshdisplay(myx11info->myoverlay);
							//isbusyrendering=1;
						}
						cover_image_release(myx11info);
						
						// visible so make this happen..
						//myx11info->myisfocusflag=1;
						// occurs when panout
					}
					else
					{
						if((myx11info->myoverlay))
						{
							cover_image_rebuild(myx11info);
							cover_image_blit(myx11info);
							liqx11overlay_hide(myx11info->myoverlay);
							//liqx11overlay_refreshdisplay(myx11info->myoverlay);
						}
						// not visible so force this..
						//myx11info->myisfocusflag=0;
						
					}
					break;				
		
				//############################################ focus
				case EnterNotify:
					//http://tronche.com/gui/x/xlib/events/window-entry-exit/
					
					liqapp_log("x11.event.EnterNotify focus=%d state=%d mode=%d detail=%d same_screen=%d window=%d subwindow=%d",
								xev.xcrossing.focus,
								xev.xcrossing.state,
								xev.xcrossing.mode,
								xev.xcrossing.detail,
								xev.xcrossing.same_screen,
								xev.xcrossing.window,
								xev.xcrossing.subwindow);
					
					/*
					if( (xev.xcrossing.focus==0) && (myx11info->myisfocusflag==0))
					{
						// we are potentially being brought back to life
						// strongly dislike all this on/off
						//liqx11info_regrab_focus(myx11info);
						break;
					}
					else
					{
						if( !myx11info->myisvisibleflag ) break;
					}
					*/

					if( myx11info->myisvisibleflag==0 && myx11info->myisfocusflag==0)
					{
						myx11info->myisvisibleflag=1;
						myx11info->myisfocusflag=1;
					}		
					
					if( !myx11info->myisvisibleflag ) break;
					
					//liqapp_log("green shortcut"); break;

                        if(!myx11info->myoverlay)
                        {
                        //	liqapp_sleep(100);
                            myx11info->myoverlay = &myx11info->myoverlaycore;
                            liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc,    canvas.pixelwidth,canvas.pixelheight);
                            //liqx11overlay_show(myx11info->myoverlay);
                            //isbusyrendering=0;
                        }
						
						if((myx11info->myoverlay))
						{
							liqx11overlay_show(myx11info->myoverlay);
							liqx11overlay_refreshdisplay(myx11info->myoverlay);
							//isbusyrendering=1;
							
							cover_image_release(myx11info);
						}
                        
                        liqx11info_regrab_focus(myx11info);
					break;				
		
				case LeaveNotify:
					liqapp_log("x11.event.LeaveNotify");
					
					if( !myx11info->myisvisibleflag ) break;
					
					//liqapp_log("green shortcut"); break;
					
					//XUnmapWindow(mydisplay,mywindow);
	
						if((myx11info->myoverlay))
						{
							liqx11overlay_hide(myx11info->myoverlay);
							liqx11overlay_refreshdisplay(myx11info->myoverlay);

							//liqx11overlay_show(myx11info->myoverlay);
							//liqx11overlay_refreshdisplay(myx11info->myoverlay);
                            //isbusyrendering=1;
						}
					break;				
	
				//############################################ focus
				case FocusIn:					
					liqapp_log("x11.event.FocusIn");
					//liqapp_log("green shortcut"); break;
					myx11info->myisfocusflag=1;
						if((myx11info->myoverlay))
						{
							liqx11overlay_show(myx11info->myoverlay);
							liqx11overlay_refreshdisplay(myx11info->myoverlay);
							//isbusyrendering=1;
						}
                        
                        liqx11info_regrab_focus(myx11info);
					break;				
		
				case FocusOut:
					liqapp_log("x11.event.FocusOut");
					myx11info->myisfocusflag=0;
	
						if((myx11info->myoverlay))
						{
							//liqx11overlay_hide(myx11info->myoverlay);
							//liqx11overlay_refreshdisplay(myx11info->myoverlay);

							//liqx11overlay_show(myx11info->myoverlay);
							//liqx11overlay_refreshdisplay(myx11info->myoverlay);
							//isbusyrendering=1;
						}
						
					break;				
				
				//############################################ render
				
				case Expose:
					liqapp_log("x11.event.expose count=%i         //render()",xev.xexpose.count);
					//liqapp_log("green shortcut"); break;
					
					cover_image_blit(myx11info);
					
					if (xev.xexpose.count == 0)
					{
						
						if(!myx11info->myoverlay)
						{
							myx11info->myoverlay = &myx11info->myoverlaycore;
							liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc,    canvas.pixelwidth,canvas.pixelheight);
							liqx11overlay_show(myx11info->myoverlay);
						}
						
						
						
						
				
						//if(xev.xexpose.count>0) goto foo;	
						{
						//	liqapp_log("event: Expose");
							ev->type = LIQEVENT_TYPE_EXPOSE;
						}


						//dirty=1;
					}
					break;







				//############################################ keyboard
				
                
                
				case KeyPress:
					//liqapp_log("keypre in  mymod=%i prev=%i",liqx11_modifier,liqx11_modifierprev);
					{
						// first, get the unmodified key
						int origkeysym = XLookupKeysym((XKeyEvent*)&xev,0);
						// find out if we need to adjust the mask
						int newmask=0;
						switch (origkeysym)
						{
							case XK_Shift_L:			newmask |= ShiftMask;
														break;
													
							case XK_Control_L:			newmask |= ControlMask;
														break;
													
							case XK_ISO_Level3_Shift:	newmask |= Mod5Mask;
														break;
						}
                        
						if(newmask)
						{
							//liqapp_log("modmod mod mask  %i",newmask);
							// actively pressing a modifier key - right now!
							liqx11_modifier |= newmask;
							// check if its the same as one we just pressed a moment ago..
							if(liqx11_modifierprev & newmask)
							{
								
								// we already have this bit set, lets make sure we toggle sticky
								if(liqx11_modifier & liqx11_modifier_StickyMask)
								{
									liqx11_modifier &= ~liqx11_modifier_StickyMask;
									// we were sticky
									// we must really clear the bit
									liqx11_modifier &= ~newmask;
								}
								else
								{
									liqx11_modifier |= liqx11_modifier_StickyMask;
								}
							}
							else
							{
								// not seen before, lets make sure its included
								liqx11_modifier |= newmask;
								// also, must REMOVE the sticky bit, incase we pressed multiple modifiers
								liqx11_modifier &= ~liqx11_modifier_StickyMask;							
							}
						}
						else
						{
							// not pressing a modifier!
							if(liqx11_modifierprev)
							{
								// actively holding a modifier
								// we dont actually need to set anything, it should already be done
							//}
							//else
							//{
							//	if(liqx11_modifierprev)
							//	{
									// we JUST pressed one before tho...
									// lets act like maemo and use it
									((XKeyEvent*)&xev)->state = (liqx11_modifierprev & ~liqx11_modifier_StickyMask);
							//	}
							}
						}
					}
					//liqapp_log("keypre out mymod=%i prev=%i",liqx11_modifier,liqx11_modifierprev);
					
	
					
					
					

					
					


			
					//if(!myx11info->myinnotifyflag) goto foo;
			
					//((XKeyEvent*)&xev)->state = 128;
			
			
					
					ev->type = LIQEVENT_TYPE_KEY;
					ev->key.state = LIQEVENT_STATE_PRESS;
					ev->key.keycode = XLookupKeysym((XKeyEvent*)&xev,0); //xev.xkey.keycode;
					ev->key.keystring[0]=0;
					ev->key.keymodifierstate = ((XKeyEvent*)&xev)->state;
					
					
					
					keylen = XLookupString((XKeyEvent*)&xev,ev->key.keystring,16, (KeySym *) NULL, (XComposeStatus *) NULL);
					liqapp_log("xNextEvent keypress %i, %i",ev->key.keycode,((XKeyEvent*)&xev)->state);
			
			
			
			
						// right in the core i can save a screen when i press the [Fullscreen] key
						if( (ev->type == LIQEVENT_TYPE_KEY) && (ev->state==LIQEVENT_STATE_PRESS) && (ev->key.keycode==65475) )	//FullScreen
						{
							int liqcanvas_takepicture();
							liqcanvas_takepicture();
						}
						
						break;
				
				
				case KeyRelease:
					
					
					//liqapp_log("keyrel in  mymod=%i prev=%i",liqx11_modifier,liqx11_modifierprev);
					
					liqx11_modifierprev = liqx11_modifier;

					{
						// first, get the unmodified key
						int origkeysym = XLookupKeysym((XKeyEvent*)&xev,0);
						// find out if we need to adjust the mask
						int newmask=0;
						switch(origkeysym)
						{
							case XK_Shift_L:			newmask |= ShiftMask;
														break;
													
							case XK_Control_L:			newmask |= ControlMask;
														break;
													
							case XK_ISO_Level3_Shift:	newmask |= Mod5Mask;
														break;
						}
						if(newmask)
						{
							// we are releasing a modifier
							if(liqx11_modifier & liqx11_modifier_StickyMask)
							{
								// we should leave it alone
							}
							else
							{
								// clear this bit from the global modifier
								liqx11_modifier &= ~newmask;						
							}
						}
						else
						{
							// normal key
							if(liqx11_modifier)
							{
								if(liqx11_modifier & liqx11_modifier_StickyMask)
								{
									// hold the modifier
								}
								else
								{
									// time to drop the modifier
									liqx11_modifier=0;
								}
							}
						}
					}
					//liqapp_log("keyrel out mymod=%i prev=%i",liqx11_modifier,liqx11_modifierprev);
					
					
					
					
					
						
						if(!myx11info->myinnotifyflag) goto foo;
						
						ev->type = LIQEVENT_TYPE_KEY;
						ev->key.state = LIQEVENT_STATE_RELEASE;
						ev->key.keycode =XLookupKeysym((XKeyEvent*)&xev,0); //xev.xkey.keycode;
						ev->key.keystring[0]=0;
						ev->key.keymodifierstate = ((XKeyEvent*)&xev)->state;
						keylen = XLookupString((XKeyEvent*)&xev,ev->key.keystring,16, (KeySym *) NULL, (XComposeStatus *) NULL);
						liqapp_log("xNextEvent keyrelease %i",ev->key.keycode);
						
						
						break;
				
				
				
				case 65: // if (xev.type == 65 || xev.type == 77)  // XShmCompletionEvent
				case 69:
				case 76:
				case 77:
						liqapp_log("x11.event.XShmCompletionEvent");
						//isbusyrendering=0;
						ev->type = LIQEVENT_TYPE_REFRESHED;
						break;	
			
				
				
				default:



						
						
						#ifdef USE_XSP
						
							
						
						
							if (xev.type == xsp_event_base)  // RAWMOUSE
							{
								
								//if(!xv_canvas_inNotify) goto foo;
								
								
								XSPRawTouchscreenEvent xsp_event;
								memcpy(&xsp_event, &xev,sizeof(XSPRawTouchscreenEvent));
						
								//liqapp_log("RawMouse %i,%i,%i",xsp_event.x,xsp_event.y,xsp_event.pressure);
								int t_x = xsp_event.x;
								int t_y = xsp_event.y;
						
								/* translate raw coordinates */
								TRANSLATE_RAW_COORDS(&t_x, &t_y,canvas.pixelwidth,canvas.pixelheight);
								if(t_x>=canvas.pixelwidth )t_x=canvas.pixelwidth -1;
								if(t_y>=canvas.pixelheight)t_y=canvas.pixelheight-1;
								if(t_x<0)t_x=0;
								if(t_y<0)t_y=0;
						
								ev->type = LIQEVENT_TYPE_MOUSE;
								ev->mouse.state = LIQEVENT_STATE_MOVE;
								ev->mouse.x = ((t_x));//*canvas.scalew);
								ev->mouse.y = ((t_y));//*canvas.scaleh);
								ev->mouse.pressure = xsp_event.pressure;
								break;
						
							}	
							
						#else // USE_XSP
						
						
							if(xev.type==ButtonPress)
							{
						//#ifndef USE_MAEMO	
						//		if(!myx11info->myinnotifyflag) goto foo;
						//#endif
						
								int xres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->width;
								int yres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->height;
							
						
								
								myx11info->myispressedflag=1;
								
                if(canvas.fullscreen)
                {
								xev.xmotion.x= xev.xmotion.x * canvas.pixelwidth  / xres;
								xev.xmotion.y= xev.xmotion.y * canvas.pixelheight / yres;
                }			
								//liqapp_log("buttonpress  %i,%i",xev.xmotion.x,xev.xmotion.y);
								//liqapp_log("ButtonPress");
								ev->type = LIQEVENT_TYPE_MOUSE;
								ev->mouse.state = LIQEVENT_STATE_PRESS;
								ev->mouse.x = ((xev.xmotion.x));
								ev->mouse.y = ((xev.xmotion.y));
								ev->mouse.pressure = 100;
								break;
							}
							else if(xev.type==MotionNotify)
							{
								if(!myx11info->myispressedflag)goto foo;
						//#ifndef USE_MAEMO
						//		if(!myx11info->myinnotifyflag) goto foo;
						//#endif
								int xres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->width;
								int yres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->height;
								
                                
                //                int qx=xev.xmotion.x;
                //                int qy=xev.xmotion.y;
                if(canvas.fullscreen)
                {
								xev.xmotion.x= xev.xmotion.x * canvas.pixelwidth  / xres;
								xev.xmotion.y= xev.xmotion.y * canvas.pixelheight / yres;
                }		 
				//				liqapp_log("motion %i,%i (q %i,%i) (can %i,%i)",xev.xmotion.x,xev.xmotion.y,qx,qy,canvas.pixelwidth,canvas.pixelheight);
								ev->type = LIQEVENT_TYPE_MOUSE;
								ev->mouse.state = LIQEVENT_STATE_MOVE;
								ev->mouse.x = ((xev.xmotion.x));// *800)/canvas.pixelwidth;
								ev->mouse.y = ((xev.xmotion.y));// *480)/canvas.pixelheight;
								ev->mouse.pressure = 200;
								break;
							}
							else if(xev.type==ButtonRelease)
							{
						//#ifndef USE_MAEMO		
						//		if(!myx11info->myinnotifyflag) goto foo;
						//#endif		
								int xres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->width;
								int yres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->height;
								
								myx11info->myispressedflag=0;
								
                if(canvas.fullscreen)
                {
								xev.xmotion.x= xev.xmotion.x * canvas.pixelwidth  / xres;
								xev.xmotion.y= xev.xmotion.y * canvas.pixelheight / yres;
                }		 
								//liqapp_log("release %i,%i",xev.xmotion.x,xev.xmotion.y);
						
								//liqapp_log("ButtonRelease");
								ev->type = LIQEVENT_TYPE_MOUSE;
								ev->mouse.state = LIQEVENT_STATE_RELEASE;
								ev->mouse.x = ((xev.xmotion.x));
								ev->mouse.y = ((xev.xmotion.y));
								ev->mouse.pressure = 0;
								break;
							}
							
							
						#endif	
							
							else
							{






								liqapp_log("Unknown event %i, let me know what happened at liquid@gmail.com",xev.type);
								
								break;
							}
						break;

				
				
			}
	
	
//liqapp_log("event completed :) returning type %i",ev->type);
	
	
	return 0;

}










