/**
 * @file	liqcanvas.c
 * @author  Gary Birkett
 * @brief 	Basic Canvas control functions
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





#include <stdlib.h>                                                                                 

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>



#include "liqapp.h"
#include "liqimage.h"
#include "liqcanvas.h"
#include "liqcliprect.h"
//#include "liqcanvas_xv.h"



int liqcanvas_firstrun_splash();

#include "liqx11info.h"

liqx11info *  liqcanvas_getx11info();
int           liqcanvas_nextevent_x11(LIQEVENT *ev,int *dirtyflagptr);




//#include "liq_xsurface.h"			// include available workhorse functions


//todo: make this a class instance, we might end up having multiple canvases..
liqcanvas canvas={0,0};


liqx11info x11infobase={NULL};




liqx11info *  liqcanvas_getx11info()
{
	return (liqx11info*)canvas.x11info;
}







int liqcanvas_takepicture()
{
							char 		fmtnow[255];
							liqapp_formatnow(fmtnow,255,"yyyymmdd_hhmmss");
							//char buf[FILENAME_MAX+1];
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
										char 		fmtnow[255];
										liqapp_formatnow(fmtnow,255,"yyyymmdd_hhmmss");
										char buf[FILENAME_MAX+1];
										snprintf(buf,FILENAME_MAX,"liq.%s.%s.scr.png",fmtnow,"lib"  );
										liqapp_log("liqcanvas_takepicture, saving canvas as '%s'",buf);
										pngerr=liqimage_pagesavepng(imgfrom,buf);
									}
									
							}
							else
							{
								// no canvas
								liqapp_log("liqcanvas_takepicture, no canvas");
							}
							return 0;
}









int liqcanvas_settitle(char *newtitle)
{
    
    if(!liqcanvas_isopen())return -1;
    
    
    XSetStandardProperties(liqcanvas_getx11info()->mydisplay, liqcanvas_getx11info()->mywindow, newtitle,
            newtitle, None, NULL, 0, NULL);
}






int liqcanvas_isopen()
{
	if(canvas.pixelwidth || canvas.pixelheight)
	{
		return 1;
		
	}
	return 0;
}

int liqcanvas_init_usecustomwindow(	int pixelwidth,int pixelheight,void *customx11display,void *customx11window)
{
	//

	if(canvas.pixelwidth || canvas.pixelheight)
	{
		{ liqapp_log("canvas liqcanvas_xv_init already open"); }
		return -1;
		
	}
	canvas.customx11display=customx11display;
	canvas.customx11window =customx11window;
	return liqcanvas_init(pixelwidth,pixelheight,0);
}


/**
 * Construct a canvas with a given height and width, also with the option of going fullscreen.
 * @param pixelwidth Width of the canvas
 * @param pixelheight Height of the canvas
 * @param fullscreen Set this to 1 in order to have a fullscreen canvas
 * @return int 0 for success, -1 for error/failure
 */
int liqcanvas_init_inner(int pixelwidth,int pixelheight,int fullscreen)
{
	if(canvas.pixelwidth || canvas.pixelheight)
	{
		{ liqapp_log("canvas liqcanvas_xv_init already open"); }
		return -1;
		
	}
	
	canvas.x11info =(void*)&x11infobase;
	
	
	if(!fullscreen)
	{
		if( (strcasecmp("RX-34", liqapp_hardware_product_get() ) ==0) || (strcasecmp("RX-44", liqapp_hardware_product_get() ) ==0) )
		{
			// this is n8x0
			pixelwidth  = ((float)pixelwidth)  * ( (float)HILDON_APPVIEW_WIDTH  / 800.0  );
			pixelheight = ((float)pixelheight) * ( (float)HILDON_APPVIEW_HEIGHT / 480.0  );
			
		}
		else
		if(  (strcasecmp("RX-51", liqapp_hardware_product_get() ) ==0) )
		{
			pixelwidth  = ((float)pixelwidth)  * ( (float)HILDON_RX_51_APPVIEW_WIDTH  / 800.0  );
			pixelheight = ((float)pixelheight) * ( (float)HILDON_RX_51_APPVIEW_HEIGHT / 480.0  );
			
		}
		
		
	}
	

	canvas.keepalivealarmtime=10000;
	canvas.pixelwidth = pixelwidth;
	canvas.pixelheight = pixelheight;
	
	canvas.fullscreen = fullscreen;
	if((canvas.fullscreen==0) && (canvas.customx11window==0))
	{
		canvas.scalew=1;//(float)canvas.pixelwidth /(float)(HILDON_APPVIEW_WIDTH);		// this should depend upon the rotation
		canvas.scaleh=1;//(float)canvas.pixelheight/(float)(HILDON_APPVIEW_HEIGHT);
		
	}
	{
		canvas.scalew=1;//(float)canvas.pixelwidth /800.0;		// this should depend upon the rotation
		canvas.scaleh=1;//(float)canvas.pixelheight/480.0;
	}
	canvas.dpix=(canvas.scalew * 225.0);		// this should depend upon the rotation
	canvas.dpiy=(canvas.scaleh * 225.0);
	
	
	//if(liqcanvas_xv_init()!=0)
	//{
	//	{ liqapp_errorandfail(-1,"canvas liqcanvas_xv_init failed"); }
	//	return -1;
	//}
	
	
	if(liqx11info_init(liqcanvas_getx11info(), pixelwidth, pixelheight, fullscreen) != 0)
	{
		{ liqapp_errorandfail(-1,"canvas liqx11info_init failed"); }
		return -1;
	}

	



	
	//#################################################
	
	liqapp_log("Canvas.dpi %i,%i",canvas.dpix,canvas.dpiy);
	

	return 0;
}



/**
 * Construct a canvas with a given height and width, also with the option of going fullscreen.
 * @param pixelwidth Width of the canvas
 * @param pixelheight Height of the canvas
 * @param fullscreen Set this to 1 in order to have a fullscreen canvas
 * @return int 0 for success, -1 for error/failure
 */
int liqcanvas_init(int pixelwidth,int pixelheight,int fullscreen)
{
	if(canvas.pixelwidth || canvas.pixelheight)
	{
		{ liqapp_log("canvas liqcanvas_xv_init already open"); }
		return -1;
		
	}
	if(liqcanvas_init_inner(pixelwidth,pixelheight,fullscreen)!=0)
	{
		{ liqapp_errorandfail(-1,"canvas liqx11info_init failed"); }
		return -1;
	}

	xsurface_drawclear_grey(canvas.surface,0);
	
		// opened
	
	int firstrun=1;


	if( strcasecmp("RX-51", liqapp_hardware_product_get() ) ==0 ) firstrun=0;

	if(firstrun)
	{
		liqapp_log("canvas liqx11info_init firstrun splash location");
		
		liqcanvas_refreshdisplay();
		
		// 20090511_023718 lcuk : show a splash and then close display
		// 20090511_023726 lcuk : this *should* remove the first run glitch cos this will catch everything
		//liqcanvas_firstrun_splash();
		int i;
		LIQEVENT event;
		int dirty=0;
		for(i=0;i<5;i++)
		{
			liqapp_sleep(50);
			while(liqcanvas_eventcount()>0)  liqcanvas_nextevent(&event,&dirty);
		}
		liqcanvas_close();
		
		
		
		
		if(liqcanvas_init_inner(pixelwidth,pixelheight,fullscreen)!=0)
		{
			{ liqapp_errorandfail(-1,"canvas liqx11info_init failed 2"); }
			return -1;
		}
		
		// opened again!
		// 20090511_023909 lcuk : user hopefully wont have problems now :)
	}
	return 0;
		

}



int liqcanvas_close()
{
	if(!liqcanvas_isopen())
	{
		liqapp_log("canvas close : wasnt opened");
		
		return-1;
	}
	//liqapp_log("canvas close");
	//if(canvas.font) liqcanvas_closefont();
	//canvas.font=NULL;


	
	//liqcanvas_xv_close();
	liqx11info_close(liqcanvas_getx11info());

	canvas.customx11display=NULL;
	canvas.customx11window =NULL;


	//liqapp_log("canvas close end");


	canvas.pixelwidth =0;
	canvas.pixelheight=0;

	return 0;
}


liqcliprect * liqcanvas_getcliprect(		)
{
	return canvas.cr;
}



liqimage * liqcanvas_getsurface(		)
{
	return canvas.surface;
}

int liqcanvas_getwidth(		)
{
	return canvas.pixelwidth;
}

int liqcanvas_getheight(		)
{
	return canvas.pixelheight;
}

int liqcanvas_getdpix(		)
{
	return canvas.dpix;
}
int liqcanvas_getdpiy(		)
{
	return canvas.dpiy;
}
float liqcanvas_getscalew(		)
{
	return canvas.scalew;
}
float liqcanvas_getscaleh(		)
{
	return canvas.scaleh;
}


//##########################################################################

int liqcanvas_eventcount()
{
	//return liqcanvas_xv_eventcount();
	return liqx11info_eventgetcount( liqcanvas_getx11info() );
}

int liqcanvas_nextevent(LIQEVENT *ptrevent,int *dirtyflagptr)
{
	if(ptrevent==NULL)
	{
		return liqapp_errorandfail(-1,"liqcanvas_nextevent passed null event pointer");
	}
	//return liqcanvas_xv_nextevent(ptrevent,dirtyflagptr);
	//return liqcanvas_nextevent_x11(ptrevent,dirtyflagptr);

	//liqapp_log("liqcanvas_nextevent");

	int res =liqx11info_get_next_liqevent(liqcanvas_getx11info(),ptrevent,dirtyflagptr);
	//liqapp_log("liqcanvas_nextevent fin res=%i",res);
	return res;
}


int liqcanvas_refreshdisplay()
{
	//liqcanvas_xv_refreshdisplay();
	
	liqx11info_refreshdisplay( liqcanvas_getx11info() );
	
	canvas.framecount++;
	
	return 0;
}



int liqcanvas_minimize()
{
	//liqcanvas_xv_minimize();
	return 0;
}


//##########################################################################
//##########################################################################
//##########################################################################
//##########################################################################
//##########################################################################





















