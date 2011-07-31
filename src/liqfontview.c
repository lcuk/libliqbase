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
 * library to hold font glyphs as a custom bitmap.
 *
 */






#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "liqapp.h"
#include "liqapp_prefs.h"
#include "liqfont.h"

//###############################################################
//###############################################################
//###############################################################
//###############################################################
#include "liqsketchfont.h"



/*
liqsketchfont *liqsketchfont_me = NULL;


void liqsketchfont_me_prepare()
{
	if(liqsketchfont_me)return;


//	int liqcell_showsketchfont =        1 == atoi(liqapp_pref_getvalue_def("showsketchfont","1"));
//	if(!liqcell_showsketchfont) return;
	
	liqsketchfont_me = liqsketchfont_new();
	liqsketchfont_configure(liqsketchfont_me,canvas.dpix,canvas.dpiy);
	

if(liqapp_fileexists("/home/user/.liqbase/generalfont.liqsketchfont"))
	liqsketchfont_fileload(liqsketchfont_me,"/home/user/.liqbase/generalfont.liqsketchfont");
else
	liqsketchfont_fileload(liqsketchfont_me,"/usr/share/liqbase/liqbook/media/liquid@gmail.com.liqsketchfont");
	
	return;
}
liqsketch * liqsketchfont_me_getglyph(int glyphindex)
{
	liqsketchfont_me_prepare();
	if(!liqsketchfont_me)return NULL;
	liqsketch *s = liqsketchfont_me->glyphs[glyphindex];
	if(s)
	{
		
		liqapp_log("liqsketchfont found glyph! %d wh(%d,%d), swh(%d,%d)",glyphindex, liqsketchfont_me->glyphwidths[glyphindex],liqsketchfont_me->maxh, s->pixelwidth,s->pixelheight );
		return liqsketchfont_me->glyphs[glyphindex];
	}
	return NULL;

}

 */
//###############################################################
//###############################################################
//###############################################################
//###############################################################


#ifdef __cplusplus
extern "C" {
#endif






//####################################################################
//####################################################################
//####################################################################


// todo:: im leaving in here a bug.  it will relate to the way none ascii glyphs are allocated and freed
// it is non optimal at present and will alloc/build/free the glyph just to size it, then repeat this process again
// when you draw it.
// maybe i need a dynamic catchall for this situation

//################################################################
//################################################################
//################################################################



liqfontglyph * liqfontglyph_alloc(int glyphindex,int width,int height)
{
	if((width*height)==0)
	{
		// wtf?
		return NULL;
	}
	char *buf = (char *)calloc(width*height,1);
	if(!buf)
	{
		liqapp_warnandcontinue(-1, "define glyph malloc failed" );
		return NULL;
	}
	//memset(buf,0,width*height*sizeof(char));
	
	
	liqfontglyph *g = (liqfontglyph*)malloc(sizeof(liqfontglyph));
	if(g)
	{
		//g->refcount=1;
		g->glyphindex = glyphindex;
		g->glyphdata = buf;
		g->glyphw = width;
		g->glyphh = height;
		g->sketchlink = NULL;
	}
	return g;
}


//void liqfontglyph_release(liqfontglyph *self)
//{
//	if(--self->refcount) return;
//	if(self->glyphdata) free(self->glyphdata);
//	free(self);
//}

void liqfontglyph_free(liqfontglyph *self)
{
	if(self->glyphdata) free(self->glyphdata);
	free(self);
}







//################################################################
//################################################################ mini global lib holding
//################################################################

static FT_Library  ftlib=NULL;
static int         ftlibopencount=0;



static int ftlib_open()
{
	ftlibopencount++;
	if(ftlibopencount>1) return 0;
FT_Error    fterr;
    // open up
    fterr = FT_Init_FreeType( &ftlib );
    if ( ( fterr ) != 0 )
    {
		ftlibopencount--;
        return liqapp_warnandcontinue(-1, "TTF Init Failed" );
    }
	return 0;
}

static int ftlib_close()
{
	ftlibopencount--;
	if(ftlibopencount>0) return 0;
	////liqapp_log("Closing TTF Library");
    FT_Done_FreeType( ftlib );
	return 0;
}







//################################################################
//################################################################
//################################################################




liqfontview *liqfontview_new()
{
	liqfontview *self = (liqfontview *)calloc(sizeof(liqfontview),1);
	if(self==NULL) {  liqapp_warnandcontinue(-1, "liqfontview_new failed" ); return NULL; }
	self->usagecount=1;
	return self;
}


liqfontview * liqfontview_hold(liqfontview *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}



void liqfontview_release(liqfontview *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqfontview_free(self);
}



void liqfontview_free(liqfontview *self)
{
	// only call this on memory actually allocated, otherwise close
	////liqapp_log("liqfont free closing self");
	liqfontview_close(self);
	////liqapp_log("liqfont free freeing");
	free(self);
	////liqapp_log("liqfont free completed");

}


void liqfontview_close(liqfontview *self)
{
	int idx;
	for(idx=0;idx<256;idx++)
	{
		liqfontglyph *g = self->glyphbuffer[idx];
		if(g)
		{
			liqfontglyph_free(g);
		}
	}
	if(self->ftface)
	{
		////liqapp_log("Closing TTF Font");
	    FT_Done_Face( (FT_FaceRec *)self->ftface );
	}
	ftlib_close();
}






liqfontview * liqfontview_newfromscale(liqfont *font,float scalew,float scaleh)
{
	liqfontview *self = liqfontview_new();
	
	
	
	if(self==NULL) {  liqapp_warnandcontinue(-1, "liqfontview_newfromfilettf creation failed" ); return NULL; }

	self->font=font;
	self->scalew=scalew;
	self->scaleh=scaleh;
	self->pixelheight=0;
	self->ftface=NULL;



	liqsketch * sketchlink = NULL;
	if(self->font->sketchfont)
	{
	liqapp_log("TTF Using sketchfont instead of %s, %i:",font->name,font->size);
		liqsketchfont *sketchfont = self->font->sketchfont;

		
		self->pixelheight = self->font->size * scaleh * 1.6;

		return self;
	}











FT_Error    fterr;
	//FT_Face ftface = NULL;

	
	//liqapp_log("TTF Font opening lib %s, %i:",name,size);

    // open up   
    if( ( ftlib_open() ) != 0 )
    {
         liqapp_warnandcontinue(-1, "TTF open Failed" );
		 liqfontview_release(self);
		 return NULL;
    }
	
	

	//liqapp_log("TTF Font opening face %s, %i:",name,size);
	char *fn = self->font->filename;
	
	if(!liqapp_fileexists(fn)) fn="/usr/share/fonts/droid/DroidSans.ttf";
	if(!liqapp_fileexists(fn)) fn="/usr/share/fonts/nokia/nosnb.ttf";

    fterr = FT_New_Face( ftlib, fn, 0, (FT_Face*)&self->ftface );

    if ( fterr == FT_Err_Cannot_Open_Stream )
	{
        liqapp_log( "TTF Could not find/open font '%s' %i",fn,fterr );
		liqfontview_release(self);
		return NULL;
	}
	
    if ( fterr )
	{
        liqapp_log( "TTF Error while opening font '%s' %i",fn, fterr );
		liqfontview_release(self);
		return NULL;
	}

	//n810 width  90mm   3.54ins  800 pix 225.988dpi
	//n810 height 54mm   2.12ins  480 pix 226.415dpi
	// sort out DPI here :)
	
	//liqapp_log("TTF Font setting size %s, %i:",name,size);
	
	
if(self->font->rotation==0)
	
	fterr = FT_Set_Char_Size( 
		((FT_Face)self->ftface), 
		0, /* char_width in 1/64th of points */  
		self->font->size*64, /* char_height in 1/64th of points */  
		scalew*72.0, /* horizontal device resolution */  
		scaleh*72.0 ); /* vertical device resolution */
else
	fterr = FT_Set_Char_Size( 
		((FT_Face)self->ftface), 
		0, /* char_width in 1/64th of points */  
		self->font->size*64, /* char_height in 1/64th of points */  
		scaleh*72.0, /* horizontal device resolution */  
		scalew*72.0 ); /* vertical device resolution */
    //fterr = FT_Set_Pixel_Sizes( ftface, size, size );



    if ( fterr )
	{
        liqapp_warnandcontinue(-1, "TTF Could not set size" );
		liqfontview_release(self);
		return NULL;
	}


	
    //int infomaxw = ftface->size->metrics.max_advance >> 6;
    int infoheight = (((FT_Face)self->ftface)->size->metrics.ascender - ((FT_Face)self->ftface)->size->metrics.descender + (2 << 6)) >> 6;
    //int infoascent = ((FT_Face)self->ftface)->size->metrics.ascender >> 6;
		
	int glyphmin = 32;
	int glyphmax = ((FT_Face)self->ftface)->num_glyphs;
	if (glyphmax<glyphmin) glyphmin=glyphmax;
	if (glyphmax>255) glyphmax=255;
	if (glyphmax==glyphmin)
	{
        liqapp_warnandcontinue(-1, "TTF sanity check failure: max==min" );
		liqfontview_release(self);
		return NULL;
	}

	self->pixelheight=infoheight;
	return self;
	
}











liqfontglyph    *liqfontview_getglyph(liqfontview *self,int glyphindex)
{
liqfontglyph    *g=NULL;
	if(glyphindex>=0 && glyphindex<=255)
	{
		g = self->glyphbuffer[glyphindex];
	}
	else
	{
		// unicode, not stored all the time..
		// infact, it should be released afterwards rather than freed..
        liqapp_warnandcontinue(-1, "TTF sorry, 8 bit only for now." );
		return NULL;
		
	}
	if(g)
	{
		return g;
	}
	// no g yet, so load and allocate it :)
	
	//
	//liqsketch * sketchlink = liqsketchfont_me_getglyph(glyphindex);
	
	
	liqsketch * sketchlink = NULL;
	if(self->font->sketchfont)
	{
		liqsketchfont *sketchfont = self->font->sketchfont;
		sketchlink = sketchfont->glyphs[glyphindex];
		//if(!sketchlink) return NULL;
	
	
		if(sketchlink)
		{
			int sw = sketchlink->pixelwidth;
			int sh = sketchlink->pixelheight;
			if(sw<60)sw=sw + (60-sw)/2;	// half way to 50 from where we are
			
			int fw = sw * self->pixelheight / sh;
			int fh = self->pixelheight;
	
		//	liqapp_log("sketchfont ahoy! %d fwh(%d,%d)  swh(%d,%d)",glyphindex, fw,fh,  sw,sh);
	
			g = liqfontglyph_alloc(glyphindex,fw,fh);
			if(!g)return NULL;
			g->sketchlink=liqsketch_hold(sketchlink);
			self->glyphbuffer[glyphindex] = g;
			return g;
		}
		else
		{
			g = liqfontglyph_alloc(glyphindex,sketchfont->avgw * self->pixelheight / sketchfont->maxh ,self->pixelheight);
			if(!g)return NULL;
			g->sketchlink=liqsketch_hold(sketchlink);
			self->glyphbuffer[glyphindex] = g;
			return g;
		}
	}


FT_Error    fterr;
unsigned int ch = glyphindex;



        fterr = FT_Load_Char( ((FT_Face)self->ftface), ch , FT_LOAD_RENDER );
        if ( fterr )
        {
            printf("Error loading glyph: %i\n", ch);
            return NULL;
        }




        FT_GlyphSlot 		slot 			= ((FT_Face)self->ftface)->glyph;
        FT_Glyph_Metrics 	glyph_metrics 	= slot->metrics;
        FT_Bitmap			*source 		= &slot->bitmap;
		unsigned char		*src 			= source->buffer;
		
		int infoheight = (((FT_Face)self->ftface)->size->metrics.ascender - ((FT_Face)self->ftface)->size->metrics.descender + (2 << 6)) >> 6;
		int infoascent = ((FT_Face)self->ftface)->size->metrics.ascender >> 6;	

		int fl = glyph_metrics.horiBearingX >> 6;;
		int ft = infoascent - slot->bitmap_top;
		
		int fh = infoheight;
        int fw = glyph_metrics.horiAdvance >> 6;		
		
		unsigned char *buf=NULL;

		if(fl<0){fw+=-fl; fl=0; }
		if(ft<0){fh+=-ft; ft=0; }
		
		//int gw=fl+(source->width);		// glyphw glyphh
		//int gh=ft+(source->rows);
		
		//fw=gw;
		//fh=gh;
		
		
		if(glyphindex==9)
		{
			liqfontglyph    * spc = liqfontview_getglyph(self,32);
			if(spc)
			{
				fw=spc->glyphw*4;
				fh=spc->glyphh;
			}
			
		}
		
		
		
		
		
		if(fl+source->width>fw){fw=fl+source->width; }
		if(ft+source->rows>fh){fh=ft+source->rows; }
		
		//if(fw<8)fw=8;

		int x;
		int y;
		unsigned char pix;
		if(self->font->rotation==0)
		{
			g = liqfontglyph_alloc(ch,fw,fh);
			if(!g)return NULL;
			
			if(glyphindex==9)goto nocopy;
			
			buf = (unsigned char *)g->glyphdata;//self->glyphdata[ch];

			for(y=0;y<source->rows;y++)
			{
				for(x=0;x<source->width;x++)
				{
					pix = src[ ((y)*source->pitch) + (x) ];					
					buf[ (ft+y) * fw + (fl+x) ] = pix;
					
					////liqapp_log("(%i,%i) off %i,pix %i", fw,fh,  (ft+y) * fw + (fl+x) , pix);
				}
			}
			//liqfont_defineglyph(self,9,self->glyphwidths[32]*4,self->glyphheights[32],0);
		}
		else
		{
			// init rot90
			g = liqfontglyph_alloc(ch,fh,fw);
			if(!g)return NULL;
			
			if(glyphindex==9)goto nocopy;
			
			//liqfont_defineglyph(self,ch,fw,fh,0);
			buf = (unsigned char *)g->glyphdata;//self->glyphdata[ch];
			if(buf)
			{
				//if(rx>fw)//liqapp_log("fw %i,   rx %i",fw,rx);
				//if(ry>fh)//liqapp_log("fh %i,   ry %i",fh,ry);

				for(y=0;y<source->rows;y++)
				{
					for(x=0;x<source->width;x++)
					{
						pix = src[ ((y)*source->pitch) + (x) ];					
						//buf[ (ft+y) * fw + (fl+x) ] = pix;
						
						int tx= (fh-1)-(ft+y);
						int ty= (fl+x);
						int off=tx + ty * fh;
						//if(off>self->glyphsizes[ch])
							////liqapp_log("(%ix%i)=%i, siz %i,    off %i,pix %i   fl %i   ft %i", fw,fh,fw*fh,self->glyphsizes[ch],   off , pix,fl,ft);
						
						
						//buf[   ( (fl+x) ) * fh + ((fh-1)-(ft+y)) ] = pix;

						buf[ off ] = pix;
						
					}
				}			
			}
		}
				//liqfont_defineglyph(self,9,self->glyphwidths[32],self->glyphheights[32]*4,0);
				//liqfont_defineglyph(self,9,self->glyphwidths[32]*4,self->glyphheights[32],0);

nocopy:

	//if(glyphindex>=0 && glyphindex<=255)
	{
		// cache 8bit :)
		self->glyphbuffer[glyphindex] = g;
	}
	return g;
}

#ifdef __cplusplus
}
#endif

