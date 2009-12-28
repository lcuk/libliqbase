

#include <stdlib.h>
#include <stdio.h>

#include <memory.h>

#include "liqcliprect.h"
#include "liq_xsurface.h"
#include "liqapp.h"
#include "liqcanvas.h"
#include "liqimage.h"
#include "liqsketch.h"
#include "liqaccel.h"




//inline void liqcliprect_drawglyph_color(liqcliprect *self,liqfont *font,int x,int y,unsigned char glyph,unsigned char grey,unsigned char u,unsigned char v)
//void        liqcliprect_drawlinecolor  (liqcliprect *self,int x1, int y1, int x2, int y2, unsigned char grey,unsigned char u,unsigned char v)

void		liqcliprect_drawthicklinecolor(		liqcliprect *self,int x1, int y1,int x2, int y2, unsigned char size, unsigned char grey,unsigned char u,unsigned char v)
{
	if(size<=1)
	{
		liqcliprect_drawlinecolor(self,x1,y1,x2,y2,grey,u,v);
		return;
	}
	if(size==2)
	{
		liqcliprect_drawlinecolor(self,x1  ,y1  ,x2,y2,grey,u,v);
		liqcliprect_drawlinecolor(self,x1  ,y1+1,x2,y2+1,grey,u,v);
		liqcliprect_drawlinecolor(self,x1+1,y1+1,x2+1,y2+1,grey,u,v);
		liqcliprect_drawlinecolor(self,x1+1,y1  ,x2+1,y2,grey,u,v);
		return;		
	}
	if(size==3)
	{
		liqcliprect_drawlinecolor(self,x1  ,y1+1,x2,y2+1,grey,u,v);
		liqcliprect_drawlinecolor(self,x1  ,y1  ,x2,y2,grey,u,v);
		liqcliprect_drawlinecolor(self,x1  ,y1-1,x2,y2-1,grey,u,v);
		liqcliprect_drawlinecolor(self,x1-1,y1  ,x2-1,y2,grey,u,v);
		liqcliprect_drawlinecolor(self,x1+1,y1  ,x2+1,y2,grey,u,v);
		return;		
	}

	if(size==4)
	{
		liqcliprect_drawthicklinecolor(self,x1-2,y1-2,x2-2,y2-2,2,grey,u,v);
		liqcliprect_drawthicklinecolor(self,x1-2,y1  ,x2-2,y2,2,grey,u,v);
		liqcliprect_drawthicklinecolor(self,x1  ,y1  ,x2,y2,2,grey,u,v);
		liqcliprect_drawthicklinecolor(self,x1  ,y1+2,x2,y2+2,2,grey,u,v);
		return;		
	}	
	
	//
	
}
