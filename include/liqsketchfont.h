



#ifndef LIQSKETCHFONT_H
#define LIQSKETCHFONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liqapp.h"
#include "liqcanvas.h"
#include "liqcliprect.h"
#include "liqsketch.h"








typedef struct liqsketchfont
{
	// font generated from sketches
	// stored at default orientation only, 
	int usagecount;
	
	liqsketch *glyphs[256];
	
	int glyphwidths[256];

	int maxw;
	int maxh;
	
	int avgw;
	
	int dpix;
	int dpiy;
}
	liqsketchfont;
	

liqsketchfont * liqsketchfont_cache_find(const char *ident);
	

int liqsketchfont_configure(liqsketchfont *self,int dpix,int dpiy);
int liqsketchfont_addglyph_size(liqsketchfont *self,unsigned char glyph,liqsketch *glyphdata,int uw,int uh);
int liqsketchfont_addglyph(liqsketchfont *self,unsigned char glyph,liqsketch *glyphdata);

int liqsketchfont_filesave(liqsketchfont *self,char *filename);
int liqsketchfont_fileload(liqsketchfont *self,char *filename);

int liqsketchfont_textfitinside(liqsketchfont *self,char *data,int availablewidth);
int liqsketchfont_textwidth(    liqsketchfont *self,char *data);
int liqsketchfont_textwidthn(   liqsketchfont *self,char *data,int datalen);


float liqsketchfont_calcaspect(int captionw,int captionh,int availw,int availh);
int liqsketchfont_textrender(liqsketchfont *font,liqcliprect *cr,int xs,int ys,int availw,int availh,char *data);


void liqsketchfont_test();
	
	
	
//#########################################################################
//#########################################################################
//######################################################################### cell construction and reference counting
//#########################################################################
//#########################################################################

liqsketchfont *liqsketchfont_new();
void liqsketchfont_hold(liqsketchfont *self);
void liqsketchfont_release(liqsketchfont *self);
void liqsketchfont_free(liqsketchfont *self);


#ifdef __cplusplus
}
#endif



#endif


