/**
 * @file	liqcanvas_firstrun_splash.c
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



//########################################################################
//########################################################################
//########################################################################

/**
 * first run "splash screen" designed to be used in the middle of firstrun detection logic to automagically remove glitches :)
 * @return Success or Failure
 *
 */


// not yet in use

int liqcanvas_firstrun_splash()
{
	liqapp_log("#################################### liqcanvas_firstrun_splash");

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


int busyflag=1;

//liqcell *keyhit=NULL;

	while(running==1)
	{
		while(liqcanvas_eventcount())
		{
waitevent:

			liqcanvas_nextevent(&ev,   &busyflag  );
			
			liqapp_log("splash time ev %i %i", ev.type,framecount);
			
			if( (liqapp_GetTicks()-tzs) > 2000)
			{
				//liqapp_log("splash time completed %i",framecount);
				running=0;
				//refreshinprogress=0;
				break;				
			}
			
			if( (ev.type == LIQEVENT_TYPE_KEY) && (ev.state==LIQEVENT_STATE_PRESS) && (ev.key.keycode==65307) )	//ESC
			{
				liqapp_log("Escape Pressed, Cancelling");
				running=0;
				break;
			}



			else if( (ev.type == LIQEVENT_TYPE_KEY) )
			{
				// user has pressed a key
				dirty=1;
				break;
			}


			else if(ev.type == LIQEVENT_TYPE_MOUSE   )// && ev.mouse.pressure==0)
			{
				// mouse moving! w00t
				hadmouse=1;
				dirty=1;
				//break;
			}
			else if(ev.type == LIQEVENT_TYPE_EXPOSE)
			{
				//liqapp_log("event expose");
				refreshinprogress=0;
				wantwait=1;
				dirty=1;
				break;
			}

			else if(ev.type == LIQEVENT_TYPE_DIRTYFLAGSET && (refreshinprogress==0))
			{
				//liqapp_log("event dirty");
				wantwait=1;
				dirty=1;
				break;
			}
			else if(ev.type == LIQEVENT_TYPE_REFRESHED)
			{
				//liqapp_log("event refreshed");
				refreshinprogress=0;
				wantwait=1;
				break;
			}


			else if(ev.type == LIQEVENT_TYPE_NONE)
			{
				//liqapp_log("event none");
				wantwait=1;
				break;
			}
			else if(ev.type == LIQEVENT_TYPE_UNKNOWN)
			{
				//liqapp_log("event unknown");
				running=0;
				break;
			}
			else
			{
				// anything else, just ignore it
				wantwait=1;
			}
		}
		dirty=1;

		if(refreshinprogress==0) if(running==0) break;
		if(((dirty==1) && (refreshinprogress==0)))
		{
			liqapp_log("render");
			liqcliprect *cr=liqcanvas_getcliprect();
			if(cr)
			{
				liqapp_log("render2");
				liqcliprect_drawclear(cr,framecount % 255,128,128);
				liqapp_log("render3");
				liqcanvas_refreshdisplay();
				liqapp_log("render4");
			}
			framecount++;
			dirty=0;
			wantwait=1;
			refreshinprogress=1;
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
	liqapp_log("liqcanvas_firstrun_splash complete %i",result);
	return result;
}
