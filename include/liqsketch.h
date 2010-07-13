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
 * Header for basic sketch
 *
 */
#ifndef LIQSKETCH_H
#define LIQSKETCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liqimage.h"

//struct liqtile;


struct liqcell;	// 20090422_194042 lcuk : todo - sort out this cycling dependency


//##################################################################

typedef struct liqpoint
{
	unsigned int usagecount;
	struct liqpoint *linkprev;
	struct liqpoint *linknext;
	int           x;
	int           y;
	int           z;	
	unsigned long t;
} liqpoint;

//liqpoint         *liqpoint_alloc();
//void          liqpoint_new(liqpoint *self);
//void          liqpoint_free(liqpoint *self);

liqpoint * 	liqpoint_new();
liqpoint * 	liqpoint_hold(liqpoint *self);
void    		liqpoint_release(liqpoint *self);
void    		liqpoint_free(liqpoint *self);




liqpoint         *liqpoint_clone(liqpoint *s);
void   liqpoint_copy(liqpoint *self,liqpoint *s) ;
void   liqpoint_getdiff(liqpoint *self,liqpoint *s,liqpoint *e) ;
int    liqpoint_issame(liqpoint *self,liqpoint *s);

//##################################################################

typedef struct liqpointrange
{
	int xl;
	int yt;
	int xr;
	int yb;
	int zf;
	int zb;
} liqpointrange;
void          liqpointrange_start(liqpointrange *self,liqpoint *p);
void          liqpointrange_extendrubberband(liqpointrange *self,liqpoint *p);
void          liqpointrange_start_xyz(liqpointrange *self,int px,int py,int pz);
void          liqpointrange_extendrubberband_xyz(liqpointrange *self,int px,int py,int pz);

int           liqpointrange_isconnected(liqpointrange *self,liqpointrange *b);
//##################################################################
typedef struct liqstroke 
{
	unsigned int usagecount;
	struct liqstroke *linkprev;
	struct liqstroke *linknext;
	
	struct liqsketch   *linkpage;
	
	unsigned char pen_y;		// todo we can pack these nowadays ;)
	unsigned char pen_u;		// infact, we can style them :)
	unsigned char pen_v;		// hmmm..
	unsigned char pen_thick;
	
	int           strokekind;	// 0=normal stroke, 1=p2p line, 2=box, 3=areafill
	
	short 		  selected;
	short         islandnumber;
	
	char *		  mediakey;		// filename used as xref for image or subtile.  when selected the file should be moved into the project folder
	// media is the key into another page
	
	struct liqsketch   *mediapage;	// this is NOT freed here, it is YOUR job to handle it
	
	// normalization involves scaling to unit
	char   		  *quadchain;	// the results of normalizing the stroke and building a quadchain list
	
	int           pointcount;
	liqpoint         *pointfirst;
	liqpoint         *pointlast;
	liqpointrange    boundingbox;
	

} liqstroke;


liqstroke * 	liqstroke_new();
liqstroke * 	liqstroke_hold(liqstroke *self);
void    		liqstroke_release(liqstroke *self);
void    		liqstroke_free(liqstroke *self);


//liqstroke        *liqstroke_alloc();
//void          liqstroke_new(liqstroke *self);
//void          liqstroke_free(liqstroke *self);


void          liqstroke_clear(liqstroke *self);

liqstroke        *liqstroke_clone(liqstroke *s);
void 		  liqstroke_appendpoint(liqstroke *self,liqpoint *p);

void          liqstroke_start(liqstroke *self,int px,int py, int pz,unsigned long pt);
void          liqstroke_extend(liqstroke *self,int px,int py, int pz,unsigned long pt);
int           liqstroke_totallength(liqstroke *self);
void          liqstroke_ensurepositive(liqstroke *self);
int           liqstroke_hittest(liqstroke *self,int px,int py);
char *        liqstroke_quadchainbuild(liqstroke *self);


//##################################################################
typedef struct liqsketch 
{
	unsigned int 		usagecount;
	//void *     			owner;
	
	int           		pixelwidth;
	int           		pixelheight;

	int           		dpix;
	int           		dpiy;
	
	char          *		title;
	char          *		filename;
	int 		  		islandcount;
	
	int           		strokecount;
	liqstroke        *	strokefirst;
	liqstroke        *	strokelast;
	//liqstroke        *strokecurrent;
	liqpointrange    	boundingbox;
	
	
	int           		backgroundstyle;		// 0 none, 1=solid,2=image,3=subsketch..
	unsigned int  		backgroundcoloryuv;
	char *		  		backgroundfilename;
	liqimage *	  		backgroundimage;
	struct liqsketch *	backgroundsketch;
	
	int                 angle;					// render rotation angle to use

	
	//struct liqcell *		extratokens;			// a set of tokens which did not match the normal defs, loaded and saved with the file			
	

} liqsketch;




liqsketch 	* 	liqsketch_newfromfile(char *filename);

liqsketch * 	liqsketch_new();
liqsketch * 	liqsketch_hold(liqsketch *self);
void    		liqsketch_release(liqsketch *self);
void    		liqsketch_free(liqsketch *self);



void          liqsketch_titlechange(liqsketch *self,char *title);

void          liqsketch_strokeinsert(liqsketch *self,liqstroke *s);
void          liqsketch_strokeupdate(liqsketch *self,liqstroke *s);
void          liqsketch_strokeremove(liqsketch *self,liqstroke *s);
void          liqsketch_clear(liqsketch *self);

int 		  liqsketch_filesave(liqsketch *self,char *filename);
int 		  liqsketch_fileload(liqsketch *self,char *filename);
int           liqsketch_fileload_memstream(liqsketch *self,char *filename,char *srcdata, int srcsize);

void          liqsketch_boundwholearea(liqsketch *self);

void 		  liqsketch_coordchange_scr_to_page(liqsketch *self,int scrx,int scry,int scrw,int scrh, int scrdpix,int scrdpiy,int *rx,int *ry);
void 		  liqsketch_islandcalcall(liqsketch *self);		// identify the islands within this page :)

#ifdef __cplusplus
}
#endif


#endif
