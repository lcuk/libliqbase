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



//############################################################# 

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
    if( !XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev) )
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
	
	
	
	
	
	
	liqapp_log("x11info preparing baselines and hint");

	XSizeHints 		myhint;
	unsigned long 	myforeground;
	unsigned long 	mybackground;
	
		mybackground  = BlackPixel(myx11info->mydisplay, myx11info->myscreen);
		myforeground  = WhitePixel(myx11info->mydisplay, myx11info->myscreen);
		
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
	
	
		XEvent event;
		do 
		{
			XNextEvent(myx11info->mydisplay, &event);
		}
		while (event.type != MapNotify || event.xmap.event != myx11info->mywindow);
	
	
	if(fullscreen)
		x11_set_fullscreen_state(myx11info->mydisplay,myx11info->mywindow,1);
	//isfullscreen=1;	
	
	
	//################################################# put the overlay in place :)
	

	liqapp_log("x11info allocating overlay");
	
	myx11info->myoverlay = &myx11info->myoverlaycore;
	liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc);
	liqx11overlay_show(myx11info->myoverlay);
	liqx11overlay_refreshdisplay(myx11info->myoverlay);


    liqx11info_regrab_focus(myx11info);


	
	return 0;
}
	

	
	
	
	
	
	
	
	
int liqx11info_close(liqx11info *myx11info)
{
	
	
	
	liqapp_log("shutting down");
	
	

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
//	return XPending(myx11info->mydisplay);

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
	return liqx11overlay_refreshdisplay(myx11info->myoverlay);
}







//############################################################# 
//############################################################# liqcanvas_nextevent_x11 
//############################################################# 
//############################################################# 














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
	
	
	
	
foo:

	//liqx11info_eventgetnext();

	//XNextEvent(dpy,&xev);
	
	
	liqx11info_eventgetnext( myx11info, &xev );
	
	
	
	int keylen=0;
	//liqapp_log("xNextEvent %i",xev.type);
	
	
	
	
	
			switch (xev.type)
			{					
	
	
				case ClientMessage:
					if (xev.xclient.data.l[0] == myx11info->my_WM_DELETE_WINDOW)
					{
						liqapp_log("event.WM_DELETE_WINDOW");
						// deleting window
						// done=1;
						// Sat Aug 29 20:54:44 2009 lcuk : find much cleaner way to handle this
						exit(0);
						break;
					}
					liqapp_log("event.ClientMessage");
					break;
					
						
				//############################################ Child lifetime events 
	
				case CreateNotify:
					liqapp_log("event.CreateNotify");
					break;
				
				case DestroyNotify:
					liqapp_log("event.DestroyNotify");
					break;			
				
				//############################################ Notify events: principle actors in arranging the window 
				
				case ConfigureNotify:
					// http://tronche.com/gui/x/xlib/events/window-state-change/configure.html
					// The X server can report ConfigureNotify events to clients wanting information about actual changes to a window's state,
					// such as size, position, border, and stacking order. 
					liqapp_log("event.ConfigureNotify");
					
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
						liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc);
						liqx11overlay_show(myx11info->myoverlay);
						//isbusyrendering=0;
					}
					//dirty=1;
					
					
					ev->type = LIQEVENT_TYPE_RESIZE;
					
					
					// need to update the cr and stuff
	
					
					break;
				
				case ReparentNotify:
					liqapp_log("event.ReparentNotify");
					break;
				
				case MapNotify:
					liqapp_log("event.MapNotify");
					break;
				
				case MappingNotify:
					liqapp_log("event.MappingNotify");
					break;				
				
				
				//############################################ Visibility handler
	
	
				case VisibilityNotify:
					liqapp_log("event.VisibilityNotify state=%i",xev.xvisibility.state);
					
					myx11info->myisvisibleflag = (xev.xvisibility.state == 0) || (xev.xvisibility.state==1);
						
					if((myx11info->myoverlay))
					{
						liqx11overlay_show(myx11info->myoverlay);
						liqx11overlay_refreshdisplay(myx11info->myoverlay);
						//isbusyrendering=1;
					}
					break;				
		
				//############################################ focus
				case EnterNotify:					
					liqapp_log("event.EnterNotify");

                        if(!myx11info->myoverlay)
                        {
                        //	liqapp_sleep(100);
                            myx11info->myoverlay = &myx11info->myoverlaycore;
                            liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc);
                            //liqx11overlay_show(myx11info->myoverlay);
                            //isbusyrendering=0;
                        }
						
						if((myx11info->myoverlay))
						{
							liqx11overlay_show(myx11info->myoverlay);
							liqx11overlay_refreshdisplay(myx11info->myoverlay);
							//isbusyrendering=1;
						}
                        
                        liqx11info_regrab_focus(myx11info);
					break;				
		
				case LeaveNotify:
					liqapp_log("event.LeaveNotify");
					
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
					liqapp_log("event.FocusIn");
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
					liqapp_log("event.FocusOut");
					myx11info->myisfocusflag=0;
	
						if((myx11info->myoverlay))
						{
							//liqx11overlay_hide(myx11info->myoverlay);
							//liqx11overlay_refreshdisplay(myx11info->myoverlay);

							liqx11overlay_show(myx11info->myoverlay);
							liqx11overlay_refreshdisplay(myx11info->myoverlay);
							//isbusyrendering=1;
						}
						
					break;				
				
				//############################################ render
				
				case Expose:
					liqapp_log("event.expose count=%i         //render()",xev.xexpose.count);
					if (xev.xexpose.count == 0)
					{
						
						if(!myx11info->myoverlay)
						{
							myx11info->myoverlay = &myx11info->myoverlaycore;
							liqx11overlay_init(myx11info->myoverlay, myx11info->mydisplay,myx11info->myscreen,myx11info->mywindow,myx11info->mygc);
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
						//liqapp_log("event.XShmCompletionEvent");
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
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	if(xev.type==KeyPress)
	{
		
		if(!myx11info->myinnotifyflag) goto foo;



		
		ev->type = LIQEVENT_TYPE_KEY;
		ev->key.state = LIQEVENT_STATE_PRESS;
		ev->key.keycode = XLookupKeysym((XKeyEvent*)&xev,0); //xev.xkey.keycode;
		ev->key.keystring[0]=0;
		
		keylen = XLookupString((XKeyEvent*)&xev,ev->key.keystring,16, (KeySym *) NULL, (XComposeStatus *) NULL);
		liqapp_log("xNextEvent keypress %i",ev->key.keycode);




			// right in the core i can save a screen when i press the [Fullscreen] key
			if( (ev->type == LIQEVENT_TYPE_KEY) && (ev->state==LIQEVENT_STATE_PRESS) && (ev->key.keycode==65475) )	//FullScreen
			{
				char 		fmtnow[255];
	 			liqapp_formatnow(fmtnow,255,"yyyymmdd_hhmmss");
				char buf[FILENAME_MAX+1];
				int pngerr =0;
				
				
				liqimage *imgfrom=NULL;//liqcamera_getimage();
				if(!imgfrom)
				{
					// camera not on, we are doing desktop
					imgfrom = canvas.surface;
				}
				if(imgfrom)
				{

				/*		if(imgfrom != canvas.surface)
						{
							// save camera image
							
							snprintf(buf,FILENAME_MAX,"%s/liq.%s.%s.cam.png",app.sketchpath,fmtnow,app.username  );
							liqapp_log("Fullscreen Pressed, saving camera  as '%s'",buf);
							pngerr=liqimage_pagesavepng(imgdesk,buf);
						}
						else
				*/
						{
							// save screenshot
							snprintf(buf,FILENAME_MAX,"liq.%s.%s.scr.png",fmtnow,"lib"  );
							liqapp_log("Fullscreen Pressed, saving canvas as '%s'",buf);
							pngerr=liqimage_pagesavepng(imgfrom,buf);
						}
						
				}
				else
				{
					// no canvas
					liqapp_log("Fullscreen Pressed, no canvas");
				}
			}
	}
	
	
	
	else if(xev.type==KeyRelease)
	{
		
		if(!myx11info->myinnotifyflag) goto foo;
		
		ev->type = LIQEVENT_TYPE_KEY;
		ev->key.state = LIQEVENT_STATE_RELEASE;
		ev->key.keycode =XLookupKeysym((XKeyEvent*)&xev,0); //xev.xkey.keycode;
		ev->key.keystring[0]=0;
		keylen = XLookupString((XKeyEvent*)&xev,ev->key.keystring,16, (KeySym *) NULL, (XComposeStatus *) NULL);
		liqapp_log("xNextEvent keyrelease %i",ev->key.keycode);
	}





#ifdef USE_XSP

	


	else if (xev.type == xsp_event_base)  // RAWMOUSE
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

	}	
	
#else // USE_XSP


	else if(xev.type==ButtonPress)
	{
//#ifndef USE_MAEMO	
//		if(!myx11info->myinnotifyflag) goto foo;
//#endif

		int xres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->width;
		int yres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->height;
	

		
        myx11info->myispressedflag=1;
        
        xev.xmotion.x= xev.xmotion.x * canvas.pixelwidth  / xres;
        xev.xmotion.y= xev.xmotion.y * canvas.pixelheight / yres;
        
        //liqapp_log("buttonpress  %i,%i",xev.xmotion.x,xev.xmotion.y);
		//liqapp_log("ButtonPress");
		ev->type = LIQEVENT_TYPE_MOUSE;
		ev->mouse.state = LIQEVENT_STATE_PRESS;
		ev->mouse.x = ((xev.xmotion.x));
		ev->mouse.y = ((xev.xmotion.y));
        ev->mouse.pressure = 100;
	}
	else if(xev.type==MotionNotify)
	{
		if(!myx11info->myispressedflag)goto foo;
//#ifndef USE_MAEMO
//		if(!myx11info->myinnotifyflag) goto foo;
//#endif
		int xres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->width;
		int yres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->height;
		
        xev.xmotion.x= xev.xmotion.x * canvas.pixelwidth  / xres;
        xev.xmotion.y= xev.xmotion.y * canvas.pixelheight / yres;
 
        //liqapp_log("motion %i,%i",xev.xmotion.x,xev.xmotion.y);
		ev->type = LIQEVENT_TYPE_MOUSE;
		ev->mouse.state = LIQEVENT_STATE_MOVE;
		ev->mouse.x = ((xev.xmotion.x));// *800)/canvas.pixelwidth;
		ev->mouse.y = ((xev.xmotion.y));// *480)/canvas.pixelheight;
        ev->mouse.pressure = 200;
	}
    else if(xev.type==ButtonRelease)
	{
//#ifndef USE_MAEMO		
//		if(!myx11info->myinnotifyflag) goto foo;
//#endif		
		int xres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->width;
		int yres   = ScreenOfDisplay (myx11info->mydisplay, DefaultScreen(myx11info->mydisplay))->height;
		
        myx11info->myispressedflag=0;
        
        xev.xmotion.x= xev.xmotion.x * canvas.pixelwidth  / xres;
        xev.xmotion.y= xev.xmotion.y * canvas.pixelheight / yres;
 
        //liqapp_log("release %i,%i",xev.xmotion.x,xev.xmotion.y);

		//liqapp_log("ButtonRelease");
		ev->type = LIQEVENT_TYPE_MOUSE;
		ev->mouse.state = LIQEVENT_STATE_RELEASE;
		ev->mouse.x = ((xev.xmotion.x));
		ev->mouse.y = ((xev.xmotion.y));
        ev->mouse.pressure = 0;
	}
	
	
#endif	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
    
 	else if (xev.type == 25)  // ResizeRequest
	{
		// obtain new dimensions and adjust our renderer to suit   
	}
	else if (xev.type == 65 || xev.type == 69 || xev.type == 77|| xev.type == 77)  // XShmCompletionEvent
	{
		//http://www.x.org/docs/Xv/video		
		//	XvShmPutImage has a parameter:
		//   send_event - Indicates whether or not an XShmCompletionEvent should be
		//                sent.  If sent, the event's major_code and minor_code
		//                fields will indicate the Xv extension's major code and
		//                XvShmPutImage's minor code.
		//if(myx11info->myinnotifyflag)
		{
		
			ev->type = LIQEVENT_TYPE_REFRESHED;
		}
		//else
		//{
		//	// actually no point in telling the caller that we have finished refreshing :)
		//	// lets leave him waiting for now
		//}
		
	}
	

	else if (xev.type == EnterNotify) // EnterNotify
	{
		liqapp_log("event: EnterNotify");
		//XRaiseWindow(dpy,window);
		myx11info->myinnotifyflag = 1;
	}
	
	

	else if (xev.type == LeaveNotify) // LeaveNotify
	{
		liqapp_log("event: LeaveNotify");
		//XRaiseWindow(dpy,window);
		myx11info->myinnotifyflag = 0;
	}
	
	
	
	
	
	


	else if (xev.type == VisibilityNotify) // VisibilityNotify
	{
		liqapp_log("event: VisibilityNotify %i",xev.xvisibility.state);
		//XRaiseWindow(dpy,window);
	}
	
	
	
	
	else if (xev.type == Expose) // expose
	{
		//XRaiseWindow(dpy,window);
		//liqcanvas_refreshdisplay();
		if(xev.xexpose.count>0) goto foo;


		{
			liqapp_log("event: Expose");
			ev->type = LIQEVENT_TYPE_EXPOSE;
		}
		
	}
	
	else if (xev.type == MappingNotify) // mapping notify
	{
		liqapp_log("event: MappingNotify");
		//XRaiseWindow(dpy,window);
	}
	
	else if (xev.type == MapNotify) // map notify
	{
		liqapp_log("event: MapNotify");
		//XRaiseWindow(dpy,window);
	}
	
	else if (xev.type == UnmapNotify) // unmap notify
	{
		liqapp_log("event: UnmapNotify");
		//XRaiseWindow(dpy,window);
	}
	else if (xev.type == ClientMessage) // client message
	{
		liqapp_log("event: ClientMessage");
		//XRaiseWindow(dpy,window);
	}	
	
	else if (xev.type == 77) // ..
	{
		//liqapp_log("event: ...");
		//XRaiseWindow(dpy,window);
	}	
	
	else if (xev.type == 22) // ..
	{
		//liqapp_log("event: ...");
		//XRaiseWindow(dpy,window);
	}	

	else if (xev.type == 94)// random events which dont occur in maemo..
	{
	
	}
	else if (xev.type == 6)// random events which dont occur in maemo.. came at a mouse move..
	{
	
	}
	else
	{

	// x11 event definitions: http://www.openmash.org/lxr/source/xlib/X11/X.h?c=tk8.3#L99
	
	//		ev->type = LIQEVENT_TYPE_UNKNOWN;
			liqapp_log("Unknown event %i, let me know what happened at liquid@gmail.com",xev.type);
			//exit(-1);
	}

	return 0;

}










