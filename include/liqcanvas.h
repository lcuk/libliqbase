/* liqbase
 * Copyright (C) 2008 Gary Birkett
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
 */

/*
 *
 * Header for canvas control.  most of these functions are in flux, they are required but depreciated.
 *
 */



#ifndef LIQCANVAS_H
#define LIQCANVAS_H

#ifdef __cplusplus
extern "C" {
#endif


#include "liqfont.h"
#include "liqimage.h"
#include "liqcliprect.h"

typedef struct
{
	int pixelwidth;
	int pixelheight;

	int fullscreen;
	float scalew;
	float scaleh;
	int dpix;
	int dpiy;
	int keepalivealarmtime;
	//liqfont *font;
	liqimage *surface;		// these are now dynamically allocated and altered depending upon the x11 overlay actions
	liqcliprect *cr;		// there might be times when these are NULL.  always read and try to check..
							// they are removed, cleared and reset by a resize/rotation operation
							// technically we should also have the same operation altering the pixelwidthheight variables

	void *customx11display;	// this should be null if created by myself
	void *customx11window;

	void *actualx11display;	// this will always be live within an x11 framework
	void *actualx11window;
	
	void *x11info;
	
	unsigned int expansion[16];
	
} liqcanvas;

//#######################################################

// optional custom event wrappers

typedef struct
{
	unsigned int type;
	unsigned long ticks;
	unsigned int state;
	unsigned int button;
	unsigned int x;					// events are scaled to canvas pixels
	unsigned int y;
	unsigned int pressure;
}
	LIQEVENT_MOUSE;

typedef struct
{
	unsigned int type;
	unsigned long ticks;
	unsigned int state;
	unsigned int keycode;				// KeySym
	char keystring[16];					// not exactly sure if it will ever be needed
	//unsigned int keyascii;
	unsigned int keymodifierstate;		// holds the x11 ShiftMask ControlMask Mod5Mask etc
}
	LIQEVENT_KEY;

//LIQEVENT_STATE_WAKEUP
typedef union
{
	struct							// evtype 0,3,4
	{
		unsigned int type;
		unsigned long ticks;
		unsigned int state;
	};
	LIQEVENT_MOUSE mouse;			// evtype 1
	LIQEVENT_KEY key;				// evtype 2
}
	LIQEVENT;

#define LIQEVENT_TYPE_NONE    	 -1
#define LIQEVENT_TYPE_UNKNOWN    0
#define LIQEVENT_TYPE_MOUSE      1
#define LIQEVENT_TYPE_KEY        2
#define LIQEVENT_TYPE_EXPOSE     3
#define LIQEVENT_TYPE_REFRESHED  4
#define LIQEVENT_TYPE_DIRTYFLAGSET 5

#define LIQEVENT_TYPE_RESIZE     6

//#define LIQEVENT_TYPE_BUTTON     4
//#define LIQEVENT_STATE_KEEPALIVE 8

#define LIQEVENT_STATE_NONE      -1
#define LIQEVENT_STATE_UNKNOWN   0
#define LIQEVENT_STATE_PRESS     1
#define LIQEVENT_STATE_RELEASE   2
#define LIQEVENT_STATE_MOVE      4





#define HILDON_APPVIEW_HEIGHT          420
#define HILDON_APPVIEW_WIDTH           720


#define HILDON_RX_51_APPVIEW_HEIGHT          420
#define HILDON_RX_51_APPVIEW_WIDTH           800


//todo: make this a class instance, we might end up having multiple canvases..
extern liqcanvas canvas;

int           liqcanvas_isopen();

int liqcanvas_settitle(char *newtitle);

//#######################################################
// allocate using your own custom window
// i expect this to fail with multiple instances
// todo: this canvas was the first code i wrote and its ingrained in the system and needs cleaning out
int           liqcanvas_init_usecustomwindow(	int pixelwidth,int pixelheight,void *customx11display,void *customx11window);


int           liqcanvas_init(			int pixelwidth,int pixelheight,int fullscreen);
int           liqcanvas_eventcount();
int           liqcanvas_nextevent(		LIQEVENT *ptrevent,int *dirtyflagptr);
int           liqcanvas_refreshdisplay();

int           liqcanvas_close();

void          liqcanvas_clear(		unsigned char grey);
liqcliprect * liqcanvas_getcliprect();		// draw everything using this, dont touch these functions below
liqimage *    liqcanvas_getsurface();


int           liqcanvas_minimize();
int           liqcanvas_getwidth();
int           liqcanvas_getheight();
int           liqcanvas_getdpix();
int           liqcanvas_getdpiy();
float         liqcanvas_getscalew();
float         liqcanvas_getscaleh();
















#ifdef __cplusplus
}
#endif


#endif
