
#include <stdlib.h>
#include <string.h>


#include "liqcamera.h"
#include "liqapp.h"
#include "liqimage.h"
#include "liqcanvas.h"
#include "liqcliprect.h"

#include "liq_xsurface.h"

#ifdef __cplusplus
extern "C" {
#endif

static void liqimage_mark(liqimage *self);




static inline int isnear(int test,int centre,int range)
{
	if( test <= (centre-range) )
	{
		// under
		return 0;
	}
	if( test >= (centre+range) )
	{
		// over
		return 0;
	}
	// within
	return 1;
}
struct span
{
	int y;
	int xs;
	int xe;
	int w;
	int pys;
	int pye;
};
#define spancount 1000
static struct span spans[spancount];
int spanupto=0;

static void liqimage_spanmark(liqimage *self,int y,int xs,int xe,int pys,int pye)
{
	if((xe-xs)<4) return;
	
	if(spanupto>=spancount)return;
	spans[spanupto].y=y;
	spans[spanupto].xs=xs;
	spans[spanupto].xe=xe;
	spans[spanupto].w=xe-xs;
	spans[spanupto].pys=pys;
	spans[spanupto].pye=pye;
	spanupto++;
	
	//while(xs<xe)
	//{
	//	liqimage_drawpsetcolor(self,xs++,y,255-pys,128,128);//pu,pv);
	//}
}

static void liqimage_spandraw(liqimage *self,int spanidx)
{
	int y=spans[spanidx].y;
	int xs=spans[spanidx].xs;
	int xe=spans[spanidx].xe;
	int pys=spans[spanidx].pys;
	while(xs<xe)
	{
		//liqimage_drawpsetcolor(self,xs++,y,255-pys,128,128);//pu,pv);
        xsurface_drawpset_yuv(self,xs++,y,255-pys,128,128);//pu,pv);
	}
}













static void spanstretch(char *src,int srclen,char *dest,int destlen)
{
	// stretch srclen characters of src all the way along dest so they occupt destlen space
	int x;
	int IntPart = srclen / destlen;
	int FractPart = srclen % destlen;
	int E = 0;
	for(x=0;x<destlen;x++)
	{
		*dest++ = *src;
		src+=IntPart;
		E+=FractPart;
		if(E>=destlen)
		{
			E-=destlen;
			src++;
		}
	}
}



void liqimage_mark_barcode(liqimage *self)
{
    
    liqapp_log("barcode starting");

	spanupto=0;
	// put markers on barcode entries
	// ignore anytihng that is not a typical barcode
	// operate quickly and do not interupt user
	int x;
	int y;
	int foundcount=0;
	//int p;
	unsigned char ply;		// left,centre,right pixels
	unsigned char plu;
	unsigned char plv;
	unsigned char pcy;
	unsigned char pcu;
	unsigned char pcv;
	unsigned char pry;
	unsigned char pru;
	unsigned char prv;
	int spanw=0;
	int spanxs=0;
	int spanxe=0;
	int spanpys=0;
	int spanpye=0;
	
	for(y=0;y<self->height;y+=1)
	{
		// identify the range
		
		// restart spancounter
		//spanupto=0;
		
		//################################################################# stage 1
		xsurface_drawpget_yuv(self,0,y,&pcy,&pcu,&pcv);
		xsurface_drawpget_yuv(self,0,y,&pry,&pru,&prv);
		for(x=0;x<self->width;x++)
		{
			// could optimize the hell out of this by removing the pget
			// would be useful for more advanced algorithms
			
			// move the pixel chain along one step
			// pcentre -> pleft, pright -> pcentre, read pixel into pright
			ply=pcy;
			plu=pcu;
			plv=pcv;
			
			pcy=pry;
			pcu=pru;
			pcv=prv;
			
			// read the value of the rightmost pixel
			if(x<self->width)
				xsurface_drawpget_yuv(self,x+1,y,&pry,&pru,&prv);
			else
				xsurface_drawpget_yuv(self,x,y,&pry,&pru,&prv);
			
			// based soley upon a calculation of whether we are a bump or a dip
			// we show ourselves
			
			//xsurface_drawpset_yuv(self,xs++,y,255-pys,128,128);
			
			const int grain=3;
			
			if( (ply<pcy) && (pcy>pry) && (isnear(pcy,ply,grain)==0) && (isnear(pcy,pry,grain)==0))
			{
				// hump
				//xsurface_drawpset_yuv(self,x,y,255,128,128);// pcu,pcv);
				xsurface_drawpset_grey(self,x,y,255);
			}
			else
			if( (ply>pcy) && (pcy<pry) && (isnear(pcy,ply,grain)==0) && (isnear(pcy,pry,grain)==0))
			{
				// dip
				//xsurface_drawpset_yuv(self,x,y,0,128,128);// pcu,pcv);
				xsurface_drawpset_grey(self,x,y,0);
			}
			else
			{
				// flat/random!
				//xsurface_drawpset_yuv(self,x,y,128,128,128);
				xsurface_drawpset_grey(self,x,y,128);
			}
			
			   
		//	if( isnear(pu,128,12) && isnear(pv,128,12))
		}

	}
    
    liqapp_log("barcode complete");
}

#ifdef __cplusplus
}
#endif

