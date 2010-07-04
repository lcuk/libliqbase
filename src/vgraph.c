
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "liqcell.h"
#include "liqcell_easypaint.h"
#include "liqaccel.h"
#include "liqapp_prefs.h"
#include "vgraph.h"

// virtual graphics and drawing
// routines to manage display of a UI


// rotation: http://www.delphi3000.com/articles/article_1466.asp?SK=
// http://www.petesqbsite.com/sections/tutorials/graphics.shtml#5


#include "liqcliprect.h"
#include "liqcell.h"
	
#ifdef __cplusplus
extern "C" {
#endif	
	
	

vgraph *vgraph_new()
{
	// use this to allocate and hold onto a reference
	// should really overload this and allow variations..
	vgraph *self = (vgraph *)malloc(sizeof( vgraph ));
	memset((char *)self,0, sizeof( vgraph ));
	self->usagecount=1;
	return self;
}


vgraph *vgraph_hold(vgraph *self)
{
	// use this to hold onto an object which someone else created
	self->usagecount++;
	return self;
}



void vgraph_free(vgraph *self)
{
	// never call _free directly, outside sources should _release the object
	vgraph_setfont(self,NULL);
	vgraph_setwindow(self,NULL);
	vgraph_settarget(self,NULL);
	free(self);
}




void vgraph_release(vgraph *self)
{
	// use this when you are finished with an object
	self->usagecount--;
	if(!self->usagecount) vgraph_free(self);
	
	
	
}



void    vgraph_setcliprect(      vgraph *self, liqcliprect *cliprect )
{
	if(self->cliprect)
	{
		liqcliprect_release(self->cliprect);
	}
	self->cliprect=cliprect;
}

liqcliprect *vgraph_getcliprect(      vgraph *self)
{
	return self->cliprect;
}






void    vgraph_convert_target2window(vgraph *self, int tx,int ty,  int *wx, int *wy)
{
	
	
	
	
	if(self->targetw && self->targeth && self->windoww && self->windowh)
	{

		//ROTATEPATCHPOINT set isportrait as required
		int isportrait = (self->windowh > self->windoww) && ((self->targetx==0 && self->targety==0));
		
		if((isportrait))
		{
			// auto portrait mode ..
			*wx = ((self->targeth-1) - (ty - self->targety)) * self->windoww / self->targeth;
			*wy = (                    (tx - self->targetx)) * self->windowh / self->targetw;
			
			//liqapp_log("convert from (%3d,%3d)",tx,ty);
			//liqapp_log("convert to   (%3d,%3d)",*wx,*wy);
			return;
		}
		
		
		int x = ((tx - self->scalex) * self->windoww / self->scalew);
		int y = ((ty - self->scaley) * self->windowh / self->scaleh);
		*wx=x;
		*wy=y;
	}
	else
	{
		*wx=0;
		*wy=0;
	}
}

void    vgraph_convert_window2target(vgraph *self, int wx,int wy,  int *tx, int *ty)
{
	if(self->targetw && self->targeth && self->windoww && self->windowh)
	{
		int x = self->scalex + (wx * self->scalew / self->windoww);
		int y = self->scaley + (wy * self->scaleh / self->windowh);
		*tx=x;
		*ty=y;
	}
	else
	{
		*tx=0;
		*ty=0;
	}
}






static float calcaspect(int captionw,int captionh,int availw,int availh)
{
	// return a scalar which when applied to the caption(w/h) will result in a rectangle of maximum dimensions to fit inside avail(w/h)
	// int rw = captionw*aspect;
	// int rh = captionh*aspect;    
	if(captionw==0)return 0;
	if(captionh==0)return 0;
	float ax = (float)availw / (float)captionw;
	float ay = (float)availh / (float)captionh;
	float ar = (ax<=ay ? ax : ay);	
	return ar;
	
}


static void vgraph_font_setscale(vgraph *self)
{
	float a,b;
	a=(float)self->scalew/(float)self->windoww;
	b=(float)self->scaleh/(float)self->windowh;
	//liqapp_log("vgraph_font_setscale   a= %3.3f b=%3.3f  scale %d,%d  win %d,%d",a,b,self->scalew,self->scaleh, self->windoww,self->windowh);
	if(self->font) liqfont_setview(self->font,a,b);
}


static void vgraph_recalc(vgraph *self)
{
	// recalculate required settings to sit window neatly within target
	if(self->targetw && self->targeth && self->windoww && self->windowh)
	{
		float ar = calcaspect(self->windoww,self->windowh,self->targetw,self->targeth);
		
		
		if(self->scaleaspectlock)
		{
		
			self->scalew = self->windoww * ar;
			self->scaleh = self->windowh * ar;
			self->scalex = self->targetx+(self->targetw - self->scalew) / 2;
			self->scaley = self->targety+(self->targeth - self->scaleh) / 2;
		}
		else
		{
			self->scalew = self->targetw;
			self->scaleh = self->targeth;
			self->scalex = self->targetx;
			self->scaley = self->targety;
			
		}
		
		vgraph_font_setscale(self);


	/*
#ifdef HASWINDOW
if(self->window)
{
		liqapp_log("scale.WIN %s xy(%d,%d),wh(%d,%d)",self->window->name,
													  self->window->x,
													  self->window->y,
													  self->window->w,
													  self->window->h);
}
#endif
		liqapp_log("scale.win %i,%i",self->windoww,self->windowh);
		liqapp_log("scale.tar %i,%i",self->targetw,self->targeth);
		liqapp_log("scale.sxy %i,%i",self->scalex,self->scaley);
		liqapp_log("scale.swh %i,%i",self->scalew,self->scaleh);


		new batch of tests :)
		
		// designed for landscape
			02:03:10 [lcuk] scale.WIN liqbook xy(0,0),wh(800,480)
			02:03:10 [lcuk] scale.win 800,480
			02:03:10 [lcuk] scale.tar 800,480
			02:03:10 [lcuk] scale.sxy 0,0
			02:03:10 [lcuk] scale.swh 800,480

		// designed for portrait
			02:03:12 [lcuk] scale.WIN liqbookreader xy(0,0),wh(480,800)
			02:03:12 [lcuk] scale.win 480,800
			02:03:12 [lcuk] scale.tar 800,480
			02:03:12 [lcuk] scale.sxy 256,0
			02:03:12 [lcuk] scale.swh 288,480
			
		// i need to recognise portrait and return mouse coordinate conversion for them
		



		
		// 20090621_193038 lcuk : no need for all this anymore :)
		
		liqapp_log("scale.win %i,%i",self->windoww,self->windowh);
		liqapp_log("scale.tar %i,%i",self->targetw,self->targeth);
		liqapp_log("scale.sxy %i,%i",self->scalex,self->scaley);
		liqapp_log("scale.swh %i,%i",self->scalew,self->scaleh);


		int mx,my;
		int dx,dy;
		
		mx=0;
		my=0;
		vgraph_convert_target2window(self,mx,my,&dx,&dy);
		liqapp_log("scale.test t2w %i,%i = %i,%i",mx,my,dx,dy);
		
		mx=self->targetw;
		my=self->targeth;
		vgraph_convert_target2window(self,mx,my,&dx,&dy);
		liqapp_log("scale.test t2w %i,%i = %i,%i",mx,my,dx,dy);
		


		mx=0;
		my=0;
		vgraph_convert_window2target(self,mx,my,&dx,&dy);
		liqapp_log("scale.test w2t %i,%i = %i,%i",mx,my,dx,dy);
		
		mx=self->windoww;
		my=self->windowh;
		vgraph_convert_window2target(self,mx,my,&dx,&dy);
		liqapp_log("scale.test w2t %i,%i = %i,%i",mx,my,dx,dy);
	*/

	}
	else
	{
		
		self->scalex=0;
		self->scaley=0;
		self->scalew=0;
		self->scaleh=0;
		
	}
}



int vgraph_setscaleaspectlock(vgraph *self,int newscaleaspectlock)
{
	self->scaleaspectlock = newscaleaspectlock;
	vgraph_recalc(self);
	return 0;	
}
int vgraph_getscaleaspectlock(vgraph *self)
{
	return self->scaleaspectlock;
}

	
int		vgraph_settarget(vgraph *self, liqimage *target )
{
	//if(self->target)
	//{
	//	liqimage_release(self->target);
	//	self->target=NULL;
	//}
	//if(target) self->target=liqimage_hold(target);
	
	if(target)
	{
		self->targetx=0;
		self->targety=0;
		self->targetw=target->width;
		self->targeth=target->height;
	}
	else
	{
		self->targetx=0;
		self->targety=0;
		self->targetw=0;
		self->targeth=0;
		
	}
	
	vgraph_recalc(self);
	return 0;
}


int		vgraph_settarget_coord(     		vgraph *self, int x,int y,    int w,int h )
{
	self->targetx=x;
	self->targety=y;
	self->targetw=w;
	self->targeth=h;
	vgraph_recalc(self);
	return 0;
}



int		vgraph_setwindow(   vgraph *self, liqcell *window )// int xs,int ys,    int xe,int ye )
{
	if(window)
	{
		//self->window = liqcell_hold(window);
		self->windowx=window->x;
		self->windowy=window->y;
		self->windoww=window->w;
		self->windowh=window->h;
	}
	else
	{
		//if(self->window) liqcell_release(self->window);
		//self->window = NULL;
		self->windowx=0;
		self->windowy=0;
		self->windoww=0;
		self->windowh=0;		
	}
	vgraph_recalc(self);
	return 0;
}


int		vgraph_setwindow_coord(     		vgraph *self, int x,int y,    int w,int h )
{
	//if(self->window) liqcell_release(self->window);
	//self->window = NULL;
		
	self->windowx=x;
	self->windowy=y;
	self->windoww=w;
	self->windowh=h;
	vgraph_recalc(self);
	return 0;
}










int		vgraph_setbackcolor(   vgraph *self, vcolor backcolor )
{
	self->backcolor=backcolor;
	return 0;
}

int		vgraph_setpencolor(    vgraph *self, vcolor pencolor )
{
	self->pencolor=pencolor;
	return 0;
}

int		vgraph_setpenthick(    vgraph *self, int penthick )
{
	self->penthick=penthick;
	return 0;
}



int		vgraph_setfont(        vgraph *self, liqfont *font)			//  char *fontname, int fontsize, int fontattributes
{
	if(self->font)
	{
		liqfont_release(self->font);
		self->font=NULL;
	}
	
	if(font)
	{
		self->font = liqfont_hold(font);
		vgraph_font_setscale(self);
	}
	return 0;
}



int		vgraph_drawclear(      vgraph *self                                  )
{
	unsigned char *yuva = (unsigned char *)&self->backcolor;
	liqcliprect_drawclear( vgraph_getcliprect(self), yuva[0],yuva[1],yuva[2] );
	return 0;
}


int		vgraph_drawpoint(      vgraph *self, int x, int y                    )
{
	
	//liqapp_log("draw.point.in  %i,%i",x,y);
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	//liqapp_log("draw.point.use %i,%i",x,y);
	unsigned char *yuva = (unsigned char *)&self->pencolor;
	liqcliprect_drawpsetcolor( vgraph_getcliprect(self),   x,y,       yuva[0],yuva[1],yuva[2] );
	return 0;
}


inline int		vgraph_drawline(       vgraph *self, int x, int y, int ex,int ey       )
{
	//liqapp_log("draw.line.in  %i,%i,%i,%i",x,y,ex,ey);
	x =  self->scalex + (x  * self->scalew / self->windoww);
	y =  self->scaley + (y  * self->scaleh / self->windowh);
	ex = self->scalex + (ex * self->scalew / self->windoww);
	ey = self->scaley + (ey * self->scaleh / self->windowh);

	//liqapp_log("draw.line.use %i,%i,%i,%i",x,y,ex,ey);

	unsigned char *yuva = (unsigned char *)&self->pencolor;
	liqcliprect_drawlinecolor( vgraph_getcliprect(self),   x,y,   ex,ey,    yuva[0],yuva[1],yuva[2] );
	return 0;
}


int		vgraph_drawbox(        vgraph *self, int x, int y, int w,int h       )
{
	
	//liqapp_log("draw.box.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	//liqapp_log("draw.box.use %i,%i,%i,%i",x,y,w,h);

	unsigned char *yuva = (unsigned char *)&self->pencolor;
	liqcliprect_drawboxlinecolor( vgraph_getcliprect(self),   x,y,   w,h,    yuva[0],yuva[1],yuva[2] );

	return 0;
}



int		vgraph_drawcolorcube(   		vgraph *self, int x, int y, int w,int h    )
{
	//liqapp_log("draw.colorcube.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	//liqapp_log("draw.colorcube.use %i,%i,%i,%i",x,y,w,h);

	unsigned char *yuva = (unsigned char *)&self->pencolor;
	liqcliprect_drawcolorcube( vgraph_getcliprect(self),   x,y,   w,h,    yuva[0] );

	return 0;
    
}
int		vgraph_drawgreycol(   	    	vgraph *self, int x, int y, int w,int h    )
{
	//liqapp_log("draw.greycol.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	//liqapp_log("draw.greycol.use %i,%i,%i,%i",x,y,w,h);
	liqcliprect_drawgreycol( vgraph_getcliprect(self),   x,y,   w,h);

	return 0;    
}








int		vgraph_drawrect(       vgraph *self, int x, int y, int w,int h       )
{
	
	//liqapp_log("draw.rect.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	
	
	//liqapp_log("draw.rect.use %i,%i,%i,%i",x,y,w,h);
	
	unsigned char *yuva = (unsigned char *)&self->backcolor;
	liqcliprect_drawboxfillcolor( vgraph_getcliprect(self),   x,y,   w,h,    yuva[0],yuva[1],yuva[2] );
	return 0;
}



int		vgraph_drawrectwash(       vgraph *self, int x, int y, int w,int h       )
{
	
	//liqapp_log("draw.rect.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	
	
	//liqapp_log("draw.rect.use %i,%i,%i,%i",x,y,w,h);
	
	unsigned char *yuva = (unsigned char *)&self->backcolor;
	liqcliprect_drawboxwashcolor( vgraph_getcliprect(self),   x,y,   w,h,    yuva[1],yuva[2] );
	return 0;
}
int		vgraph_drawrectfadeoutcolor(       vgraph *self, int x, int y, int w,int h       )
{
	
	//liqapp_log("draw.rect.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	
	
	//liqapp_log("draw.rect.use %i,%i,%i,%i",x,y,w,h);
	
	unsigned char *yuva = (unsigned char *)&self->backcolor;
	//liqcliprect_drawboxwashcolor( vgraph_getcliprect(self),   x,y,   w,h,    yuva[1],yuva[2] );
	liqcliprect_drawboxfadeoutcolor(vgraph_getcliprect(self) ,x,y,   w,h,   yuva[0],yuva[1], yuva[2],yuva[3] );
	return 0;
}




int		vgraph_drawcircle(     vgraph *self, int x, int y, int radius        )
{




	return vgraph_drawellipse(self,x,y,radius,radius);
}

















int		vgraph_drawellipse(    vgraph *self, int x, int y, int rx,int ry     )
{
	//liqapp_log("draw.ellipse.in  %i,%i,%i,%i",x,y,rx,ry);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	rx = (rx * self->scalew / self->windoww);
	ry = (ry * self->scaleh / self->windowh);
	
	
	
	//liqapp_log("draw.ellipse.use %i,%i,%i,%i",x,y,rx,ry);
	
	unsigned char *yuva = (unsigned char *)&self->pencolor;
	
int idx;
for(idx=0;idx<5;idx++)
{
	liqcliprect_drawboxlinecolor( vgraph_getcliprect(self),   x-rx,y-ry,   rx*2,ry*2,    yuva[0],yuva[1],yuva[2] );
	rx/=2;
	ry/=2;
}
	return 0;}


int		vgraph_drawtext(       vgraph *self, int x, int y, char *text        )
{
	if(!self->font)return -1;
	if(!text)return -1;
	if(!*text) return 0;

	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	//w = (w * self->scalew / self->windoww);
	//h = (h * self->scaleh / self->windowh);
	
	unsigned char *yuva = (unsigned char *)&self->pencolor;
	
	liqcliprect_drawtext_color( vgraph_getcliprect(self),self->font,   x,y, text,  yuva[0],yuva[1],yuva[2]);
	
	return 0;
}

























int		vgraph_drawsketch_old(       vgraph *self, int x, int y, int w,int h  ,liqsketch *sketch     )
{
	if(!sketch) return -1;
/*	
	liqapp_log("draw.sketch.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	
	
	liqapp_log("draw.sketch.use %i,%i,%i,%i",x,y,w,h);
 */	
	
	
	unsigned char *yuva = (unsigned char *)&self->backcolor;
	liqcliprect_drawboxfillcolor( vgraph_getcliprect(self),   x,y,   w,h,    yuva[0],yuva[1],yuva[2] );
	//vsketch_rendertograph(sketch,self,x,y,w,h);
	return 0;
}




int		vgraph_drawimage(      vgraph *self, int x, int y, int w,int h , liqimage *image      )
{

	if(!image) return -1;
	
	//liqapp_log("draw.sketch.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	
	liqcliprect_drawimagecolor(vgraph_getcliprect(self),  image, x,y,   w,h, 1 );
	//liqcliprect_drawboxfillcolor( vgraph_getcliprect(self),   x,y,   w,h,   128,128,128 );
	return 0;
}







int		vgraph_drawsketch(       vgraph *self, int x, int y, int w,int h  ,liqsketch *sketch     )
{
	if(!sketch) return -1;
	
	//liqapp_log("draw.sketch.in  %i,%i,%i,%i",x,y,w,h);
	
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	w = (w * self->scalew / self->windoww);
	h = (h * self->scaleh / self->windowh);
	
	
	
	//liqapp_log("draw.sketch.use %i,%i,%i,%i",x,y,w,h);
	liqcliprect_drawsketch( vgraph_getcliprect(self), sketch,  x,y,   w,h,   0 );
	
	//liqcliprect_drawboxfillcolor( vgraph_getcliprect(self),   x,y,   w,h,   128,128,128 );
	return 0;
}


//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################




int		vgraph_drawcell(      vgraph *self, int x, int y, int w,int h , liqcell *cell      )
{
	//liqapp_log("draw.cell.in  %i,%i,%i,%i   %s",x,y,w,h,cell->name);
	

	

	//ROTATEPATCHPOINT set is portrait as required
	int isportrait = (cell->h > cell->w) && ((x==0 && y==0));
		
	if( isportrait  )// && (aax < -200 ) )
	{		
		// autorotate...
		// testing potential autorotation patch
		// this will only work on root level display elements
		// its very hackish and example based
		
		//liqapp_log("vgraph_drawcell autorot strt xy(%d,%d),wh(%d,%d) %s",x,y,w,h,cell->name);
		static liqimage *img = NULL;
		if(img)
		{
			if(img->width == liqcanvas_getheight() && img->height == liqcanvas_getwidth())
			{
				// ok to keep cached
			}
			else
			{
				// renew
				liqimage_release(img);
				img=NULL;
			}
		}
		if(img==NULL)
		{
			//liqapp_log("vgraph_drawcell autorot alloc");
			img = liqimage_newatsize(liqcanvas_getheight(), liqcanvas_getwidth(),0);
		}
		liqcliprect *cr = liqcliprect_newfromimage(img);
		//liqcliprect_drawtext(cr,doc->displayfont,0,y,dline->linedata);
		//xsurface_drawrectwash_uv(img,0,0,480,800,128,128);
		
		//liqapp_log("vgraph_drawcell autorot run  xy(%d,%d),wh(%d,%d) %s",x,y,w,h,cell->name);
		//y = self->scalex + (y * self->scalew / self->windoww);
		//x = self->scaley + (x * self->scaleh / self->windowh);
		//h = (h * self->scalew / self->windoww);
		//w = (w * self->scaleh / self->windowh);
		
		liqcliprect_drawclear(cr,0,128,128);
		if(h>w)
			liqcell_easypaint(cell,cr,x,y,w,h);
		else
			liqcell_easypaint(cell,cr,x,y,h,w);
		liqimage_rotate( liqcanvas_getsurface(), img, 90 );
		liqcliprect_release(cr);
		
		//liqapp_log("vgraph_drawcell autorot done xy(%d,%d),wh(%d,%d) %s",x,y,w,h,cell->name);
	}
	else
	{

		x = self->scalex + (x * self->scalew / self->windoww);
		y = self->scaley + (y * self->scaleh / self->windowh);
		w = (w * self->scalew / self->windoww);
		h = (h * self->scaleh / self->windowh);
			
		liqcell_easypaint(cell,vgraph_getcliprect(self),x,y,w,h);
	}
	
	
	

	//liqapp_log("draw.cell.out  %i,%i,%i,%i   %s",x,y,w,h,cell->name);
		
	return 0;
}



//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################


int		vgraph_pget(      vgraph *self, int x, int y,unsigned char *grey,unsigned char *u,unsigned char *v)
{
	
	//liqapp_log("draw.pget.in  %i,%i",x,y);
	x = self->scalex + (x * self->scalew / self->windoww);
	y = self->scaley + (y * self->scaleh / self->windowh);
	//liqapp_log("draw.pget.use %i,%i",x,y);

	liqcliprect_drawpgetcolor( vgraph_getcliprect(self),   x,y,       grey,u,v );
	return 0;
}

#ifdef __cplusplus
}
#endif

