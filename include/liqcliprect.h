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
 * Header for the cliprect library.  this is the prefered way to interact with graphics.
 *
 */



#ifndef LIQCLIPRECT_H
#define LIQCLIPRECT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "liqfont.h"
#include "liqsketch.h"
#include "liqimage.h"

typedef struct liqcliprect
{
	int usagecount;
	int sx;
	int sy;
	int ex;
	int ey;
	liqimage *surface;		// thinking about this one... it may make encapsutation easier
} liqcliprect;



//##################################################################
//##################################################################
//##################################################################

liqcliprect * 	liqcliprect_newfromimage(		liqimage *surface);

//##################################################################
//##################################################################
//##################################################################

liqcliprect * 	liqcliprect_new();
liqcliprect * 	liqcliprect_hold(liqcliprect *self);
void    		liqcliprect_release(liqcliprect *self);
void    		liqcliprect_free(liqcliprect *self);

int    	liqcliprect_getx(liqcliprect *self);
int    	liqcliprect_gety(liqcliprect *self);
int    	liqcliprect_getw(liqcliprect *self);
int    	liqcliprect_geth(liqcliprect *self);

//liqcliprect*liqcliprect_create();
//void 		liqcliprect_free(					liqcliprect *self);

//inline void liqcliprect_initfromimage(		liqcliprect *self,liqimage *surface);




inline void liqcliprect_shrink(					liqcliprect *self,int sx,int sy,int ex,int ey);
inline void liqcliprect_copy(					liqcliprect *self,liqcliprect *other);

inline int  liqcliprect_isvalid(				liqcliprect *self);

/*
inline int liqcliprect_rectcheckinside(struct liqcliprect *self,int sx,int sy,int ex,int ey);
inline int liqcliprect_pointcheckinside(struct liqcliprect *self,int x,int y);
 */
void 		liqcliprect_print(					liqcliprect *self,char *prefix);

//##################################################################
//################################################################## drawing functions
//##################################################################

inline void liqcliprect_drawclear(				liqcliprect *self,unsigned char grey,unsigned char u,unsigned char v);
void 		liqcliprect_drawpsetcolor(			liqcliprect *self,int x, int y, unsigned char grey,unsigned char u,unsigned char v);
inline void liqcliprect_drawpgetcolor(      	liqcliprect *self,int x1, int y1, unsigned char *grey,unsigned char *u,unsigned char *v);

//##################################################################
//################################################################## lines and boxes
//##################################################################

void 		liqcliprect_drawlinerowcolor(		liqcliprect *self,int x1, int y1, int x2, int y2, unsigned char grey,unsigned char u,unsigned char v);
void 		liqcliprect_drawlinecolcolor(		liqcliprect *self,int x1, int y1,int x2, int y2, unsigned char grey,unsigned char u,unsigned char v);
void 		liqcliprect_drawlinecolor(			liqcliprect *self,int x1, int y1, int x2, int y2, unsigned char grey,unsigned char u,unsigned char v);

void		liqcliprect_drawthicklinecolor(		liqcliprect *self,int x1, int y1,int x2, int y2, unsigned char thickness, unsigned char grey,unsigned char u,unsigned char v);

void 		liqcliprect_drawboxlinecolor(		liqcliprect *self,int x,int y,int w,int h,unsigned char grey,unsigned char u,unsigned char v);
void 		liqcliprect_drawboxfillcolor(		liqcliprect *self,int x,int y,int w,int h,unsigned char grey,unsigned char u,unsigned char v);
void 		liqcliprect_drawboxfillblendcolor(	liqcliprect *self,int x,int y,int w,int h,unsigned char grey,unsigned char u,unsigned char v,unsigned char blend);
void 		liqcliprect_drawboxwashcolor(		liqcliprect *self,int x,int y,int w,int h,unsigned char u,unsigned char v);

void 		liqcliprect_drawcolorcube(liqcliprect *self,int x,int y,int w,int h,unsigned char grey);
void        liqcliprect_drawgreycol(liqcliprect *self,int x,int y,int w,int h);
void 		liqcliprect_drawboxfadeoutcolor(liqcliprect *self,int x,int y,int w,int h,unsigned char grey,unsigned char u,unsigned char v,unsigned char spread);


//##################################################################
//################################################################## quick font tools
//##################################################################

inline void liqcliprect_drawglyph_grey(			liqcliprect *self,liqfont *font,int x,int y,unsigned char glyph);
int  		liqcliprect_drawtext(         		liqcliprect *self,liqfont *font,int xs,int ys,char *data);
int  		liqcliprect_drawtextn(				liqcliprect *self,liqfont *font,int xs,int ys,char *data,int datalen);
void 		liqcliprect_drawtextcentredon(		liqcliprect *self,liqfont *font,int cx,int cy,char *text);
void 		liqcliprect_drawtextcentredonlimit(liqcliprect *self,liqfont *font,int cx,int cy,char *text,int availablewidth);
void 		liqcliprect_drawtextinside(			liqcliprect *self,liqfont *font,int x,int y,int w,int h,char *text,int alignx);


//##################################################################
//################################################################## slower font tools
//##################################################################


inline void liqcliprect_drawglyph_color(liqcliprect *self,liqfont *font,int x,int y,unsigned char glyph,unsigned char grey,unsigned char u,unsigned char v);
int 		liqcliprect_drawtext_color(liqcliprect *self,liqfont *font,int xs,int ys,char *data,unsigned char grey,unsigned char u,unsigned char v);
int 		liqcliprect_drawtextn_color(liqcliprect *self,liqfont *font,int xs,int ys,char *data,int datalen,unsigned char grey,unsigned char u,unsigned char v);
void 		liqcliprect_drawtextcentredon_color(liqcliprect *self,liqfont *font,int cx,int cy,char *text,unsigned char grey,unsigned char u,unsigned char v);
void 		liqcliprect_drawtextcentredonlimit_color(liqcliprect *self,liqfont *font,int cx,int cy,char *text,int availablewidth,unsigned char grey,unsigned char u,unsigned char v);
void 		liqcliprect_drawtextinside_color(liqcliprect *self,liqfont *font,int x,int y,int w,int h,char *text,int alignx,unsigned char grey,unsigned char u,unsigned char v);


//##################################################################
//################################################################## page
//##################################################################


void 		liqcliprect_drawsketch(				liqcliprect *self,liqsketch *page,int l,int t,int w,int h,int drawmode);	// 0=preview, 1=latest point only, 2=fully detailed


//##################################################################
//################################################################## images
//##################################################################


inline void liqcliprect_drawimagecolor(			liqcliprect *self,liqimage *image,int x,int y,int w,int h, int aspectlock);
inline void liqcliprect_drawimageblendcolor(	liqcliprect *self,liqimage *image,int x,int y,int w,int h,char blend,int aspectlock);

#ifdef __cplusplus
}
#endif


#endif
