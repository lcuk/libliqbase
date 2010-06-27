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
 * Header for font glyph buffering routines
 *
 */


#ifndef liqfont_H
#define liqfont_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
	//int 			refcount;
	int				glyphindex;
	int				glyphw;
	int				glyphh;
	int				glyphbaseline;		//
	char           *glyphdata;			// allocate any used
} liqfontglyph;

liqfontglyph * liqfontglyph_alloc(int glyphindex,int width,int height);
//void liqfontglyph_release(liqfontglyph *self);
void liqfontglyph_free(liqfontglyph *self);







typedef struct liqfont     liqfont;
typedef struct liqfontview liqfontview;

struct liqfontview 
{
	int usagecount;
	liqfont *       font;
	float 			scalew;				// actual realized cache size
	float 			scaleh;
	liqfontglyph    *glyphbuffer[256];	// allocate any used
	int				pixelheight;		// maximum glyphwidth 	use this for obtaining boundary
	void 			*ftface;
};




// use the cache functions for speed :)
liqfontview * 	liqfontview_newfromscale(liqfont *font,float scalew,float scaleh);
liqfontview * 	liqfontview_new();
liqfontview *	liqfontview_hold(liqfontview *self);
void 			liqfontview_release(liqfontview *self);
void 			liqfontview_free(liqfontview *self);		// do not use directly
void 			liqfontview_close(liqfontview *self);
liqfontglyph *  liqfontview_getglyph(liqfontview *self,int glyphindex);	// release afterwards : liqfontviewglyph_release








struct liqfont
{
	int usagecount;
	char *			name;
	char *			style;
	char *			filename;
	int				size;
	int				rotation;			// 0/90/180/270 currently fixed to 0, but capable of others with tweaks
	liqfontview *   viewcache[32];		// each available rendered view
	int             viewcacheused;
	liqfontview *   viewcachecurrent;	// current actual set in stone view of this font :)	
};

// use the cache functions for speed :)
liqfont *liqfont_cache_getttf(const char *name,int size,int rotation);
void     liqfont_cache_release(liqfont *self);
liqfont * liqfont_newfromfilettf(const char *name,int size,int rotation);

liqfont *liqfont_new();
liqfont *liqfont_hold(liqfont *self);
void liqfont_release(liqfont *self);

void liqfont_free(liqfont *self);		// do not use directly

void liqfont_close(liqfont *self);
//int liqfont_openblank(liqfont *self,char *name,int size,int rotation,float scalew,float scaleh);


int liqfont_setview(liqfont *self,float scalew,float scaleh);

liqfontglyph *liqfont_getglyph(liqfont *self,int glyphindex);	// release afterwards : liqfontglyph_release
int liqfont_getglyphwidth(liqfont *self,int glyphindex);		// automatic handling
int liqfont_getglyphheight(liqfont *self,int glyphindex);

int liqfont_textfitinside(liqfont *self, const char *data,int availablewidth);
int liqfont_textwidth(liqfont *self, const char *data);
int liqfont_textwidthn(liqfont *self, const char *data,int datalen);
int liqfont_textheight(liqfont *self);


#ifdef __cplusplus
}
#endif

#endif
