/**
 * @file	liqcell_easyrun.c
 * @author  Gary Birkett
 * @brief 	Run liqcell events
 * 
 * Copyright (C) 2008 Gary Birkett
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>


#include "liqbase.h"

#include "liqcell.h"
#include "liqcell_easyrun.h"
#include "liqcell_easypaint.h"
#include "vgraph.h"



//extern liqcell *universe;



extern int liqcell_showdebugboxes;



static void liqcellmouseeventargs_stroke_start(liqcellmouseeventargs *self,int mx,int my,int mz)
{
	unsigned long 	mt=liqapp_GetTicks();
	self->mcnt=1;

	self->msx=mx;
	self->msy=my;
	self->msz=mz;
	self->mst=mt;

	self->mdx=0;		// simply 0 for starting
	self->mdy=0;
	self->mdz=0;
	self->mdt=0;

	self->mex=mx;
	self->mey=my;
	self->mez=mz;
	self->met=mt;


	liqstroke_clear(self->stroke);
	liqstroke_start(self->stroke,mx,my,mz,mt);

	//liqapp_GetTicks();

}


static void liqcellmouseeventargs_stroke_extend(liqcellmouseeventargs *self,int mx,int my,int mz)
{
	unsigned long 	mt=liqapp_GetTicks();

	self->mcnt++;

	self->mdx=mx-self->mex;
	self->mdy=my-self->mey;
	self->mdz=mz-self->mez;
	self->mdt=mt-self->met;

	self->mex=mx;
	self->mey=my;
	self->mez=mz;
	self->met=mt;

	liqstroke_extend(self->stroke,mx,my,mz,mt);

}


	

//########################################################################
//########################################################################
//########################################################################

typedef struct vrect
{
	unsigned int usagecount;

	int x;
	int y;
	int w;
	int h;
}
	vrect;

int vrect_ensurepositive(vrect *self)
{
	// ensure that the width and height are positive, may involve adjusting the offsets
	if(self->w<0){ self->x=self->x-self->w; self->w=-self->w; }
	if(self->w<0){ self->y=self->y-self->h; self->h=-self->h; }
	return 0;
}

int vrect_scaletofit(vrect *self,vrect *avail,vrect *required)
{
	// set the scale
	if(self->w<0){ self->x=self->x-self->w; self->w=-self->w; }
	if(self->w<0){ self->y=self->y-self->h; self->h=-self->h; }
	return 0;
}





//########################################################################
//########################################################################
//########################################################################






float calcaspect(int captionw,int captionh,int availw,int availh)
{
	if(captionw==0)return 0;
	if(captionh==0)return 0;
	float ax = (float)availw / (float)captionw;
	float ay = (float)availh / (float)captionh;
	float ar = (ax<=ay ? ax : ay);
	return ar;

}





liqcell *liqcell_easyhittest(liqcell *self,  int mx,int my,int *hitx,int *hity)
{
	//	mx and my have already been located within the frame of self
//char buff[256];
//liqcell_getqualifiedname(self,buff,256);
	//liqapp_log("in '%s' : starting",buff);

	if(mx < self->x) return NULL;
	if(my < self->y) return NULL;
	mx-=self->x;
	my-=self->y;
	if(mx >= self->w) return NULL;
	if(my >= self->h) return NULL;
	// we are inside
	//liqapp_log("in '%s' : checking deeper",buff);
	// check the children now
	
	// 20090620_114104 lcuk : ensure hittest works from the end to the start so that latest things (which should be ontop) work
	// 20090620_114616 lcuk : this might actually break some existing layouts, but the constructor just needs tweaking
	// 20090620_114633 lcuk : to change the position of the buttons in use
	
	//liqcell *c=liqcell_getlinkchild(self);
	liqcell *c=liqcell_lastchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{
			liqcell *hit = liqcell_easyhittest(c, mx,my, hitx,hity);
			if(hit)
			{
				//liqapp_log("in '%s' : found good child, completed",buff);
				return hit;
			}
		}
		//c=liqcell_getlinknext(c);
		c=liqcell_getlinkprev(c);
	}
	*hitx=mx;
	*hity=my;
	return self;
}



static void savethumb(liqcell *cell)
{
	liqcell_hold(cell);
	// 20090528_231040 lcuk : this locks up dunno why
	// 20090624_005139 lcuk : trying it as a widget itself, to see if it was initializers at fault
	liqapp_log("...creating image %s",cell->name);
	liqimage *img = liqimage_newatsize(canvas.pixelwidth,canvas.pixelheight,0);
	
	liqapp_log("...creating cliprect");
	
	liqcliprect *cr = liqcliprect_newfromimage(img);
	
	liqapp_log("...painting cell %s",cell->name);
	liqcell_easypaint(cell,cr,0,0,canvas.pixelwidth,canvas.pixelheight);
	
	liqapp_log("...building filename");

				char 		fmtnow[255];
	 			liqapp_formatnow(fmtnow,255,"yyyymmdd_hhmmss");
				char buf[FILENAME_MAX+1];
				snprintf(buf,FILENAME_MAX,"%s/sketches/liq.%s.%s.scr.png",app.userdatapath,fmtnow,cell->name  );




	
	liqapp_log("...saving image as '%s'",buf);

				liqimage_pagesavepng(img,buf);
	


//01:49:32 png writing png
//01:49:32 png cleaning up
//01:49:32 ...releasing cr
//01:49:32 liqcliprect free
//01:49:32 liqimage free
//01:49:32 liqimage pagereset
//01:49:32 ...releasing image
//01:49:32 ...done
// 20090624_015023 lcuk : a bug is occuring, the liqimage instance is being freed too early
// 20090624_015040 lcuk : that means something is releasing it within the middle of something else
// 20090624_015052 lcuk : but did not get hold of it first
	
				
	
	liqapp_log("...releasing cr");
	liqcliprect_release(cr);
	
	liqapp_log("...releasing image");
	liqimage_release(img);
	
	liqapp_log("...done");
	
	liqcell_release(cell);
	
}



	static int toolitem_click(liqcell *self, liqcellclickeventargs *args, liqcell *tool)
	{
		//args->newdialogtoopen = liqcell_getcontent( self );
		liqcell_setvisible(tool,0);
		return 1;
	}

	static int tool_help_click(liqcell *self, liqcellclickeventargs *args, liqcell *tool)
	{
		return 1;
	}
	static int tool_bug_click(liqcell *self, liqcellclickeventargs *args, liqcell *tool)
	{
		return 1;
	}
	static int tool_tag_click(liqcell *self, liqcellclickeventargs *args, liqcell *tool)
	{
		return 1;
	}
	
	static int tool_pic_click(liqcell *self, liqcellclickeventargs *args, liqcell *tool)
	{
		liqcell * content = liqcell_child_lookup(tool,"content");
		savethumb( liqcell_getcontent(content) );
		liqcell_setvisible(tool,0);
		//int res = liqdialog_showtree("quick view","view of cell contents","",liqcell_getcontent(content) );
		return 1;
	}
	
	static int tool_pin_click(liqcell *self, liqcellclickeventargs *args, liqcell *tool)
	{
		return 1;
	}


liqcell * toolclick(liqcell *vis)
{
	liqcell *self = liqcell_quickcreatewidget("tools","form", 800,480);

	if(self)
	{

		// need a top bar

		//liqcell_child_insert( self, uititlebar_create("top","liqbase playground","welcome to your world") );

	

		//############################# icon:label
		liqcell *icon = liqcell_quickcreatevis("icon", "label", 6, 8, 52, 40);
		//liqcell_setfont(	icon, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (12), 0) );
		//liqcell_setcontent(icon, vis );
		//liqcell_setcaption(icon, "icon" );
		//liqcell_propsets(  icon, "textcolor", "rgb(255,255,255)" );
		//liqcell_propsets(  icon, "backcolor", "rgb(0,0,0)" );
		//liqcell_propsets(  icon, "bordercolor", "rgb(200,100,100)" );
		//liqcell_propseti(  icon, "textalign", 2 );
		liqcell_child_append(  self, icon);
		
		//############################# title:label
		liqcell *title = liqcell_quickcreatevis("title", "label", 66, 8, 728, 40);
		liqcell_setfont(	title, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (34), 0) );
		liqcell_setcaption(title, liqcell_getcaption(vis) );
		liqcell_propsets(  title, "textcolor", "rgb(255,255,255)" );
		liqcell_propsets(  title, "backcolor", "rgb(0,0,0)" );
		liqcell_propseti(  title, "textalign", 0 );
		liqcell_child_append(  self, title);	




		liqcell *b;
		
		int hh = 480-64;

		b = liqcell_quickcreatevis("help","button",  800-50,64+hh*0.0,   50,+hh*0.2 );
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd_withcontext(b,    "click",   tool_help_click, self);
		liqcell_propsets(  b,    "backcolor", "rgb(0,100,0)" );
		liqcell_child_insert( self, b );



		b = liqcell_quickcreatevis("draw","button",  800-50,64+hh*0.2,   50,hh*0.2 );
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd_withcontext(b,    "click",   tool_bug_click, self);
		liqcell_propsets(  b,    "backcolor", "rgb(100,0,0)" );
		liqcell_child_insert( self, b );
	
		
		b = liqcell_quickcreatevis("tag","button",  800-50,64+hh*0.4,   50,hh*0.2);
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd_withcontext(b,    "click",   tool_tag_click, self);
		liqcell_propsets(  b,    "backcolor", "rgb(0,100,100)" );
		liqcell_child_insert( self, b );		




		b = liqcell_quickcreatevis("pic","button",  800-50,64+hh*0.6,   50,hh*0.2);
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd_withcontext(b,    "click",   tool_pic_click, self);
		liqcell_propsets(  b,    "backcolor", "rgb(100,100,0)" );
		liqcell_child_insert( self, b );




		b = liqcell_quickcreatevis("save","button",  800-50,64+hh*0.8,   50,hh*0.2 );
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd_withcontext(b,    "click",   tool_pin_click, self);
		liqcell_propsets(  b,    "backcolor", "rgb(0,0,100)" );
		liqcell_child_insert( self, b );







		liqcell *c;
		
		
/*		c = liqcell_quickcreatevis("contentdraw", "liqtop", 0,56,   800-50,480-56 );
		liqcell_handleradd_withcontext( c,    "click",   toolitem_click,self);
		liqcell_child_insert( self, c );
*/	
		c = liqcell_quickcreatevis("content", NULL, 0,56,   800-50,480-56 );
		liqcell_setcontent( c, vis );
		liqcell_setenabled( c, 0 );		// make sure it has fadeout
		//liqcell_handleradd_withcontext( c,    "click",   toolitem_click,self);
		liqcell_child_insert( self, c );


	}
	//liqcell_easyrun(self);
	return self;
}


//	liqcell *self=
//}










































liqcell *liqcell_findnexthandler(liqcell*self, liqcell*root,char *handlername)
{
	// 20090520_011358 lcuk : todo, fix this 
		liqcell *c=liqcell_getlinkchild(self);
		while(c && liqcell_getvisible(c)  )
		//if(c && liqcell_getvisible(c)  )
		{
			if(liqcell_handlerfind(c,handlername))
			{
				return c;
			}
			liqcell *d = liqcell_findnexthandler(c,root,handlername);
			if(d) return d;
			c=liqcell_getlinknext(c);
		}
		
		if(self==root)return NULL;
		//if(liqcell_getlinknext(self))
		//	return liqcell_getlinknext(self);
		
		return NULL;//liqcell_getlinkparent(self);

}

liqcell *liqcell_findfirsthandler(liqcell*root,char *handlername)
{
			if(liqcell_handlerfind(root,handlername))
			{
				return root;
			}

	// 20090520_011358 lcuk : todo, fix this 
		liqcell *c=liqcell_getlinkchild(root);
		while(c && liqcell_getvisible(c)  )
		//if(c && liqcell_getvisible(c)  )
		{
			if(liqcell_handlerfind(c,handlername))
			{
				return c;
			}
			liqcell *d = liqcell_findnexthandler(c,root,handlername);
			if(d) return d;
			c=liqcell_getlinknext(c);
		}
		return NULL;

}









// 20090702_192535 lcuk : very quickly, the zoom in the middle of this needs replacing
// 20090702_192547 lcuk : it needs to have an enter from parameter rect
// 20090702_192602 lcuk : this should indicate in screen coordinates where the item is to start from
// 20090702_192626 lcuk : during the course of the next Nth of a second the cell spends coming to rest from that point
// 20090702_192721 lcuk : after completing, it should deblur in a similar manner.
// 20090702_192752 lcuk : a disolve effect would be magical tonight..
// 20090702_192827 lcuk : this completely solves the problem of the zoom in the core of this function







//########################################################################
//########################################################################
//########################################################################

int liqcell_easyrun_depth=0;



/**
 * Main program event handler.
 * @param self The liqcell to run
 * @return Success or Failure
 *
 */
int liqcell_easyrun(liqcell *self)
{
	liqapp_log("#################################### liqcell easyrun (%i,%i) :: %s",self->w,self->h,self->name);
	if(liqcell_showdebugboxes)
		liqcell_print2(self);
	if(self->w==0 || self->h==0)
	{
		liqapp_log("liqcell easyrun cannot continue, cell size must be >0");
		return -1;
	}
	
	liqcell_easyrun_depth++;
	
	// what i should do is resize the contents to match the frame
	// - actually, that may happen more than once per session
	liqcell_handlerrun(self,"dialog_open",NULL);
	
	
	vgraph *graph = vgraph_new();
	////vgraph_setscaleaspectlock(graph,  0);
	//vgraph_setscaleaspectlock(graph,  1);
	//vgraph_setcliprect(graph, liqcanvas_getcliprect() );
	//vgraph_settarget(graph,   liqcanvas_getsurface()  );
	//vgraph_setwindow( graph,  (self)                  );
	
	
liqimage    *targetsurface = NULL;//liqcanvas_getsurface();
liqcliprect *targetcr      = NULL;//liqcanvas_getcliprect();




	void easyrun_realtime_reshape()
	{
		//vgraph_setscaleaspectlock(graph,  0);
		vgraph_setscaleaspectlock(graph,  1);
		vgraph_setcliprect(graph, NULL );
		vgraph_settarget(graph,   NULL  );
		
		vgraph_setcliprect(graph, liqcliprect_hold(liqcanvas_getcliprect()) );
		vgraph_settarget(graph,   liqimage_hold(liqcanvas_getsurface())  );
		vgraph_setwindow( graph,  (self)                  );
		
		
		targetsurface = liqcanvas_getsurface();
		targetcr      = liqcanvas_getcliprect();
		
	}

	easyrun_realtime_reshape();



int 			running=1;
int 			result=0;

unsigned long 	tzs=liqapp_GetTicks();
unsigned long 	tz0=liqapp_GetTicks();
unsigned long 	tz1=liqapp_GetTicks();
LIQEVENT 		ev;
int 			framecount=0;
int 			dirty=1;		// ensure we are drawn at least once :)
int 			wantwait=0;
int 			hadmouse=0;
int 			refreshinprogress=0;
unsigned long 	refreshstarttime=0;		// if we have a refresh in progress




liqcellpainteventargs paintargs;
	//paintargs.cr = liqcanvas_getcliprect();
	paintargs.graph = graph;
	paintargs.runfast=0;

liqcellmouseeventargs mouseargs;
	//mouseargs.cr = liqcanvas_getcliprect();
	mouseargs.graph = graph;
	mouseargs.mcnt=0;
	mouseargs.hit = NULL;

	mouseargs.stroke = liqstroke_new();


liqcellkeyeventargs keyargs;
liqcellclickeventargs clickargs;
	clickargs.newdialogtoopen = NULL;

int wx=0;
int wy=0;


int omsx=0;
int omsy=0;
int omex=0;
int omey=0;

int 			zoom_in_progress=0;
unsigned long 	zoom_start=0;
void *			zoom_app=NULL;
int             zoom_direction=0;
int zw=0;
int zh=0;
int zx=0;
int zy=0;

int hotx=0;
int hoty=0;
liqcell *hot=NULL;

	//liqcell_setstroke(self, liqstroke_hold(mouseargs.stroke));
	
	// try to find the first keyhandler :)
liqcell *keyhit=liqcell_findfirsthandler(self,"keypress");




		// 20090215:gb: hmmm, flicker problem
		// serious graphical overwriting and flicker ensue when fullspeed refreshing when idle mainly
		// (fullspeed refreshing is drawing a series of live scenes as fast as possible only waiting for refreshed event)
		// but often with other random stuff
		// sometimes however its rock solid stable and works perfectly
		// is x11 native update interupting the stream from xv i wonder

		// 20090215:gb: ok, this got weird quickly
		// in the event handler above, I have a LIQEVENT_TYPE_REFRESHED event
		// which comes internally from the XShmCompletionEvent event raised sometime after calling XvShmPutImage()
		// if I register the completion above and loop back round into while(liqcanvas_eventcount())
		// then the flickering occurs.
		// if I add a "break;" to the specific Refreshed handler and exit the loop without
		// calling liqcanvas_eventcount() again until i have already drawn the screen
		// then there is no flicker.
		// VERY strange, following into liqcanvas_eventcount() now ...
		// this calls liqcanvas_xv_eventcount() to directly handle the maemo events
		// which as its only code line is: int evc=XEventsQueued(dpy, QueuedAfterFlush);
		// is this line telling the system they CAN run now, yealding, ie a "doevents()" ????
		// a process is then starting and we are colliding for use of the framebuffer
		// there is no spinlock in interface use.

		// marked as solved for future reference

		// this is the basic event loop:

		// while(running)
		// {
		//		while(XEventsQueued())
		// 		{
		// 			// handle events
		//			XNextEvent()
		// 			case MOUSE/KEY etc
		//			case XShmCompletionEvent
		// 				oktodraw=true;
		// 				break;                         <<< adding or removing this caused or removed the flicker
		// 		}
		// 		if(oktodraw)
		// 		{
		//			DrawFrame();
		//	 	  	XvShmPutImage();
		//		}
		// }


liqcell *jumpprev=NULL;
liqcell *jumpnext=NULL;

	liqcell *rr=liqcell_getlinkprev(self);
	while(rr)
	{
		if(liqcell_getvisible(rr))
		{
			jumpprev = rr;
			break;
		}
		rr=liqcell_getlinkprev(rr);
	}
	rr=liqcell_getlinknext(self);
	while(rr)
	{
		if(liqcell_getvisible(rr))
		{
			jumpprev = rr;
			break;
		}
		rr=liqcell_getlinknext(rr);
	}
	
int jumpdir=0;		// -1== to prev,  1=tonext

	while(running==1)
	{
		//liqapp_log("Runloop %i  ud=%i",framecount,universe->dirty);
		hadmouse=0;
		while(liqcanvas_eventcount())// && (liqcellcount>0))
		{
waitevent:
			//goto skipev;


			//if(self->dirty) liqcell_setdirty(self,0);
			//if(universe->dirty) liqcell_setdirty(universe,0);


			//liqcell_setdirty(universe,1);

			//liqapp_log("Evtloop framecount=%i",framecount);
			//liqcanvas_nextevent(&ev,   &self->dirty  );


			//if( refreshinprogress)
			{
			//	liqcanvas_nextevent(&ev,  NULL);// &universe->dirty  );
			}
			//else
			{
				//liqcanvas_nextevent(&ev,   &universe->dirty  );
				liqcanvas_nextevent(&ev,   &self->dirty  );
			}
			
			//liqapp_log("Evtloop got ev.type = %i ",ev.type);



			//liqcanvas_nextevent(&ev,   NULL  );
			//todo: upon hearing about a blanking signal, we should automatically switch ourselves to a slow update
			if( (ev.type == LIQEVENT_TYPE_KEY) && (ev.state==LIQEVENT_STATE_PRESS) && (ev.key.keycode==65307) )	//ESC
			{
				liqapp_log("Escape Pressed, Cancelling");
				running=0;
				//result=NULL;
				break;
			}



			else if( (ev.type == LIQEVENT_TYPE_KEY) )
			{
				// user has pressed a key
				// do we have a current edit component?
				// if not we should look for one
				// failing that we should open up a search box...
				if(!keyhit)
				{
					// search for a cell with a key handler
				}
				if(keyhit)
				{
					// depending upon the key we should handle it...
					//liqapp_log("hello?");

					keyargs.keycode = ev.key.keycode;

					strncpy(keyargs.keystring,ev.key.keystring, 16 );

					keyargs.ispress=(ev.state==LIQEVENT_STATE_PRESS);

					if(ev.state==LIQEVENT_STATE_PRESS)
					{

								liqcell *vhit=keyhit;
								while(vhit && (liqcell_handlerfind(vhit,"keypress")==NULL)  )
								{
									vhit=liqcell_getlinkparent(vhit);
								}
								if(vhit)
								{
									if( liqcell_handlerrun(vhit,"keypress",&keyargs) )
									{
									}
								}


						//if( liqcell_handlerrun(keyhit,"keypress",&keyargs) )
						//{
						//}
					}
					else
					{
								liqcell *vhit=keyhit;
								while(vhit && (liqcell_handlerfind(vhit,"keyrelease")==NULL)  )
								{
									vhit=liqcell_getlinkparent(vhit);
								}
								if(vhit)
								{
									if( liqcell_handlerrun(vhit,"keyrelease",&keyargs) )
									{
									}
								}
						//if( liqcell_handlerrun(keyhit,"keyrelease",&keyargs) )
						//{
						//}
					}
				}
				dirty=1;
				break;
			}


			else if(ev.type == LIQEVENT_TYPE_MOUSE && (zoom_in_progress==0)   )// && ev.mouse.pressure==0)
			{
				// mouse moving! w00t

				// get hold of actual coordinates...
				int mx=ev.mouse.x;
				int my=ev.mouse.y;
			//	liqapp_log("event mouse scrn (%i,%i)",mx,my);


				// convert to be in the context of a cell
				wx=0;
				wy=0;

				vgraph_convert_target2window(graph ,mx,my,  &wx,&wy);
				//liqapp_log("mouse scrn (%i,%i)   cell (%i,%i)",mx,my,  wx,wy);

				hotx=0;
				hoty=0;




			if(mouseargs.mcnt==0)
			{
				hot = liqcell_easyhittest(self, wx,wy, &hotx,&hoty);
				//hot = hit;//liqcell_easyhittest(self, wx,wy, &hotx,&hoty);
				//liqcell *sel = hot;
				//if(hot)
				//{
					//char buff[256];
					//liqcell_getqualifiedname(hot,buff,256);

					//liqapp_log("mouse cell m(%3i,%3i) w(%3i,%3i) h(%3i,%3i) : '%s'",mx,my,wx,wy,hotx,hoty,buff);

					// 20090317_0033 lcuk : do not change wxy now, leave at original offsets
					//wx=hotx;
					//wy=hoty;
				//}
				//else
				//{

				//	liqapp_log("mouse miss m(%3i,%3i) w(%3i,%3i) h(%3i,%3i) : '%s'",mx,my,wx,wy,hotx,hoty,"NULL");
				//}
			}
			
			


				// 20090317_0032 lcuk : the stroke is entirely based on original coordinates
				if(ev.mouse.pressure!=0)
				{
					if(mouseargs.mcnt==0)
					{
						// starting
						liqcellmouseeventargs_stroke_start(&mouseargs,wx,wy,ev.mouse.pressure);

						mouseargs.hit = hot;




						//if(hot) liqcell_zorder_totop(hot);


						keyhit = hot;
						
						
						omsx=mx;
						omsy=my;
						omex=mx;
						omey=my;

					}
					else
					{
						// in progress
						liqcellmouseeventargs_stroke_extend(&mouseargs,wx,wy,ev.mouse.pressure);

						omex=mx;
						omey=my;

					}
				}
				else
				{
						// completed
						liqcellmouseeventargs_stroke_extend(&mouseargs,wx,wy,ev.mouse.pressure);

						omex=mx;
						omey=my;


				}


							if( ((targetsurface->width-omsx)<64) && (omsy<64) && ((targetsurface->width-omex)<64) && (omey<64) )
							{
								if(ev.mouse.pressure!=0)
								{
									hot=NULL;
								}
								else
								{
									
									// tools button :)
									

									
									if(strcasecmp( liqcell_getname(self), "tools" )==0)
									{
										// 20090606_013753 lcuk : if we are already in a tools object, close it again :)
										running=0;
										goto quickfin;
										
									}

											// 20090606_003219 lcuk : mark these as clear and shortcut the process
											mouseargs.mcnt=0;
											mouseargs.hit = NULL;

											hot=self;
											hotx=0;
											hoty=0;

											zoom_app=toolclick(self);
											zoom_in_progress=1;
											zoom_start=liqapp_GetTicks();
											zoom_direction = 1;

									goto quickfin;


								}
								
							}


							if( (omsx<64) && ((targetsurface->height-omsy)<64) && (omex<64) && ((targetsurface->height-omey)<64) )
							{
								if(ev.mouse.pressure!=0)
								{
									hot=NULL;
								}
								else
								{
									running=0;
									goto quickfin;
								}
								
							}






				if(hot)// && hot==mouseargs.hit)
				{
					// still using the right area :)

					//if( liqcell_handlerrun(hot,"mouse",&mouseargs) )
					//{
					//}
						//liqapp_log("mouse test '%s'",hot->name);

						{
							int vx=hotx;
							int vy=hoty;
							liqcell *vhit=hot;
							while(vhit && (liqcell_handlerfind(vhit,"mouse")==NULL)  )
							{
								//liqapp_log("mouse skip  '%s'",vhit->name);

								vx-=vhit->x;
								vy-=vhit->y;
								vhit=liqcell_getlinkparent(vhit);
							}
							if(vhit)
							{
								//liqapp_log("mouse run  '%s'",vhit->name);

								//##########################################
								// get absolute offset (make this a cell_fn?) must stop when it gets to the dialog item itself though
								int ox=0;
								int oy=0;
								liqcell *ohit=vhit;
								while(ohit && ohit!=self)
								{
									ox+=ohit->x;
									oy+=ohit->y;
									ohit=liqcell_getlinkparent(ohit);
								}

								mouseargs.ox=ox;
								mouseargs.oy=oy;

								if( liqcell_handlerrun(vhit,"mouse",&mouseargs) )
								{
									// handled it \o/
									if(self->visible)
									{
										// refresh display
										// sleep
									}
								}
							}
						}


					if(ev.mouse.pressure==0)
					{


							
						// check if we need to send click
						//if(  (liqcell_handlerfind(self,"mouse")==NULL) || (liqstroke_totallength(mouseargs.stroke) < 25)   )
						if(  (liqstroke_totallength(mouseargs.stroke) < 50)) //25)   )
						{
							
							char buff[256];
							liqcell_getqualifiedname(hot,buff,256);
							liqapp_log("click test '%s'",buff);//hot->name);

							clickargs.newdialogtoopen=NULL;


							{
								liqcell *vhit=hot;
								while(vhit && (liqcell_handlerfind(vhit,"click")==NULL)  )
								{
									vhit=liqcell_getlinkparent(vhit);
								}
								if(vhit)
								{
									liqapp_log("click run '%s'",vhit->name);
									if( liqcell_handlerrun(vhit,"click",&clickargs) )
									{
										// handled it \o/
										if(clickargs.newdialogtoopen)
										{
											//
											//liqcell_easyrun( clickargs.newdialogtoopen );
											// we should technically run the transitions here first

											zoom_app=clickargs.newdialogtoopen;
											zoom_in_progress=1;
											zoom_start=liqapp_GetTicks();
											zoom_direction = 1;
										}


										wantwait=0;
										//refreshinprogress=0;
										dirty=1;
										hadmouse=1;

									}
								}
							}
						}
					}

				}
				else
				{
					//mouseargs.mcnt=0;
				}

				if(ev.mouse.pressure==0)
				{
					// we should now make sure the mouse handler object is marked as completed
					mouseargs.mcnt=0;
					mouseargs.hit = NULL;
					//hot=NULL;
				}
quickfin:
				hadmouse=1;
				dirty=1;
				//break;
			
			}
			else if(ev.type == LIQEVENT_TYPE_EXPOSE)
			{
				liqapp_log("event expose");
				// ok, we want to be exposed
				refreshinprogress=0;
				wantwait=1;
				dirty=1;
				break;
			}

			else if(ev.type == LIQEVENT_TYPE_DIRTYFLAGSET && (refreshinprogress==0))
			{
				liqapp_log("event dirty");
				
				// ok, we want to be exposed
				//refreshinprogress=0;
				wantwait=1;
				dirty=1;

				//liqcell_setdirty(universe,0);  //->sirty=0;
				break;
			}
			else if(ev.type == LIQEVENT_TYPE_REFRESHED)
			{
				//liqapp_log("event refreshed");
				// ok, we have finished the refresh
				refreshinprogress=0;

//#ifndef USE_MAEMO
				wantwait=1;
//#endif
				break;
			}
			
			else if(ev.type == LIQEVENT_TYPE_RESIZE)
			{
				liqapp_log("event resize");
				
				easyrun_realtime_reshape();
				
				refreshinprogress=0;
				wantwait=0;
				dirty=1;
				break;
			}

			else if(ev.type == LIQEVENT_TYPE_NONE)
			{
				liqapp_log("event none");
				// just move on
				//refreshinprogress=0;
//#ifndef USE_MAEMO
				wantwait=1;
//#endif
				break;
			}
			else if(ev.type == LIQEVENT_TYPE_UNKNOWN)
			{
				liqapp_log("event unknown");
				running=0;
				break;
			}

			else
			{
				// anything else, just ignore it
//#ifndef USE_MAEMO
				wantwait=1;
//#endif
				//break;
			}
		}

skipev:


		if(refreshinprogress==0 && self->visible==0)break;



		float zoom_duration = 0.15;//0.4;//0.01;//0.1;//0.2;	// time to go from fullscreen to zoomed in
		if((zoom_in_progress) && (hot) && (refreshinprogress==0))
		{
			float zoomruntime = (liqapp_GetTicks()-zoom_start) / (1000.0);
			float zoomfactor =  zoomruntime / zoom_duration;

			if(zoomfactor >= 1 || zoomfactor<0)
			{
				// finished zooming now.

				if(zoom_direction==1)
				{

					//zoom_in_progress=0;
					// run the zoom_app :)

					liqcell_easyrun( zoom_app );

					zoom_app = NULL;

					zoom_direction = -1;
					zoom_start=liqapp_GetTicks();
					zoomruntime=zoom_duration * 0.1; // 20090410_135220 lcuk : this 0.1 is wrong when zoom_duration is a long time, but right with low durations
					zoomfactor=0.1;
					goto moar;
				}
				else
				{
					// totally finished now..
					zoom_in_progress=0;
				}
			}
			else
			{
moar:
				if(zoom_direction==-1) zoomfactor = 1-zoomfactor;
				//int hisa = calcaspect(hot->w,hot->h, self->w,self->h);
				//int hisw = hot->w * hisa;
				//int hish = hot->h * hisa;
				float hfx=1;//(float)hisw / (float)self->w;
				float hfy=1;//(float)hish / (float)self->h;
				// we are some fraction between viewing the whole screen and being zoomed directly on hot
				//
				zw = self->w + (float)self->w * zoomfactor * (((float)self->w / (float)hot->w * hfx)-1 );
				zh = self->h + (float)self->h * zoomfactor * (((float)self->h / (float)hot->h * hfy)-1 );
				int rx=0;
				int ry=0;
				liqcell *r=hot;
				while(r)
				{
					if(r==self) break;
					rx+=r->x;
					ry+=r->y;
					r=r->linkparent;
				}
				zx = -(float)rx * (zoomfactor) * (((float)self->w / (float)hot->w * hfx));
				zy = -(float)ry * (zoomfactor) * (((float)self->h / (float)hot->h * hfy));

				//liqapp_log("self(%i,%i)   hot(%i,%i)-step(%i,%i)   z(%i,%i)-step(%i,%i)",
				//					self->w,self->h,
				//					hot->x,hot->y, hot->w,hot->h,
				//					zx,zy,zw,zh);
			}

			dirty=1;
		}
		if(self->dirty && (refreshinprogress==0))// && (dirty==0))
		{
			dirty=1;
			self->dirty=0;
		}
		if(refreshinprogress==0) if(running==0) break;
		if(paintargs.runfast) dirty=1;
		if(((dirty==1) && (refreshinprogress==0)))
		{
		//	liqapp_log("render %i  ud=%i",framecount,universe->dirty);
			//liqapp_log("rendering %i",framecount);
			//liqcliprect_drawclear(liqcanvas_getcliprect(),255,128,128);
			liqcliprect_drawclear(liqcanvas_getcliprect(),0,128,128);
			// ensure runfast is unset before attmpting the next loop
			paintargs.runfast=0;
			//##################################################### render handler
			// do whatever we want..
			liqcell_handlerrun(self,"paint",&paintargs);
			float fac = 1;
			fac=1;
			int w=(((float)self->w)*fac);
			int h=(((float)self->h)*fac);
			int x=0;//self->x;//-w/2;
			int y=0;//self->y;//-h/2;
			if(zoom_in_progress)
			{
				w=zw;
				h=zh;
				x=zx;
				y=zy;
			}
			if(mouseargs.mcnt>0)
			{
			//	w=ev.mouse.x;
			}
			//liqapp_log("render drawing wh(%i,%i)",w,h);
			vgraph_drawcell(graph,x,y,w,h,self);
			//liqapp_log("render adding nav items");
			static liqimage *infoback=NULL;
			static liqimage *infoclose=NULL;
			static liqimage *infotools=NULL;
			// 20090614_213546 lcuk : now, i know where i am installed, i can use that path hopefully
			if(!infoback)
			{
				//liqapp_log("************************************************************************************** read");
				infoback = liqimage_cache_getfile("/usr/share/liqbase/media/quickicons/back.png", 0,0,1);
			}
			if(!infoclose)
			{
				//liqapp_log("************************************************************************************** read");
				infoclose = liqimage_cache_getfile("/usr/share/liqbase/media/quickicons/gtk-close.png", 0,0,1);
			}
			if(!infotools)
			{
				//liqapp_log("************************************************************************************** read");
				infotools = liqimage_cache_getfile("/usr/share/liqbase/media/quickicons/package_system.png", 0,0,1);
			}
			if(infoback && infoclose)
			{
				//liqapp_log("************************************************************************************** use");
				if(liqcell_easyrun_depth==1)
					liqcliprect_drawimagecolor(targetcr, infoclose, 0,targetsurface->height-48,48,48, 1);
				else
				
					liqcliprect_drawimagecolor(targetcr, infoback , 0,targetsurface->height-48,48,48, 1);
			}
			if( infotools )
			{
					liqcliprect_drawimagecolor(targetcr, infotools , targetsurface->width-48,0 ,48,48, 1);
			}
 		
			
			//liqapp_log("render adding framecount");		
// 20090520_014021 lcuk : show frame information
/*
			static liqfont *infofont=NULL;
			if(!infofont)
			{
				infofont = liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", 20, 0);
			}
			if(0==liqfont_setview(infofont, 1,1 ))
			{
				char *cap=liqcell_getcaption(self);
				if(!cap || !*cap) cap="[nameless]";
				char buff[255];
				snprintf(buff,sizeof(buff),"liqbase '%s' %3i, %3.3f, %3.3f",cap,framecount, liqapp_fps(tz0,tz1,1) ,liqapp_fps(tzs,tz1,framecount) );
				//liqapp_log(buff);
				int hh=liqfont_textheight(infofont);
				liqcliprect_drawtextinside_color(targetcr, infofont,  0,0, targetsurface->width,hh, buff,0, 255,128,128);
			}		
 */		
			//liqapp_log("render refreshing");
			liqcanvas_refreshdisplay();
			//liqapp_log("render done");
			framecount++;
			dirty=0;

// Sun Apr 05 16:49:47 2009 lcuk : kots x86 machine does not send back refresh events
// needs this for now until we work out why
//#ifndef USE_MAEMO
			wantwait=1;
			refreshinprogress=1;
//#endif
// Mon Apr 06 01:45:51 2009 lcuk : damn, it flickers, we need time to sort this properly in the new handler

			refreshstarttime=liqapp_GetTicks();
			tz0=tz1;
			tz1=liqapp_GetTicks();
		}
		if(refreshinprogress)
		{
			if( (liqapp_GetTicks()-refreshstarttime) > 1000)
			{
				// we have been waiting to refresh for ages now, we should stop trying
				// most likely because we went to another screen and it ate our event
				refreshinprogress=0;
				wantwait=0;
				dirty=1;
			}
			else
			{
				// carry on waiting, no point rushing
				wantwait=1;
			}
		}
		if(wantwait || refreshinprogress)
		{
			wantwait=0;
			goto waitevent;
		}
	}
	liqapp_log("liqcell easyrun complete %i",result);
	vgraph_release(graph);
	liqstroke_release(mouseargs.stroke);
	liqcell_handlerrun(self,"dialog_close",NULL);
	liqcell_easyrun_depth--;
	return result;
}
