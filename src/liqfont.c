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
#include "liqfont.h"

#ifdef __cplusplus
extern "C" {
#endif


//####################################################################
//####################################################################
//####################################################################


static struct
{
	char *key;
	liqfont *data;	
}
			cachestack[64];
static int  cachemax=64-1;
static int  cacheused=0;


static int liqfont_cache_clean_unused(int maxremove)
{
	// cleans all fonts not in use (upto maxremove count)
	int f=0;
	int removedcount=0;
	for(f=0;f<cacheused;f++)
	{
		if(cachestack[f].data->usagecount==1)
		{
			//liqapp_log("releasing %s",cachestack[f].key);
			// not used by anyone other than cache anymore..
			// remove it from the stack
			// and if we are not the final item, move the final item to here
			free(cachestack[f].key);
			liqfont_release(cachestack[f].data);
			
			cachestack[f].key =NULL;				
			cachestack[f].data=NULL;				
			//liqapp_log("released, adjusting stack");
			
			int g;
			for(g=f+1;g<cacheused;g++)
			{
				//cachestack[g-1] = cachestack[g];
				cachestack[g-1].key =cachestack[g].key;				
				cachestack[g-1].data=cachestack[g].data;				
			}
		/*	
			cachestack[f].key =NULL;				
			cachestack[f].data=NULL;				
			if(f<(cacheused-1))
			{
				cachestack[f].key  = cachestack[cacheused-1].key;
				cachestack[f].data = cachestack[cacheused-1].data;
				f--;	// move backwards slightly to ensure we rescan the moved item
			}
		*/
			//liqapp_log("released, done");
			if(f<(cacheused-1))
				f--;
				
			cacheused--;
			removedcount++;
			if(removedcount>=maxremove) break;
		}
	}
	//liqapp_log("removed %i",removedcount);
	return removedcount;
}


//####################################################################
//####################################################################
//####################################################################


liqfont *liqfont_cache_getttf(const char *name,int size,int rotation)
{
	//
	liqfont *self=NULL;
	char cachekey[256];
	int f;
	if(!liqapp_fileexists(name))
	{
		name =  "/usr/share/fonts/truetype/freefont/FreeSans.ttf";
	}


	snprintf(cachekey,256,"FONT:%s,%i,%i",name,size,rotation);
	//liqapp_log( "TTF cache seeking %s", cachekey );
	if(cacheused>=cachemax)
	{
		//liqapp_log( "TTF cache cleaning %s", cachekey );
		if(liqfont_cache_clean_unused(8)==0)
		{
			// all font slots actively in use
			// error in app or just very varied
	        //liqapp_log( "TTF cache full %s", cachekey );
			return NULL;
		}
	}
	//for(f=0;f<cacheused;f++)
	for(f=cacheused-1;f>=0;f--)
	{
		if(strcmp(cachestack[f].key,cachekey)==0)
		{
			// no differences..
			//liqapp_log( "TTF cache matched %s %i", cachekey ,cachestack[f].data->usagecount);
			self = cachestack[f].data;
			//self->usagecount++;
			liqfont_hold(self);
			//liqapp_log("found %s", cachekey );
			// 20090414_221517 lcuk : make sure we return a font of the correct scale
			liqfont_setview(self,1,1);
			return self;
		}
		// whilst I am searching, perhaps I should be moving 0 rated items to the bottom of the stack
	}
		//liqapp_log("not found %s", cachekey );
	// not yet in the cache
	//liqapp_log( "TTF cache creating %s", cachekey );

//int err=0;
	self=liqfont_newfromfilettf(name, size,rotation);
	if(!self)
	{
        liqapp_log( "TTF couldn't create %s", cachekey );
		return NULL;		
	}

	// simply add our own lock onto this font handle :)
	
	liqfont_hold(self);
	
	//self->usagecount=1;

	//liqapp_log( "TTF cache inserting %s", cachekey );

	f=cacheused;
	cachestack[f].key  = strdup(cachekey);
	cachestack[f].data = self;
	cacheused++;
	////liqapp_log( "TTF cache completed %s", cachekey );
	return self;
}





void liqfont_cache_release(liqfont *self)
{
	// shouldnt actually do anything with this yet
	// we have an extra lock on the data
	// i hope to god the user doesnt do extra holds/releases, but i suppose thats the same with anything
	liqfont_release(self);
}




//####################################################################
//####################################################################
//####################################################################



















//########################################################################
//######################################################################## textfitinside - tell me how much text fits within
//########################################################################

int liqfont_getglyphwidth(liqfont *self,int glyphindex)
{
	// the intent bug crops up here for unicode glyphs, since we do too much just to get its size
	int w=0;
	liqfontglyph *g = liqfont_getglyph(self,glyphindex);
	if(g)
	{
		w=g->glyphw;
	}
	return w;
}

int liqfont_getglyphheight(liqfont *self,int glyphindex)
{
	// the intent bug crops up here for unicode glyphs, since we do too much just to get its size
	int h=0;
	liqfontglyph *g = liqfont_getglyph(self,glyphindex);
	if(g)
	{
		h=g->glyphh;
	}
	return h;
}

int liqfont_textfitinside(liqfont *self, const char *data,int availablewidth)
{
	if(!data) return 0;
	int x=0;
	unsigned char ch;
	int w=0;
	int len=0;
	while ( (ch=*data++) )
	{
		liqfontglyph *g = liqfont_getglyph(self,ch);
		if(g)
		{
			w=g->glyphw;
			if(x+w>=availablewidth)return len;
			x+=w;
		}
		len++;
	}
	return len;
}

int liqfont_textwidth(liqfont *self, const char *data)
{
	if(!data) return 0;
	int x=0;
	unsigned char ch;
	while ( (ch=*data++) )
	{
		liqfontglyph *g = liqfont_getglyph(self,ch);
		if(g)
		{
			x+=g->glyphw;
		}
		//x+=self->glyphwidths[ch];
	}
	return x;
}
	

int liqfont_textwidthn(liqfont *self, const char *data,int datalen)
{
	if(!data) return 0;
	int x=0;
	unsigned char ch;
	if(datalen<=0)return x;
	while(datalen--)
	{
		ch=*data++;
		liqfontglyph *g = liqfont_getglyph(self,ch);
		if(g)
		{
			x+=g->glyphw;
		}
		//x+=self->glyphwidths[ch];
	}
	return x;
}


//################################################################
//################################################################
//################################################################




liqfont *liqfont_new()
{
	liqfont *self = (liqfont *)calloc(sizeof(liqfont),1);
	if(self==NULL) {  liqapp_warnandcontinue(-1, "liqfont_new failed" ); return NULL; }
	self->usagecount=1;
	return self;
}


liqfont * liqfont_hold(liqfont *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}



void liqfont_release(liqfont *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqfont_free(self);
}



void liqfont_free(liqfont *self)
{
	// only call this on memory actually allocated, otherwise close
	////liqapp_log("liqfont free closing self");
	liqfont_close(self);
	////liqapp_log("liqfont free freeing");
	free(self);
	////liqapp_log("liqfont free completed");

}


void liqfont_close(liqfont *self)
{
	if(self->name)
	{
		free(self->name);
		self->name=NULL;
	}
	if(self->style)
	{
		free(self->style);
		self->style=NULL;
	}
	if(self->filename)
	{
		free(self->filename);
		self->filename=NULL;
	}
	
	int idx;
	for(idx=0;idx<self->viewcacheused;idx++)
	{
		liqfontview *v = self->viewcache[idx];
		if(v)
		{
			liqfontview_release(v);
		}
	}
}

liqfont * liqfont_newfromfilettf(const char *name,int size,int rotation)
{

	//rotation=90;
	//rotation=0;
	//liqapp_log("TTF Font opening File %s, %i:",name,size);
	
	liqfont *self = liqfont_new();
	if(self==NULL) {  liqapp_warnandcontinue(-1, "liqfont_newfromfilettf creation failed" ); return NULL; }
	
	if(size<6)size=6;
	if(size>100)size=100;
	if(rotation==0 || rotation==90)
		rotation=rotation;
	else
		rotation=0;		// for now, quick cop out...
	self->name = strdup(name);
	self->filename = strdup(name);
	self->size = size;
	self->rotation = rotation;
	memset((char *)self->viewcache,0,sizeof(self->viewcache));
	self->viewcacheused=0;
	self->viewcachecurrent=NULL;
	liqfont_setview(self,1,1);	
	return self;
	
}




liqfontglyph    *liqfont_getglyph(liqfont *self,int glyphindex)
{
liqfontglyph    *g=NULL;

	if(glyphindex>=0 && glyphindex<=255)
	{
		if(!self->viewcachecurrent) return NULL;
		g = liqfontview_getglyph( self->viewcachecurrent, glyphindex);
	}
	else
	{
		// unicode, not stored all the time..
		// infact, it should be released afterwards rather than freed..
        liqapp_warnandcontinue(-1, "TTF sorry, 8 bit only for now." );
		return NULL;
		
	}

	return g;
}


int liqfont_textheight(liqfont *self)
{
	if(self->viewcachecurrent)
	{
		return self->viewcachecurrent->pixelheight;
	}
	return 0;
}


int liqfont_setview(liqfont *self,float scalew,float scaleh)
{
	
	//liqapp_log("viewscale %3.3f,%3.3f",scalew,scaleh);

	//if(scalew<1)
	//{
	//	if(scaleh<1)
	//		scalew=scaleh;
	//	else
	//		scalew=1;
	//}
	float fac=((scalew+scaleh)/2);
	scalew=fac;
	scaleh=fac;
	
	
	if(scalew<0.05 || scaleh < 0.05)
	{
		// dont even try
		self->viewcachecurrent= NULL;
		return -1;
	}
	
	//
	if(self->viewcachecurrent)
	{
		// always act as if there is only one size in the cache, but expand it shortly
		if(self->viewcachecurrent->scalew==scalew && self->viewcachecurrent->scaleh==scaleh)
		{
			// already correct, bail
			return 0;
		}
		// ok, walk BACKWARDS and check if we have it
		self->viewcachecurrent= NULL;		
	}
	
	int f;
	int g;
	for(f=self->viewcacheused-1;f>=0;f--)
	{
		if(self->viewcache[f]->scalew==scalew && self->viewcache[f]->scaleh==scaleh)
		{
			// set the viewcachecurrent and bail
			// should push this to the top of the stack if not already
			self->viewcachecurrent = self->viewcache[f];
			for(g=f+1;g<self->viewcacheused;g++)
			{
				self->viewcache[g-1] = self->viewcache[g];
			}
			// ensure that the current selection bubbles to the top
			self->viewcache[self->viewcacheused-1] = self->viewcachecurrent; 
			return 0;
		}
	}
	// should release the LOWEST/oldest cache item
	if(self->viewcacheused>=30)
	{
		// only release if we have no room
		
		// lets be ruthless and free up 8 at a time...
		int deletecyclestoclear;
		for(deletecyclestoclear=0;deletecyclestoclear< 8 ;deletecyclestoclear++)
		{
			liqfontview_release(self->viewcache[0]);
			self->viewcache[0]=NULL;
			for(g=1;g<self->viewcacheused;g++)
			{
				self->viewcache[g-1] = self->viewcache[g];
			}
			self->viewcacheused--;
		}
	}
	
	

	

		if(self->viewcacheused>=31)
		{
			// error..
			liqapp_log("liqfont_setview full cache %3.3f,%3.3f" ,scalew,scaleh);
			return -1;
					
		}
		

	//liqapp_log("viewscale alloc %3.3f,%3.3f",scalew,scaleh);
	
	self->viewcachecurrent =liqfontview_newfromscale(self,scalew,scaleh);
	if(!self->viewcachecurrent)
	{
		//liqapp_log("liqfont_setview failed %3.3f,%3.3f" ,scalew,scaleh);
		return -1;
	}
	
	self->viewcache[ self->viewcacheused++ ]=self->viewcachecurrent;
	return 0;

}

#ifdef __cplusplus
}
#endif

