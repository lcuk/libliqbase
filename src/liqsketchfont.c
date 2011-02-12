
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
 * sketch based font handling subsystem
 * todo: allow variants of each glyph to be handleable/saveable,
 *       retrying a letter 4 or 5 times can be a good way to get loads of slight randomness
 *       text will appear to be more lifelike and natural.
 *       it would be almost impossible to tell difference from real handwritten :)
 *       
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>


#include "liqbase.h"
#include "filebuf.h"
#include "liqdoc.h"


#include "liqsketchfont.h"


static struct
{
	char *key;
	liqsketchfont *data;	
}
			cachestack[64];
static int  cachemax=64-1;
static int  cacheused=0;


liqsketchfont * liqsketchfont_cache_find(const char *ident)
{
	//
	liqsketchfont *self=NULL;
	char cachekey[256];
	int f;


	snprintf(cachekey,256,"%s",ident);
	liqapp_log( "sketchfont cache seeking %s", cachekey );
	if(cacheused>=cachemax)
	{
			// all font slots actively in use
			// error in app or just very varied
	        liqapp_log( "sketchfont cache full %s", cachekey );
			return NULL;
	}
	//for(f=0;f<cacheused;f++)
	for(f=cacheused-1;f>=0;f--)
	{
		if(strcmp(cachestack[f].key,cachekey)==0)
		{
			// no differences..
			liqapp_log( "sketchfont cache matched %s %i", cachekey ,cachestack[f].data->usagecount);
			self = cachestack[f].data;
			//self->usagecount++;
			liqsketchfont_hold(self);
			return self;
		}
		// whilst I am searching, perhaps I should be moving 0 rated items to the bottom of the stack
	}
		//liqapp_log("not found %s", cachekey );
	// not yet in the cache
	liqapp_log( "sketchfont cache creating %s", cachekey );



	char filename[128];
	snprintf(filename,128,"/usr/share/liqbase/liqbook/media/%s.liqsketchfont",ident);

	if( ! liqapp_fileexists(filename))
	{
		
		if(liqapp_fileexists("/home/user/.liqbase/generalfont.liqsketchfont"))
			snprintf(filename,128,"/home/user/.liqbase/generalfont.liqsketchfont");
		else
			snprintf(filename,128,"/usr/share/liqbase/liqbook/media/liquid@gmail.com.liqsketchfont");

	}
	if( ! liqapp_fileexists(filename))
	{
		// no assoc font
        liqapp_log( "sketchfont cache couldn't find %s", cachekey );
		return NULL;
	}
	
	
	self = liqsketchfont_new();
	if(!self)
	{
        liqapp_log( "sketchfont cache couldn't create %s", cachekey );
		return NULL;		
	}
	liqsketchfont_configure(self,canvas.dpix,canvas.dpiy);
	liqsketchfont_fileload(self,filename);

	// simply add our own lock onto this sketchfont handle :)
	
	liqsketchfont_hold(self);
	
	//self->usagecount=1;

	liqapp_log( "sketchfont cache inserting %s", cachekey );

	f=cacheused;
	cachestack[f].key  = strdup(cachekey);
	cachestack[f].data = self;
	cacheused++;
	liqapp_log( "sketchfont cache completed %s", cachekey );
	return self;
}




//#######################################################################
//#######################################################################
//#######################################################################
//#######################################################################



int liqsketchfont_configure(liqsketchfont *self,int dpix,int dpiy)
{
	int idx;
	for(idx=0;idx<256;idx++)
	{
		self->glyphs[idx]=0;
		self->glyphwidths[idx]=0;
	}
	
	self->dpix=dpix;
	self->dpiy=dpiy;
	
	self->avgw=0;
	
	self->maxw=0;
	self->maxh=0;
	return 0;
}
int liqsketchfont_addglyph_size(liqsketchfont *self,unsigned char glyph,liqsketch *glyphdata,int uw,int uh)
{
	self->glyphs[glyph] = glyphdata;
	self->glyphwidths[glyph] = uw;
	if(uw>self->maxw)self->maxw=uw;
	self->avgw=(self->avgw+uw)/2;
	if(uh>self->maxh)self->maxh=uh;
	return 0;
}

int liqsketchfont_addglyph(liqsketchfont *self,unsigned char glyph,liqsketch *glyphdata)
{
	int uw = glyphdata->boundingbox.xr-glyphdata->boundingbox.xl;
	int uh = glyphdata->boundingbox.yb-glyphdata->boundingbox.yt;
	liqsketchfont_addglyph_size(self,glyph,glyphdata,uw,uh);
	return 0;
}



int liqsketchfont_filesave(liqsketchfont *self,char *filename)
{
	FILE * fd = fopen(filename,"w");
	if(!fd)
	{
		liqapp_log("liqsketchfont_filesave: couldn't open '%s' for writing",filename);
		return -1;
	}
		liqapp_log("liqsketchfont_filesave: creating '%s'",filename);
	
											fprintf(fd,"liqsketchfont\n");
											fprintf(fd,"size:%i,%i\n",self->maxw,self->maxh);
											fprintf(fd,"dpi:%i,%i\n",self->dpix,self->dpiy);
	
	int idx;
	int miny=9999;
	for(idx=0;idx<256;idx++)
	{
		if(self->glyphs[idx])
		{
			liqsketch *pg=self->glyphs[idx];
			liqstroke *s = pg->strokefirst;
			while(s)
			{
				liqpoint *p=s->pointfirst;
				while(p)
				{
					if(p->y<miny)miny=p->y;
					p=p->linknext;
				}
				s=s->linknext;
			}
		}
	}

	for(idx=0;idx<256;idx++)
	{
		if(self->glyphs[idx])
		{
		liqapp_log("liqsketchfont_filesave: writing '%s': %i",filename,idx);


			//
											fprintf(fd," ch:%i,%i\n",idx,self->glyphwidths[idx]);
			liqsketch *pg=self->glyphs[idx];
			liqstroke *s = pg->strokefirst;
			while(s)
			{
				//
											fprintf(fd,"  st:%i,%i,%i\n",s->pen_y,s->pen_u,s->pen_v);
				liqpoint *p=s->pointfirst;
				while(p)
				{
					//
											fprintf(fd,"   pt:%i,%i,%i\n",p->x,p->y-miny,p->z);
					p=p->linknext;
				}
				s=s->linknext;
			}
		}
	}
	
	fclose(fd);

		liqapp_log("liqsketchfont_filesave: finished '%s'",filename);

}	















int liqsketchfont_fileload(liqsketchfont *self,char *filename)
{
	liqapp_log("liqsketchfont_fileload '%s'",filename);
	struct doc doc;
	int err=0;
	
	liqapp_log("liqsketchfont_fileload init");
	
	doc.renderfont=NULL;
	
	err=doc_initfromfilename(&doc,filename);
	if(err)
	{
    	{ return liqapp_warnandcontinue(-1,"liqsketchfont_fileload couldnt open"); }						
	}
	
	
	int linenum=1;	
	struct docline *docline = doc.linefirst;

	if(!docline)
	{
		// empty file
		doc_close(&doc);
	   	{ return liqapp_warnandcontinue(-1,"liqsketchfont_fileload file is empty"); }						
	}


	liqapp_log("liqsketchfont_fileload scan lines, start: '%s'",docline->linedata);


	if(strncmp(docline->linedata,"liqsketchfont",5) != 0)
	{
		// invalid header
		doc_close(&doc);
	   	{ return liqapp_warnandcontinue(-1,"liqsketchfont_fileload invalid file header"); }
	}

	liqsketch *  pg = NULL;
	liqstroke *st = NULL;
	liqpoint * pt = NULL;
	
	liqapp_log("liqsketchfont_fileload configuring");
	
	
	liqsketchfont_configure(self,225,255);		// use default

	liqapp_log("liqsketchfont_fileload reading lines");

	while(docline)
	{
		// a line is "cmd:arg,arg,arg"
		// different cmds do different things
		char *indat = docline->linedata;
		while(*indat==' ' || *indat=='\t')indat++;
		
		char *colon = strchr(indat,':');

	//	liqapp_log("liqsketchfont_fileload trying line: %5i '%s'",linenum,indat);

		if(!colon)
		{
			// this is an invalid line
		}
		else
		{
			// now we can break into comma delims
			// this is destructive to the memory, we null as we go along..
			

			*colon = '\0';
			char *xcmd = indat;
			char *xcols[80+1];
			int   xcolcount=0;

			indat = colon+1;
			while(*indat==' ' || *indat=='\t')indat++;
			xcols[0] = indat;
			xcolcount++;
			
			while(*indat && xcolcount<80)
			{
				if(*indat==',')
				{
					*indat=0;
					indat++;
					while(*indat==' ' || *indat=='\t')indat++;
					xcols[xcolcount++]=indat;
				}
				else
					indat++;
			}
			
		//	liqapp_log("liqsketchfont_fileload line '%s'",docline->linedata);
			// ok, sorted, now we have a really quick simple bit of parsing to do :)
			// rotate this round so the most frequent items are compared first
			if((strcmp(xcmd,"size")==0) && xcolcount==2)
			{
				self->maxw = atoi( xcols[0] );
				self->maxh = atoi( xcols[1] );
			}
			else
			if((strcmp(xcmd,"dpi")==0) && xcolcount==2)
			{
				self->dpix = atoi( xcols[0] );
				self->dpiy = atoi( xcols[1] );
			}
			else
			if((strcmp(xcmd,"ch")==0) && xcolcount==2)
			{
				//
				// ok, gonna define a glyph

				pg = liqsketch_new();
				pg->pixelwidth=atoi( xcols[1] );
				pg->pixelheight=self->maxh;
				pg->dpix=self->dpix;
				pg->dpiy=self->dpiy;
				st = NULL;
				pt = NULL;
				
				//
				liqsketchfont_addglyph_size(self,  (unsigned char)atoi( xcols[0] ),  pg,  pg->pixelwidth,  pg->pixelheight);
				
				liqapp_log("glyph %d:   swh(%d,%d)",atoi( xcols[0] ),   pg->pixelwidth,pg->pixelheight);

			}
			else
			if((strcmp(xcmd,"st")==0) && xcolcount==3)
			{
				st = liqstroke_new();
				st->pen_y = (unsigned char) atoi( xcols[0] );
				st->pen_u = (unsigned char) atoi( xcols[1] );
				st->pen_v = (unsigned char) atoi( xcols[2] );
			}
			else
			if((strcmp(xcmd,"pt")==0) && xcolcount==3)
			{
				if(st->pointcount==0)
				{
					liqstroke_start(st,
								 (unsigned char) atoi( xcols[0] ),
								 (unsigned char) atoi( xcols[1] ),
								 (unsigned char) atoi( xcols[2] ),
								 0
								 );
					liqsketch_strokeinsert(pg,st);
				}
				else
				{		
					liqstroke_extend(st,
								 (unsigned char) atoi( xcols[0] ),
								 (unsigned char) atoi( xcols[1] ),
								 (unsigned char) atoi( xcols[2] ),
								 0
								 );
					liqsketch_strokeupdate(pg,st);					
				}
				
			}
			else
			{
				// invalid command
			}
		}
		docline=docline->linknext;
		linenum++;
	}
	doc_close(&doc);
	
	return 0;
}























































































int liqsketchfont_textfitinside(liqsketchfont *self,char *data,int availablewidth)
{
	int tot=0;
	int fit=0;
	while(*data)
	{
		tot+=self->glyphwidths[*data++];
		if(tot>=availablewidth) return fit;
		fit++;
	}
	return fit;	
}
int liqsketchfont_textwidth(    liqsketchfont *self,char *data)
{
	int tot=0;
	while(*data)
	{
		tot+=self->glyphwidths[*data++];
	}
	return tot;
}
int liqsketchfont_textwidthn(   liqsketchfont *self,char *data,int datalen)
{
	int tot=0;
	while(*data && datalen>0)
	{
		tot+=self->glyphwidths[*data++];
		datalen--;
	}
	return tot;
}




float liqsketchfont_calcaspect(int captionw,int captionh,int availw,int availh)
{
	if(captionw==0)return 0;
	if(captionh==0)return 0;
	float ax = (float)availw / (float)captionw;
	float ay = (float)availh / (float)captionh;
	float ar = (ax<=ay ? ax : ay);


	//liqapp_log("from %i,%i",captionw,captionh);
	//liqapp_log("to   %i,%i",availw,availh);
	//liqapp_log("ax   %f,%f,%f",ax,ay,ar);
	
	
	return ar;
	
}


int liqsketchfont_textrender(liqsketchfont *font,liqcliprect *cr,int xs,int ys,int availw,int availh,char *data)
{
	
	int captionw = liqsketchfont_textwidth(font,data);
	int captionh = font->maxh;
	
	float ar = liqsketchfont_calcaspect(  captionw,captionh,  availw,availh);
	
	int x=xs;
	int h=(int)(    ((float)captionh) * ar      );
	
	unsigned char ch;
	while ( (ch=*data++) )
	{
		int w= (int)(     ((float)font->glyphwidths[ch]) * ar );
		//if((w>0) && font->glyphs[ch])
		if(font->glyphs[ch])
		{
			//liqapp_log("tr p=%i,%i  f=%i,%i    ar=%f",x,ys,w,h,ar);
			liqcliprect_drawsketch(cr,        font->glyphs[ch],x,ys,w,h,  0 );
			//cliprect_drawboxlinecolor(cr,                 x,ys,w,h,  255,128,128 );
		}
		x+=w;
	}
	return x;
}



void liqsketchfont_test()
{

	liqsketchfont fontcore;
	liqsketchfont *font=&fontcore;
	liqsketchfont_configure(font,canvas.dpix,canvas.dpiy);
	
	liqsketchfont_fileload(font,"/usr/share/liqbase/liqbook/media/_mytestfont.liqsketchfont");
	
/*	
	int idx;
	for(idx=0;idx<256;idx++)
	{
		if(fontpages[idx])
		{
			liqsketchfont_addglyph(font,(char)idx,fontpages[idx]);
		}
	}
*/

	font->glyphwidths[9 ] = font->avgw * 4;
	font->glyphwidths[32] = font->avgw;
	
	
	liqsketchfont_textrender(font,canvas.cr,0,0,canvas.pixelwidth,100,"mary had a little lamb, its fleece was white as snow.");
	
	
	
//	liqsketchfont_filesave(font,"_mytestfont.liqsketchfont");
	
}	
	
	
	
	
//#########################################################################
//#########################################################################
//######################################################################### cell construction and reference counting
//#########################################################################
//#########################################################################

liqsketchfont *liqsketchfont_new()
{
	// use this to allocate and hold onto a reference
	// should really overload this and allow variations..
	liqsketchfont *self = (liqsketchfont *)malloc(sizeof( liqsketchfont ));
	memset((char *)self,0, sizeof( liqsketchfont ));
	self->usagecount=1;
	return self;
}

void liqsketchfont_hold(liqsketchfont *self)
{
	// use this to hold onto an object which someone else created
	self->usagecount++;
}

void liqsketchfont_release(liqsketchfont *self)
{
	// use this when you are finished with an object
	self->usagecount--;
	if(!self->usagecount) liqsketchfont_free(self);
}

void liqsketchfont_free(liqsketchfont *self)
{
	// never call _free directly, outside sources should _release the object
	int idx;
	for(idx=0;idx<256;idx++)
	{
		if(self->glyphs[idx])
		{
			liqsketch_release(self->glyphs[idx]);
			self->glyphs[idx] = NULL;
		}
	}
	free(self);
}




