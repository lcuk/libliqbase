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
 * Header for Routines which write data onto a YUV liqimage surface
 *
 */



#ifndef liq_xsurface_h
#define liq_xsurface_h

#ifdef __cplusplus
extern "C" {
#endif

#include "liqimage.h"
#include "liqfont.h"
void xsurface_drawstrip_colortest1(
	register unsigned int  linecount,
	register unsigned int  charsperline,
	register unsigned char *srcdataptr,
	register unsigned char *dstdataptr,
	register unsigned int  srclinejump,
	register unsigned int  dstlinejump,

	liqimage *destimage,
	unsigned char col_y,
	unsigned char col_u,
	unsigned char col_v,
	int dsx,int dsy
	);
void xsurface_drawstrip_or(
	register unsigned int  linecount,
	register unsigned int  charsperline,
	register unsigned char *srcdataptr,
	register unsigned char *dstdataptr,
	register unsigned int  srclinejump,
	register unsigned int  dstlinejump);
void xsurface_drawstrip(
	register unsigned int  linecount,
	register unsigned int  charsperline,
	register unsigned char *srcdataptr,
	register unsigned char *dstdataptr,
	register unsigned int  srclinejump,
	register unsigned int  dstlinejump);

void xsurface_drawglyph_grey(    liqimage *surface,liqfont *font,int x,int y,unsigned char glyph);


       int  xsurface_drawtext_grey(     liqimage *surface,liqfont *font,int xs,int ys,char *data);
       int  xsurface_drawtextn_grey(    liqimage *surface,liqfont *font,int xs,int ys,char *data,int datalen);
void xsurface_drawclear_grey(    liqimage *surface,unsigned char grey);
void xsurface_drawclear_yuv(     liqimage *surface,unsigned char grey,unsigned char u,unsigned char v);
void xsurface_drawrect_yuv(      liqimage *surface,int x,int y,int w,int h, unsigned char grey,unsigned char u,unsigned char v);
void xsurface_drawrect_grey(     liqimage *surface,int x,int y,int w,int h, unsigned char grey);
void xsurface_drawrectwash_uv(   liqimage *surface,int x,int y,int w,int h, unsigned char u,unsigned char v);
void xsurface_drawfadeoutrect_yuv(liqimage *surface,int x,int y,int w,int h, unsigned char grey,unsigned char u,unsigned char v,unsigned char spread);


void xsurface_drawpset_yuv(      liqimage *surface,int x,int y,char grey,char u,char v);
void xsurface_drawpset_grey(     liqimage *surface,int x,int y,char grey);
void xsurface_drawpget_yuv(      liqimage *surface,int x1, int y1, unsigned char *grey,unsigned char *u,unsigned char *v);
void xsurface_drawline_yuv(      liqimage *surface,int x1, int y1, int x2, int y2, char grey,char u,char v);
void xsurface_drawthickline_yuv( liqimage *surface,int x1, int y1, int x2, int y2,unsigned char thickness, char grey,char u,char v);
void xsurface_drawline_grey(     liqimage *surface,int x1, int y1, int x2, int y2, char grey);
void xsurface_drawline_greyinv(  liqimage *surface,int x1, int y1, int x2, int y2);
void xsurface_drawcircle_grey(   liqimage *surface,int cx, int cy, int r,unsigned char grey);

void xsurface_drawimage_color(   liqimage *surface,liqimage *image,int x,int y);

void xsurface_drawzoomimage(

										liqimage *srcimage,
										int six,int siy,		// SrcImgPos
										int siw,int sih, 		// SrcImgSize

										liqimage *dstimage,
										int dix,int diy,		// DstImgPos
										int diw,int dih 		// DstImgSize

										);

void xsurface_drawzoomblendimage(

										liqimage *srcimage,
										int six,int siy,		// SrcImgPos
										int siw,int sih, 		// SrcImgSize

										liqimage *dstimage,
										int dix,int diy,		// DstImgPos
										int diw,int dih, 		// DstImgSize

										unsigned char blend
										);


#ifdef __cplusplus
}
#endif

#endif
