

#include <stdlib.h>
#include <stdio.h>

#include <memory.h>
#include <math.h>

#include "liqcliprect.h"
#include "liq_xsurface.h"
#include "liqapp.h"
#include "liqapp_prefs.h"
#include "liqcell_easyrun.h"
#include "liqcell_easypaint.h"
#include "liqcanvas.h"
#include "liqimage.h"
#include "liqsketch.h"
#include "liqaccel.h"

#ifdef __cplusplus
extern "C" {
#endif

//############################################### costable
#define PI 3.14159265358979323846f

#define costable_size      (512)
#define costable_size_div2 (costable_size / 2)
#define costable_size_div4 (costable_size / 4)
#define costable_size_last (costable_size - 1)

float   costable[ costable_size ];
int     costable_fudged[ costable_size ];
int     costable_ready = 0;
#define costable_fudge 1024

inline float costable_getcos(float n)
{
	float f = n * costable_size_div2 / PI;
	int i = f;
	if (i < 0) i=-i;
	return costable[ (i + costable_size_div4) % costable_size_last];
}

inline float costable_getsin(float n)
{
	float f = n * costable_size_div2 / PI;
	int i = f;
	if (i < 0)
		return costable[ -((-i) % costable_size_last) + costable_size  ];
	else
		return costable[ (i) % costable_size_last];
}

inline float costable_fudgegetcos(float n)
{
	float f = n * costable_size_div2 / PI;
	int i = f;
	if (i < 0) i=-i;
	return costable_fudged[ (i + costable_size_div4) % costable_size_last];
}

inline float costable_fudgegetsin(float n)
{
	float f = n * costable_size_div2 / PI;
	int i = f;
	if (i < 0)
		return costable_fudged[ -((-i) % costable_size_last) + costable_size  ];
	else
		return costable_fudged[ (i) % costable_size_last];
}


void costable_init()
{
	if(costable_ready)return;
	int i;
	for (i=0;i<costable_size;i++)
	{
		costable[i] = (float)sin(i * PI / costable_size_div2);
		costable_fudged[i] = costable_fudge * sin(i * PI / costable_size_div2);
	}
	costable_ready=1;

}

//###############################################

				// stupendously slow proof of concept


				void matrot_slow( int cx,int cy, int *px,int *py,float angle )
				{
					if(!angle)return;	// all ok
					float p1x = *px - cx;
					float p1y = *py - cy;
					// finally rotating
					
					*px = cx + p1x * cos(angle) - p1y * sin(angle);
					*py = cy + p1x * sin(angle) + p1y * cos(angle);
					
				}
				
				// make it faster
				// improvise however required :)
				

				void matrot_mid( int cx,int cy, int *px,int *py,float angle )
				{
					if(!angle)return;	// all ok
					float p1x = *px - cx;
					float p1y = *py - cy;
					// finally rotating
					
					*px = cx + p1x * costable_getcos(angle) - p1y * costable_getsin(angle);
					*py = cy + p1x * costable_getsin(angle) + p1y * costable_getcos(angle);
					
				}
		
		 		// maybe a bit buggy, testing previous version
				// seems stable now :) and fast enough
				// it was reading the accelerometer so often
				
				void matrot( int cx,int cy, int *px,int *py,float angle )
				{
					if(!angle)return;	// all ok
					int p1x = *px - cx;
					int p1y = *py - cy;
					// finally rotating
					
					*px = cx + (p1x * costable_fudgegetcos(angle) / costable_fudge) - (p1y * costable_fudgegetsin(angle) / costable_fudge);
					*py = cy + (p1x * costable_fudgegetsin(angle) / costable_fudge) + (p1y * costable_fudgegetcos(angle) / costable_fudge);
					
				}
		
//##################################################################
//##################################################################
//##################################################################


liqcliprect *liqcliprect_newfromimage(			liqimage *surface)
{
	liqcliprect *self = liqcliprect_new();
	if(self==NULL) {  liqapp_errorandfail(-1, "liqcliprect new failed" ); return NULL; }

	//liqapp_log("liqcliprect newfromimage");
	self->sx=0;
	self->sy=0;
	self->ex=surface->width-1;
	self->ey=surface->height-1;

	self->surface=   liqimage_hold(surface);
	return self;
}

//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################



liqcliprect *liqcliprect_new()
{
	liqcliprect *self = (liqcliprect *)calloc(sizeof(liqcliprect),1);
	if(self==NULL) {  liqapp_errorandfail(-1, "liqcliprect new failed" ); return NULL; }
	self->usagecount=1;
	return self;
}

liqcliprect * liqcliprect_hold(liqcliprect *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}

void liqcliprect_release(liqcliprect *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqcliprect_free(self);
}

void liqcliprect_free(liqcliprect *self)
{
	//liqapp_log("liqcliprect free");
	//liqcliprect_clear(self);
	if(self->surface){ liqimage_release(self->surface);  self->surface=NULL; }
	free(self);
}



//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################


int liqcliprect_isvalid(liqcliprect *self)
{
	if(self->sx>=self->ex)return 0;
	if(self->sy>=self->ey)return 0;
	return 1;
}

void liqcliprect_shrink(liqcliprect *self,int sx,int sy,int ex,int ey)
{
	// given a liqcliprect and another rectangle,
	if(sx > self->sx) self->sx=sx;
	if(sy > self->sy) self->sy=sy;
	if(ex < self->ex) self->ex=ex;
	if(ey < self->ey) self->ey=ey;
	if(self->ex<self->sx)self->ex=self->sx;
	if(self->ey<self->sy)self->ey=self->sy;
}
void liqcliprect_copy(liqcliprect *self,liqcliprect *other)
{
	self->sx=other->sx;
	self->sy=other->sy;
	self->ex=other->ex;
	self->ey=other->ey;
	self->surface=liqimage_hold(other->surface);
}

/*
inline int liqcliprect_rectcheckinside(liqcliprect *self,int sx,int sy,int ex,int ey)
{
	if(sx >= self->sx &&
	   sy >= self->sy &&
	   ex <= self->ex &&
	   ey <= self->ey)
		return 1;
	else
		return 0;
}

inline int liqcliprect_pointcheckinside(liqcliprect *self,int x,int y)
{
	if(x >= self->sx &&
	   y >= self->sy &&
	   x <= self->ex &&
	   y <= self->ey)
		return 1;
	else
		return 0;
}
 */

void liqcliprect_print(liqcliprect *self,char *prefix)
{
	liqapp_log("clip %s : %i %i : %i %i",prefix,self->sx,self->ex,self->sy,self->ey);
}



int    	liqcliprect_getx(liqcliprect *self)
{
	return self->sx;
}
int    	liqcliprect_gety(liqcliprect *self)
{
	return self->sy;
}
int    	liqcliprect_getw(liqcliprect *self)
{
	return self->ex-self->sx;
}
int    	liqcliprect_geth(liqcliprect *self)
{
	return self->ey-self->sy;
}

//##################################################################
//################################################################## drawing functions
//##################################################################







void liqcliprect_drawclear(liqcliprect *self,unsigned char grey,unsigned char u,unsigned char v)
{
	// todo follow the clipregion rules tsk tsk
	if(self->sx==0 && self->sy==0 && self->ex==(self->surface->width-1) && self->ey==(self->surface->height-1))
		xsurface_drawclear_yuv(self->surface,grey,u,v);
	else
		liqcliprect_drawboxfillcolor(self,self->sx,self->sy,self->ex-self->sx,self->ey-self->sy,grey,u,v);

}



void 		liqcliprect_drawpsetcolor(			liqcliprect *self,int x, int y, unsigned char grey,unsigned char u,unsigned char v)
{
	xsurface_drawpset_yuv(self->surface,x,y,grey,u,v);
}


void liqcliprect_drawpgetcolor(      	liqcliprect *self,int x, int y, unsigned char *grey,unsigned char *u,unsigned char *v)
{
	xsurface_drawpget_yuv(self->surface,x,y,grey,u,v);

}






void liqcliprect_drawlinerowcolor(liqcliprect *self,int x1, int y1, int x2, int y2, unsigned char grey,unsigned char u,unsigned char v)
{
	// horizontal row x1..x2,y1
	if(y1<self->sy) return;
	if(y1>self->ey) return;
	if(x1>x2){int t=x1;x1=x2;x2=t;}
	if(x2<self->sx) return;
	if(x1>self->ex) return;
	if(x1<self->sx) x1=self->sx;
	if(x2>self->ex) x2=self->ex;
	// draw the line now cleanly and technically without any further bound checking - it is 100% inside the boundary
	xsurface_drawline_yuv(self->surface,x1,y1,x2,y1,grey,u,v);
}

void liqcliprect_drawlinecolcolor(liqcliprect *self,int x1, int y1,int x2, int y2, unsigned char grey,unsigned char u,unsigned char v)
{
	// vert column x1,y1..y2
	//grey=255;//return;
	//return;

	if(x1<self->sx) return;
	if(x1>self->ex) return;
	if(y1>y2){int t=y1;y1=y2;y2=t;}
	if(y2<self->sy) return;
	if(y1>self->ey) return;
	if(y1<self->sy) y1=self->sy;
	if(y2>self->ey) y2=self->ey;
	// draw the line now cleanly and technically without any further bound checking - it is 100% inside the boundary
	xsurface_drawline_yuv(self->surface,x1,y1,x1,y2,grey,u,v);
}

void liqcliprect_drawlinecolor(liqcliprect *self,int x1, int y1, int x2, int y2, unsigned char grey,unsigned char u,unsigned char v)
{
	if(x1==x2)
	{
		liqcliprect_drawlinecolcolor(self,x1,y1,x2,y2,grey,u,v);
		return;
	}
	if(y1==y2)
	{
		liqcliprect_drawlinerowcolor(self,x1,y1,x2,y2,grey,u,v);
		return;
	}
	// full line x1..x2,y1..y2
	if(x1<self->sx) return;
	if(x1>self->ex) return;
	if(y1<self->sy) return;
	if(y1>self->ey) return;

	if(x2<self->sx) return;
	if(x2>self->ex) return;
	if(y2<self->sy) return;
	if(y2>self->ey) return;
	// draw the line now cleanly and technically without any further bound checking - it is 100% inside the boundary
	xsurface_drawline_yuv(self->surface,x1,y1,x2,y2,grey,u,v);
}

void		liqcliprect_drawthicklinecolor(		liqcliprect *self,int x1, int y1,int x2, int y2, unsigned char thickness, unsigned char grey,unsigned char u,unsigned char v)
{
	if(thickness<=1)
	{
		
		liqcliprect_drawlinecolor(self,x1,y1,x2,y2,grey,u,v);
		return;
	}
	// full line x1..x2,y1..y2
	if(x1<self->sx) return;
	if(x1>self->ex) return;
	if(y1<self->sy) return;
	if(y1>self->ey) return;

	if(x2<self->sx) return;
	if(x2>self->ex) return;
	if(y2<self->sy) return;
	if(y2>self->ey) return;
	// draw the line now cleanly and technically without any further bound checking - it is 100% inside the boundary
	xsurface_drawthickline_yuv(self->surface,x1,y1,x2,y2,thickness,grey,u,v);
}

//##################################################################
//##################################################################
//##################################################################

void liqcliprect_drawboxlinecolor(liqcliprect *self,int x,int y,int w,int h,unsigned char grey,unsigned char u,unsigned char v)
{
	
//	if(x&1){x--;w++; }
//	if((x+w)&1){ w++; }
int r=x+w-1;
int b=y+h-1;
		liqcliprect_drawlinerowcolor(self,x,y,  r,y,grey,u,v);
		liqcliprect_drawlinerowcolor(self,x,b,  r,b,grey,u,v);
		liqcliprect_drawlinecolcolor(self,x,y,  x,b,grey,u,v);
		liqcliprect_drawlinecolcolor(self,r,y,  r,b,grey,u,v);
}



//##################################################################
//##################################################################
//##################################################################

void liqcliprect_drawboxfillcolor(liqcliprect *self,int x,int y,int w,int h,unsigned char grey,unsigned char u,unsigned char v)
{
	//if(w<=0)return;
	//if(h<=0)return;
	if(w<0){ x+=w;w=-w;}
	if(h<0){ y+=h;h=-h;}
int r=(x+w)-1;
int b=(y+h)-1;
	if(x<self->sx)x=self->sx;
	if(y<self->sy)y=self->sy;
	if(r>self->ex)r=self->ex;
	if(b>self->ey)b=self->ey;
	if(r&1)r++;
	if(b&1)b++;
	//canvas_drawrectcolor(x,y,(r-x)+1,(b-y)+1,grey,u,v);

	xsurface_drawrect_yuv(self->surface,x,y,(r-x)+1,(b-y)+1,grey,u,v);
}


void 		liqcliprect_drawboxfillblendcolor(	liqcliprect *self,int x,int y,int w,int h,unsigned char grey,unsigned char u,unsigned char v,unsigned char blend)
{
	//if(w<=0)return;
	//if(h<=0)return;
	if(w<0){ x+=w;w=-w;}
	if(h<0){ y+=h;h=-h;}
int r=(x+w)-1;
int b=(y+h)-1;
	if(x<self->sx)x=self->sx;
	if(y<self->sy)y=self->sy;
	if(r>self->ex)r=self->ex;
	if(b>self->ey)b=self->ey;
	if(r&1)r++;
	if(b&1)b++;
	//canvas_drawrectcolor(x,y,(r-x)+1,(b-y)+1,grey,u,v);

	xsurface_drawfadeoutrect_yuv(self->surface,x,y,(r-x)+1,(b-y)+1,grey,u,v,blend);
}








void liqcliprect_drawboxwashcolor(liqcliprect *self,int x,int y,int w,int h,unsigned char u,unsigned char v)
{
	//if(w<=0)return;
	//if(h<=0)return;
	if(w<0){ x+=w;w=-w;}
	if(h<0){ y+=h;h=-h;}
int r=(x+w)-1;
int b=(y+h)-1;
	if(x<self->sx)x=self->sx;
	if(y<self->sy)y=self->sy;
	if(r>self->ex)r=self->ex;
	if(b>self->ey)b=self->ey;
	if(r&1)r++;
	if(b&1)b++;
	//canvas_drawrectcolor(x,y,(r-x)+1,(b-y)+1,grey,u,v);

	xsurface_drawrectwash_uv(self->surface,x,y,(r-x)+1,(b-y)+1,u,v);
}


void liqcliprect_drawboxfadeoutcolor(liqcliprect *self,int x,int y,int w,int h,unsigned char grey,unsigned char u,unsigned char v,unsigned char spread)
{
	//if(w<=0)return;
	//if(h<=0)return;
	if(w<0){ x+=w;w=-w;}
	if(h<0){ y+=h;h=-h;}
int r=(x+w)-1;
int b=(y+h)-1;
	if(x<self->sx)x=self->sx;
	if(y<self->sy)y=self->sy;
	if(r>self->ex)r=self->ex;
	if(b>self->ey)b=self->ey;
	if(r&1)r++;
	if(b&1)b++;
	//canvas_drawrectcolor(x,y,(r-x)+1,(b-y)+1,grey,u,v);

	//xsurface_drawrect_yuv(self->surface,x,y,(r-x)+1,(b-y)+1,grey,u,v);
	xsurface_drawfadeoutrect_yuv(self->surface,x,y,(r-x)+1,(b-y)+1,grey,u,v,spread);

}


void liqcliprect_drawcolorcube(liqcliprect *self,int x,int y,int w,int h,unsigned char grey)
{
	//if(w<=0)return;
	//if(h<=0)return;
	if(w<0){ x+=w;w=-w;}
	if(h<0){ y+=h;h=-h;}
int r=(x+w)-1;
int b=(y+h)-1;

	if(x<self->sx)x=self->sx;
	if(y<self->sy)y=self->sy;
	if(r>self->ex)r=self->ex;
	if(b>self->ey)b=self->ey;
	if(r&1)r++;
	if(b&1)b++;
    
    //xsurface_drawrect_grey(self->surface,x,y,w,h,grey);
	


	int tw=(r-x)/16;
	int th=(b-y)/16;
	int u;
	int v;
    while((tw*16)<w)tw++;
    while((th*16)<h)th++;
	//xsurface_drawrect_yuv(self->surface,x,y,(r-x)+1,(b-y)+1,grey,u,v);
	for(v=1;v<255;v+=16)
	{
		int xx=x;
		for(u=1;u<255;u+=16)
		{
			xsurface_drawrect_yuv(self->surface,xx,y,tw+1,th+1,grey,u,v);
			xx+=tw;
		}
		y+=th;
	}
}

void liqcliprect_drawgreycol(liqcliprect *self,int x,int y,int w,int h)
{
	//if(w<=0)return;
	//if(h<=0)return;
	if(w<0){ x+=w;w=-w;}
	if(h<0){ y+=h;h=-h;}
int r=(x+w)-1;
int b=(y+h)-1;

	if(x<self->sx)x=self->sx;
	if(y<self->sy)y=self->sy;
	if(r>self->ex)r=self->ex;
	if(b>self->ey)b=self->ey;
	if(r&1)r++;
	if(b&1)b++;
    
    
    
    
    

	int tw=(r-x)/16;
	int th=(b-y)/16;
    while((tw*16)<w)tw++;
    while((th*16)<h)th++;
    
    int gg;
    
	//xsurface_drawrect_yuv(self->surface,x,y,(r-x)+1,(b-y)+1,grey,u,v);
    for(gg=0;gg<=255;gg+=16)
	//for(v=1;v<255;v+=16)
	{
        xsurface_drawrect_yuv(self->surface,x,y,w,th+1,gg,128,128);
        
		//int xx=x;
		//for(u=1;u<255;u+=16)
		//{
		//	xsurface_drawrect_yuv(self->surface,xx,y,tw+1,th+1,grey,u,v);
		//	xx+=tw;
		//}
		y+=th;
	}

}





//##################################################################
//##################################################################
//##################################################################



void liqcliprect_drawglyph_grey(liqcliprect *self,liqfont *font,int x,int y,unsigned char glyph)
{
liqfontglyph *g = liqfont_getglyph(font,glyph);
	if(!g)return;
	//if(font->glyphdata[glyph]==NULL) return;
	int gw  =g->glyphw; // font->glyphwidths[glyph];
	int gh  =g->glyphh; // font->glyphheights[glyph];

	int gtw = gw;
	int sw  = self->surface->width;
	//int sh  = self->surface->height;
	unsigned int goff = 0;
	int xu=x-self->sx;
	if(xu<0)	//x<0
	{
		if(xu<-gw) return;
		gw+=xu;
		goff-=xu;
		x=self->sx;
	}
	unsigned int gskip = gtw-gw;
	if(y+gh<self->sy) return;



	if(x+gw>self->ex)
	{
		if(x>=self->ex) return;
		gskip+=(x+gw)-(self->ex);
		gw=(self->ex-x);
	}

	if(y+gh>self->ey)
	{
		if(y>=self->ey) return;
		gh=(self->ey-y);
	}
	unsigned int poff = sw * y + x;
	unsigned int pskip = sw-gw;
//---------------------------------------
	unsigned char *pdata;
	unsigned char *gdata;

	int yu=y-self->sy;

	if(yu<0)
	{
		yu=-yu;
		goff+=gtw*yu;
		poff+=sw*yu;
		gh-=yu;
		y=self->sy;
	}
	gdata = & ((unsigned char*)g->glyphdata) [ goff ] ;
	pdata =  ((unsigned char*)&self->surface->data[ self->surface->offsets[0] + poff ]);
	xsurface_drawstrip_or(gh,gw,gdata,pdata,gskip,pskip);


	//xsurface_drawstrip_colortest1(gh,gw,gdata,pdata,gskip,pskip,      self->surface, 100,y % 255,x % 255,      x,y);

}




//########################################################################
//######################################################################## draw text as fast as possible onto xv surface :)
//########################################################################

int liqcliprect_drawtext(liqcliprect *self,liqfont *font,int xs,int ys,char *data)
{
	int x=xs;
	unsigned char ch;
	while ( (ch=*data++) )
	{
		liqcliprect_drawglyph_grey(self,font,x,ys, ch );
		x+= liqfont_getglyphwidth(font,ch);//  font->glyphwidths[ch];
	}
	return x;
}

int liqcliprect_drawtextn(liqcliprect *self,liqfont *font,int xs,int ys,char *data,int datalen)
{
	int x=xs;
	unsigned char ch;
	if(datalen<=0)return x;
	while(datalen--)
	{
		ch=*data++;
		liqcliprect_drawglyph_grey(self,font,x,ys, ch );
		x+=liqfont_getglyphwidth(font,ch);//font->glyphwidths[ch];
	}
	return x;
}

void liqcliprect_drawtextcentredon(liqcliprect *self,liqfont *font,int cx,int cy,char *text)
{
	int tw=liqfont_textwidth(font,text);
	liqcliprect_drawtext(self,font,cx-tw/2,cy-liqfont_textheight(font)/2,text);
}

void liqcliprect_drawtextcentredonlimit(liqcliprect *self,liqfont *font,int cx,int cy,char *text,int availablewidth)
{
	// draw some text within a boundary
	int tcnt=liqfont_textfitinside(font,text,availablewidth);
	int tw=liqfont_textwidthn(font,text,tcnt);
	liqcliprect_drawtextn(self,font,cx-tw/2,cy-liqfont_textheight(font)/2,text,tcnt);
}


void liqcliprect_drawtextinside(liqcliprect *self,liqfont *font,int x,int y,int w,int h,char *text,int alignx)
{
	// tiny bit of optimising possible
	int tcnt=strlen(text);//liqfont_textfitinside(font,text,w+1);
	int tw=liqfont_textwidthn(font,text,tcnt);
	int cx=x+(w/2);
	int cy=y+(h/2);
	if(alignx==0)
	{
		liqcliprect_drawtextn(self,font,x,cy-liqfont_textheight(font)/2,text,tcnt);
	}
	else if(alignx==1)
	{
		liqcliprect_drawtextn(self,font,cx-tw/2,cy-liqfont_textheight(font)/2,text,tcnt);
	}
	else
	{
		liqcliprect_drawtextn(self,font,x+w-tw,cy-liqfont_textheight(font)/2,text,tcnt);

	}
}



//########################################################################
//######################################################################## draw text as fast as possible onto xv surface :) (color is a bit slower)
//########################################################################


void liqcliprect_drawglyph_color(liqcliprect *self,liqfont *font,int x,int y,unsigned char glyph,unsigned char grey,unsigned char u,unsigned char v)
{
	if((grey==255) && (u==128) && (v==128)){ liqcliprect_drawglyph_grey(self,font,x,y,glyph); return; }

liqfontglyph *g = liqfont_getglyph(font,glyph);
	if(!g)return;
	//if(font->glyphdata[glyph]==NULL) return;
	int gw  =g->glyphw; // font->glyphwidths[glyph];
	int gh  =g->glyphh; // font->glyphheights[glyph];


	int gtw = gw;
	int sw  = self->surface->width;
	//int sh  = self->surface->height;
	unsigned int goff = 0;
	int xu=x-self->sx;
	if(xu<0)	//x<0
	{
		if(xu<-gw) return;
		gw+=xu;
		goff-=xu;
		x=self->sx;
	}
	unsigned int gskip = gtw-gw;
	if(y+gh<self->sy)return;



	if(x+gw>self->ex)
	{
		if(x>=self->ex)return;
		gskip+=(x+gw)-(self->ex);
		gw=(self->ex-x);
	}

	if(y+gh>self->ey)
	{
		if(y>=self->ey) return;
		gh=(self->ey-y);
	}
	unsigned int poff = sw * y + x;
	unsigned int pskip = sw-gw;
//---------------------------------------
	unsigned char *pdata;
	unsigned char *gdata;

	int yu=y-self->sy;

	if(yu<0)
	{
		yu=-yu;
		goff+=gtw*yu;
		poff+=sw*yu;
		gh-=yu;
		y=self->sy;
	}
	gdata = & ((unsigned char*)g->glyphdata) [ goff ] ;
	pdata =  ((unsigned char*)&self->surface->data[ self->surface->offsets[0] + poff ]);
	//xsurface_drawstrip_or(gh,gw,gdata,pdata,gskip,pskip);


	xsurface_drawstrip_colortest1(gh,gw,gdata,pdata,gskip,pskip,      self->surface, grey,u,v,      x,y);

}


int liqcliprect_drawtext_color(liqcliprect *self,liqfont *font,int xs,int ys,char *data,unsigned char grey,unsigned char u,unsigned char v)
{
	if(font->rotation==0 || font->rotation==180)
	{
		int x=xs;
		unsigned char ch;
		while ( (ch=*data++) )
		{
			liqcliprect_drawglyph_color(self,font,x,ys, ch , grey,u,v);
			x+=liqfont_getglyphwidth(font,ch);//font->glyphwidths[ch];
		}
		return x;
	}
	else
	{
		int y=ys;
		unsigned char ch;
		while ( (ch=*data++) )
		{
			liqcliprect_drawglyph_color(self,font,xs,y, ch , grey,u,v);
			y+=liqfont_getglyphheight(font,ch);//font->glyphwidths[ch];
		}
		return y;
	}
}

int liqcliprect_drawtextn_color(liqcliprect *self,liqfont *font,int xs,int ys,char *data,int datalen,unsigned char grey,unsigned char u,unsigned char v)
{
	if(font->rotation==0 || font->rotation==180)
	{	
		int x=xs;
		unsigned char ch;
		if(datalen<=0)return x;
		while(datalen--)
		{
			ch=*data++;
			liqcliprect_drawglyph_color(self,font,x,ys, ch , grey,u,v );
			x+=liqfont_getglyphwidth(font,ch);//font->glyphwidths[ch];
		}
		return x;
	}
	else
	{
		int y=ys;
		unsigned char ch;
		if(datalen<=0)return y;
		while(datalen--)
		{
			ch=*data++;
			liqcliprect_drawglyph_color(self,font,xs,y, ch , grey,u,v );
			y+=liqfont_getglyphheight(font,ch);//font->glyphwidths[ch];
		}
		return y;
	}
}

void liqcliprect_drawtextcentredon_color(liqcliprect *self,liqfont *font,int cx,int cy,char *text,unsigned char grey,unsigned char u,unsigned char v)
{
	int tw=liqfont_textwidth(font,text);
	liqcliprect_drawtext_color(self,font,cx-tw/2,cy-liqfont_textheight(font)/2,text, grey,u,v);
}

void liqcliprect_drawtextcentredonlimit_color(liqcliprect *self,liqfont *font,int cx,int cy,char *text,int availablewidth,unsigned char grey,unsigned char u,unsigned char v)
{
	// draw some text within a boundary
	int tcnt=liqfont_textfitinside(font,text,availablewidth);
	int tw=liqfont_textwidthn(font,text,tcnt);
	liqcliprect_drawtextn_color(self,font,cx-tw/2,cy-liqfont_textheight(font)/2,text,tcnt, grey,u,v);
}


void liqcliprect_drawtextinside_color(liqcliprect *self,liqfont *font,int x,int y,int w,int h,char *text,int alignx,unsigned char grey,unsigned char u,unsigned char v)
{
	// tiny bit of optimising possible
	int tcnt=strlen(text);//liqfont_textfitinside(font,text,w+1);
	int tw=liqfont_textwidthn(font,text,tcnt);
	int cx=x+(w/2);
	int cy=y+(h/2);
	if(alignx==0)
	{
		liqcliprect_drawtextn_color(self,font,x,cy-liqfont_textheight(font)/2,text,tcnt, grey,u,v);
	}
	else if(alignx==1)
	{
		liqcliprect_drawtextn_color(self,font,cx-tw/2,cy-liqfont_textheight(font)/2,text,tcnt, grey,u,v);
	}
	else
	{
		liqcliprect_drawtextn_color(self,font,x+w-tw,cy-liqfont_textheight(font)/2,text,tcnt, grey,u,v);

	}
}




//########################################################################
//######################################################################## draw page as fast as possible onto xv surface :)
//########################################################################


int _liqcliprect_recursion_depth=0;

void liqcliprect_drawsketch(liqcliprect *self,liqsketch *page,int l,int t,int w,int h,int drawmode)	// 0=preview, 1=detailed, 2=latest point only 4=no aspect
{
	
	if(!page)return;
	if(l+w<self->sx || t+h<self->sy) return;
	if(l>=self->ex) return;
	if(t>=self->ey) return;
	if(w<2 || h<2) return;		// dont be silly :)
	
	if(drawmode==5)drawmode=8;

	// no point in displaying the entire blank frame to the user,
	// so we specify only the used rectangle within the page

	int fmx;
	int fmy;

	int fox;
	int foy;


	// From
	if(drawmode)
	{
		fox = 0;
		foy = 0;
		fmx = page->pixelwidth;
		fmy = page->pixelheight;
		if(fmx==0 || fmy==0) return;

	}
	else
	{
		fox = page->boundingbox.xl;
		foy = page->boundingbox.yt;
		fmx = page->boundingbox.xr - page->boundingbox.xl;
		fmy = page->boundingbox.yb - page->boundingbox.yt;
		if(fmx==0 || fmy==0) return;

	}


	// required for rotation
	float angle;
	if(drawmode)
	{
		angle=0;
	}
	else
	{
		if(liqcell_easyrun_autorotating)
			angle = page->angle + liqaccel_getangle();
		else
			angle = page->angle;
	}
	
	
	//liqapp_log("ang:   %-3.3f   loc:   %-3.3f",angle,angleloc);
			   
	
	costable_init();
	
	
	// FromCentre used for rotation to know the centre of the from used block
	int fcx = fox + fmx/2;
	int fcy = foy + fmy/2;




	int fmap2=fmx*fmx+fmy*fmy;


	// To
	int tmx = w-1; if(tmx<0)tmx=0;
	int tmy = h-1; if(tmy<0)tmy=0;
	
	
	if((drawmode & 1))
	{
		// for direct drawing, make sure we use the direct size of the available sketch :)
		tmx=page->pixelwidth;
		tmy=page->pixelheight;
	}
	
	if(tmy==0 || tmy==0) return;
	int tox = l;
	int toy = t;
	int tmap2 = tmx*tmx+tmy*tmy;

	float adx = (float)page->dpix / (float)self->surface->dpix;		// fixed to display canvas instead of being to surface
	float ady = (float)page->dpiy / (float)self->surface->dpiy;

	//================================ calc aspect ratio
	float ax = adx * (float)tmx / (float)fmx;
	float ay = ady * (float)tmy / (float)fmy;
	float ar = (ax<=ay ? ax : ay);

	int rx = (ar * (float)fmx)/adx;
	int ry = (ar * (float)fmy)/ady;

//	int fx = ((float)tmx / rx);
//	int fy = ((float)tmy / ry);



	liqapp_log("sk.want o(%i,%i) m(%i,%i) drawmode=%d",l,t,w,h,drawmode);
	liqapp_log("sk.from o(%i,%i) m(%i,%i)",fox,foy,fmx,fmy);
	liqapp_log("sk.to   o(%i,%i) m(%i,%i)",tox,toy,tmx,tmy);
	liqapp_log("sk.ax   %f,%f,%f",ax,ay,ar);
	liqapp_log("sk.r    %i,%i",rx,ry);

	

	//================================ push altered aspect result into tmxy

	// this silly little reduction causes problems with live drawing..
	// but it looks better for icons etc showing used area
	if(!drawmode)
	{
		rx=(float)rx*0.9;
		ry=(float)ry*0.9;
	}

	if((drawmode & 4)==0)
	{

		if(rx<tmx) tox+=(tmx-rx)/2;
		if(ry<tmy) toy+=(tmy-ry)/2;

		tmx = rx;
		tmy = ry;
		liqapp_log("eep");
	}
	
	

	
	
	// automatic quality reduction skip factor (high divisor==higher quality)
	//int rpt=(fmap2/tmap2)/4;// /4;


	//int rpt=(fmap2/tmap2)/8;// try this at high res, doubt it will work
	//liqapp_log("adx*ady=%f",(adx*ady));


	//gb:18oct2008:set this too high, reverting
	//int rpt=((int)((float)(fmap2/tmap2)/(adx*ady))) >>3;// try this at high res, doubt it will work



	// this is jagged on 810 desktop minimal
	//int rpt=(fmap2/tmap2)/8;

	// 20090615_235452 lcuk :  this is too low now I changed the size of the canvas to be by default smaller
	//int rpt=(fmap2/tmap2)/16;
	
	// 20090615_235500 lcuk : should find something that works in all cases
	//int rpt=(fmap2/tmap2)/32;


	// 20090712_021132 lcuk : again, a change, the algo needs properly dealing with
	// 20090712_021154 lcuk : next time you call here, make the time to do it
	//int rpt=(fmap2/tmap2)/16;
	
	
	
	// 20090712_145556 lcuk : lets try something radical
	int rpt=(fmap2/tmap2)/8;
	
	
	
	// Sun Aug 23 12:41:07 2009 lcuk : fuckit, try full res always and see
	if(liqapp_hardware_product_ispowerful_get() && (angle==0))
		rpt=1;

	rpt=0;
	

	switch(page->backgroundstyle)
	{


	//	case 0:		// none
	//					break;


		case 1:		// solid
			{
						//unsigned char *yuva = (unsigned char *)&sketch->backgroundcolor;
						{
						//	app_log("back");
						//	vcliprect_drawboxfillcolor( canvas.cr,   tox,toy,  tmx,tmy ,yuva[0],yuva[1],yuva[2]);
						}
						break;
			}


		case 2:		// image
			{
				// todo
				// there is a special case scenario with both image and sketch which needs catering for
				// when I am showing the preview of the modified section of the current sketch
				// the background sketch or image must be scaled and positioned correctly to lie over the
				// right piece of the image
				// this special mode will be indicated by the sketch itself and the background content
				// having identical dimensions
				// other

						if(!page->backgroundimage && page->backgroundfilename)
						{
							// todo: auto attempt load on render?
							//page->backgroundimage = vimage_newfromlibrary(page->backgroundfilename);
						}


						if(page->backgroundimage)
						{
							//vgraph_drawimage( 	 self, tox,toy,  tmx,tmy, page->backgroundimage);
							liqcliprect_drawimagecolor(self ,page->backgroundimage,tox,toy,  tmx,tmy,0);
						}
						break;
			}

		case 3:		// sketch
			{
						if(!page->backgroundsketch && page->backgroundfilename)
						{
							// todo: auto attempt load on render?
							//page->backgroundsketch = vsketch_newfromlibrary(sketch->backgroundfilename);
						}


						if(page->backgroundsketch)
						{
							liqcliprect_drawsketch(self,page->backgroundsketch,tox,toy,  tmx,tmy,0);
						}
						break;
			}

	}


						if(page->backgroundimage)
						{
							//vgraph_drawimage( 	 self, tox,toy,  tmx,tmy, page->backgroundimage);

							//liqcliprect_drawcolorcube(self ,tox,toy,  tmx,tmy,128);

							//liqcliprect_drawimagecolor(self ,page->backgroundimage,tox,toy,  tmx,tmy,0);

						}

	// For every point I am drawing from the Page
	// I must Subtract FO From Offset
	// I must then Divide by FM From Magnitude
	// Then Multiply by TM To Magnitude
	// Then add TO To Offset
	if(drawmode)rpt=0;

	// optimize: combine t/f into a single variable up here.  reason against, it would need to be a float?
	// todo: ensure we render at correct aspect ratio, we can just adjust these parameters
	liqstroke *stroke=page->strokefirst;
	while(stroke)
	{
		liqstroke_hold(stroke);
		if(stroke->pointcount>=2)
		{
			unsigned char y=stroke->pen_y;
			unsigned char u=stroke->pen_u;
			unsigned char v=stroke->pen_v;

			// this is a good out from the dark function, but not for now..

			/*
			if(rpt>0)
			{
				y/=rpt;
				u=128+((u-128)/rpt);
				v=128+((v-128)/rpt);
			}
			*/
				//v=v-40;
				//if(v<0) v=255+v;

			liqpoint *p1;
			liqpoint *p2;

			int p1x;
			int p1y;

			int p2x;
			int p2y;

			int lsx;
			int lsy;
			int lex;
			int ley;


			int isselected=stroke->selected;





			//######################################## normal stroke
			if(stroke->strokekind==0)
			{

				if(1) // drawmode!=1)   //  =2 || drawmode==0)
				{
					p1 = stroke->pointfirst;
				}
				else
				{
					// start at end-1
					p1 = stroke->pointlast->linkprev;
					if(p1->linkprev)p1=p1->linkprev;
					if(p1->linkprev)p1=p1->linkprev;
				}
				//p1 = stroke->pointfirst;


				p1x=p1->x;	// rotate now..
				p1y=p1->y;
				
				// finally rotating
				matrot(fcx,fcy, &p1x,&p1y,angle);



				p2 = p1->linknext;
				while(p2)
				{

					p2x=p2->x;   // rotate now..
					p2y=p2->y;

					// finally rotating
					matrot(fcx,fcy, &p2x,&p2y,angle);


					// the heavy math part is here...
					lsx=tox+((p1x-fox)*tmx/fmx);
					lsy=toy+((p1y-foy)*tmy/fmy);
					lex=tox+((p2x-fox)*tmx/fmx);
					ley=toy+((p2y-foy)*tmy/fmy);
					
					// idea:
					// try rotating the target coords instead of the dest
					// the thinking here is that it gets me in thinking for the cliprect at the head of the function
					//matrot(tcx,tcy, &lxs,&lsy,angle);
					//matrot(tcx,tcy, &les,&ley,angle);


					// high definition color scaling :)
					int g=(450-p1->z);
					g=(g*256) / 250;
					if(g<0)g=0;
					if(g>255)g=255;
					float f=(float)g / 256;
					float fy=y;
					float fu=u;
					float fv=v;
					unsigned char sy=	    (char)(      f * (fy    )) ;
					unsigned char su=	    (char)(128 + f * (fu-128)) ;
					unsigned char sv=	    (char)(128 + f * (fv-128)) ;
					
					
					if(p2->linknext==NULL) liqapp_log("sk.line xy(%d,%d)-xy(%d,%d)",lsx,lsy,lex,ley);
					
					
					liqcliprect_drawlinecolor(self,lsx,lsy,lex,ley,  sy,su,sv);
					if(isselected) liqcliprect_drawlinecolor(self,lsx+1,lsy+1,lex+1,ley+1,    sy,su,sv);
					
					
					//liqcliprect_drawthicklinecolor(self,lsx,lsy,lex,ley, 5, sy,su,sv);
					//if(isselected) liqcliprect_drawthicklinecolor(self,lsx+1,lsy+1,lex+1,ley+1, 5,   sy,su,sv);
					
					
					
					
					//stroke->pen_thick = 4;		// fakey!
					//int thick_ratio = (float)stroke->pen_thick * (fmap2/tmap2);
					//liqapp_log("thick %i,  %3.3f %3.3f %3.3f",thick_ratio, fmap2,tmap2,(fmap2/tmap2));
					//if(isselected) thick_ratio+=2;
					//liqcliprect_drawthicklinecolor(self,lsx,lsy,lex,ley,  thick_ratio,  sy,su,sv);




					//liqcliprect_drawlinecolor(self,lsx,lsy,lex,ley,    y,u,v);

					//canvas_line(lsx,lsy,lex,ley,    y);

					p1=p2;
					p2=p2->linknext;

					p1x=p2x;
					p1y=p2y;

					if(p2 && rpt)
					{	// move us on, at this point, rendering half resolution is ok :)
						//todo: make this work properly, dropoff with scale is too steep
						int cnt=rpt;
						while(p2->linknext && cnt-->0)
							p2=p2->linknext;
						// i could do this by a factor of anything and reduce resolution as difference increases
					}
				}
			}
			//######################################## crows nest line
			else if(stroke->strokekind==1)
			{
				p1 = stroke->pointfirst;
				p1x=p1->x;	// rotate now..
				p1y=p1->y;

				p2 = stroke->pointlast;
				p2x=p2->x;   // rotate now..
				p2y=p2->y;

				// the heavy math part is here...
				lsx=tox+((p1x-fox)*tmx/fmx);
				lsy=toy+((p1y-foy)*tmy/fmy);
				lex=tox+((p2x-fox)*tmx/fmx);
				ley=toy+((p2y-foy)*tmy/fmy);

				liqcliprect_drawlinecolor(self,lsx,lsy,lex,ley,    y,u,v);
			}

			//######################################## simple box outline
			else if(stroke->strokekind==2)
			{
				p1 = stroke->pointfirst;
				p1x=p1->x;	// rotate now..
				p1y=p1->y;

				p2 = stroke->pointlast;
				p2x=p2->x;   // rotate now..
				p2y=p2->y;

				// the heavy math part is here...
				lsx=tox+((p1x-fox)*tmx/fmx);
				lsy=toy+((p1y-foy)*tmy/fmy);
				lex=tox+((p2x-fox)*tmx/fmx);
				ley=toy+((p2y-foy)*tmy/fmy);

				liqcliprect_drawboxlinecolor(self,lsx,lsy,lex-lsx,ley-lsy,    y,u,v);
			}

			//######################################## filled box
			else if(stroke->strokekind==3)
			{
				p1 = stroke->pointfirst;
				p1x=p1->x;	// rotate now..
				p1y=p1->y;

				p2 = stroke->pointlast;
				p2x=p2->x;   // rotate now..
				p2y=p2->y;

				// the heavy math part is here...
				lsx=tox+((p1x-fox)*tmx/fmx);
				lsy=toy+((p1y-foy)*tmy/fmy);
				lex=tox+((p2x-fox)*tmx/fmx);
				ley=toy+((p2y-foy)*tmy/fmy);

				liqcliprect_drawboxfillcolor(self,lsx,lsy,lex-lsx,ley-lsy,    y,u,v);
			}

			//######################################## subpage (gulp!)
			else if(stroke->strokekind==4)
			{
				//p1 = stroke->pointfirst;
				//p1x=p1->x;	// rotate now..
				//p1y=p1->y;

				//p2 = stroke->pointlast;
				//p2x=p2->x;   // rotate now..
				//p2y=p2->y;

				p1x=stroke->boundingbox.xl;
				p1y=stroke->boundingbox.yt;
				p2x=stroke->boundingbox.xr;
				p2y=stroke->boundingbox.yb;

				// the heavy math part is here...
				lsx=tox+((p1x-fox)*tmx/fmx);
				lsy=toy+((p1y-foy)*tmy/fmy);
				lex=tox+((p2x-fox)*tmx/fmx);
				ley=toy+((p2y-foy)*tmy/fmy);


				//if(_liqcliprect_recursion_depth<4)
				if((lex-lsx)>10 &&(ley-lsy)>10)
				{
					_liqcliprect_recursion_depth++;
					if(stroke->mediapage)
						liqcliprect_drawsketch(self,stroke->mediapage,lsx,lsy,lex-lsx,ley-lsy,0);
					_liqcliprect_recursion_depth--;
				}

				liqcliprect_drawboxlinecolor(self,lsx,lsy,lex-lsx,ley-lsy,    y,u,v);

			}


			// add other stroke types here :)








			// supposing we actually said this stroke was a BOX/Circle/Polygon/Triangle/Tile etc
			// what would we draw then?
		}
		
		liqstroke_release(stroke);
		stroke=stroke->linknext;
	}
}




int		liqcliprect_drawcell(      liqcliprect *self, liqcell *cell,   int x, int y, int w,int h       )
{

	liqcell_easypaint(cell,self,x,y,w,h);
	return 0;
}










































static float calcaspect(int captionw,int captionh,int availw,int availh)
{
	//todo: centralize this, its used in multiple places
	if(captionw==0)return 0;
	if(captionh==0)return 0;
	float ax = (float)availw / (float)captionw;
	float ay = (float)availh / (float)captionh;
	float ar = (ax<=ay ? ax : ay);
	return ar;

}








//########################################################################
//######################################################################## draw image as fast as possible onto xv surface :)
//########################################################################



void liqcliprect_drawimagecolor(		liqcliprect *self,liqimage *image,int x,int y,int w,int h,int aspectlock)
{

	if(!w || !h) return;
	if(w<0){ x+=w;w=-w;}
	if(h<0){ y+=h;h=-h;}
	//liqapp_log("drawimg d(%i,%i)-step(%i,%i)  i(%i,%i)-step(%i,%i)  ",x,y,w,h,    0,0,image->width,image->height,aspectlock);

	if(aspectlock)
	{
		// 20090408_001022 lcuk : adjust the requested boundary now to enforce rigid aspect lock
		float ar = calcaspect(image->width,image->height,w,h );
		int sw = image->width * ar;
		int sh = image->height * ar;
		x = x+(w-sw) / 2;
		y = y+(h-sh) / 2;
		w=sw;
		h=sh;
		if(!w || !h) return;
		// 20090408_001100 lcuk : then let it fall through to be adjusted by the boundary (which I know works from testing upto now)
	}


	// identify lower edge
int r=(x+w);
int b=(y+h);

	// identify the union of requested area and writable area
	liqcliprect cr;
	liqcliprect_copy(&cr,self);
	liqcliprect_shrink(&cr,x,y,r,b);
	liqimage_release(cr.surface);

	// check its valid
	int cw=(cr.ex+1)-cr.sx;
	int ch=(cr.ey+1)-cr.sy;
	if(!cw || !ch)return;

	// adjust the boundary to compensate for clipping

	int ix = 0;
	int iy = 0;
	int iw = image->width;
	int ih = image->height;

	if(cw<w)
	{
		iw = iw * cw/w ;
		if(x<cr.sx)
		{
			// we were clipped from the left
			ix = image->width * (cr.sx-x)/w;
		}
	}
	if(ch<h)
	{
		ih = ih *  ch / h;
		if(y<cr.sy)
		{
			// he here clipped from the top
			iy = image->height * (cr.sy-y)/h;
            ih--;
		}
	}

	int dw=image->width *cw/w;
	int dh=image->height*ch/h;
	int dx=image->width -dw;
	int dy=image->height-dh;
	if(!dw || !dh)return;


	dw=iw;
	dh=ih;
	dx=ix;
	dy=iy;



	//liqapp_log("dxy %i,%i dwh %i,%i    iwh %i,%i  cr(%i,%i wh %i,%i)",dx,dy,dw,dh,  image->width-1,image->height-1,cr.sx,cr.sy,cw,ch);

	if(w==0||h==0)return;
	xsurface_drawzoomimage( image,
								dx,dy,
								dw,dh,


							self->surface,
								//x,y,
								//w,h
								cr.sx,cr.sy,
								cw,ch
								);


}








void liqcliprect_drawimageblendcolor(		liqcliprect *self,liqimage *image,int x,int y,int w,int h,char blend,int aspectlock)
{

	// 20090408_002917 lcuk : same as drawimagecolor above, but for blended

	if(!w || !h) return;
	if(w<0){ x+=w;w=-w;}
	if(h<0){ y+=h;h=-h;}
	//liqapp_log("drawimg d(%i,%i)-step(%i,%i)  i(%i,%i)-step(%i,%i)  ",x,y,w,h,    0,0,image->width-1,image->height-1);

	if(aspectlock)
	{
		// 20090408_001022 lcuk : adjust the requested boundary now to enforce rigid aspect lock
		float ar = calcaspect(image->width,image->height,w,h );
		int sw = image->width * ar;
		int sh = image->height * ar;
		x = x+(w-sw) / 2;
		y = y+(h-sh) / 2;
		w=sw;
		h=sh;
		if(!w || !h) return;
		// 20090408_001100 lcuk : then let it fall through to be adjusted by the boundary (which I know works from testing upto now)
	}


	// identify lower edge
int r=(x+w);
int b=(y+h);

	// identify the union of requested area and writable area
	liqcliprect cr;
	liqcliprect_copy(&cr,self);
	liqcliprect_shrink(&cr,x,y,r,b);
	liqimage_release(cr.surface);

	// check its valid
	int cw=(cr.ex+1)-cr.sx;
	int ch=(cr.ey+1)-cr.sy;
	if(!cw || !ch)return;

	// adjust the boundary to compensate for clipping

	int ix = 0;
	int iy = 0;
	int iw = image->width;
	int ih = image->height;

	if(cw<w)
	{
		iw = iw * cw/w ;
		if(x<cr.sx)
		{
			// we were clipped from the left
			ix = image->width * (cr.sx-x)/w;
		}
	}
	if(ch<h)
	{
		ih = ih *  ch / h;
		if(y<cr.sy)
		{
			// he here clipped from the top
			iy = image->height * (cr.sy-y)/h;
            ih--;
		}
	}

	int dw=image->width *cw/w;
	int dh=image->height*ch/h;
	int dx=image->width -dw;
	int dy=image->height-dh;
	if(!dw || !dh)return;


	dw=iw;
	dh=ih;
	dx=ix;
	dy=iy;

	xsurface_drawzoomblendimage( image,
								dx,dy,
								dw,dh,


							self->surface,
								//x,y,
								//w,h
								cr.sx,cr.sy,
								cw,ch,

								blend
								);


/* this is just thoughts, and was going to implement this here to test making a 9block rounded edge blitter work
    // the aim is to allow a specified image to be used in a stretch corners concept
	// to keep rounded edges looking sharp and correct
	// but allowing a textured core area to stretch easily
	// test of the special renderer

	int iw=w ;
	int ih=h ;

	int gw=8;
	int gh=8;
	
	
	void blit(int cx,int cy,int cw,int ch)
	{
		cx+=x;
		cy+=y;
		//
		if(cx+cw < dx){ return; }
		if(cx    < dx){ cw-=(dx-cx); cx=dx; }
		if(cx    > dx+dw){ return; }
		if(cx+cw > dx+dw){ cw=cx-(dx+dw); }
		if(cy+ch < dy){ return; }
		if(cy    < dy){ ch-=(dy-cy); cy=dy; }
		if(cy    > dy+dh){ return; }
		if(cy+ch > dy+dh){ ch=cy-(dy+dh); }
		
		xsurface_drawzoomblendimage( image, cx,cy,cw,ch ,
							self->surface,
								//x,y,
								//w,h
								cr.sx,cr.sy,
								cw,ch,

								blend
								);		
	}



	blit(0,    0,      gw,      gh);
	blit(gw,   0,      iw-gw-gw,gh);
	blit(iw-gw,0,      gw,      gh);



	blit(0,    gh,      gw,      ih-gh-gh);
	blit(gw,   gh,      iw-gw-gw,gh-gh-gh);
	blit(iw-gw,gh,      gw,      gh-gh-gh);

	blit(0,    ih-gh,   gw,      gh);
	blit(gw,   ih-gh,   iw-gw-gw,gh);
	blit(iw-gw,ih-gh,   gw,      gh);
*/

}

#ifdef __cplusplus
}
#endif

