
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

const int grain=3;


// this is the "micro" variant
// i should make this a library selectable plugin soon



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


int getbalance(int l,int c,int r,int grain)
{
	// return integer indicating the balance of a triplet
	// given the 3 values, assess whether they are a straight line or a slope
	// the index returned indicates the motion
	
	// [ ] [ ] [ ]
	//  o   o   o
	//  o   o   o
	//  o   o   o


	//  o   o   o
	//  o   c   o 	// a hill - left is less than centre which is greater than r
	//  l   o   r	
	

	//  l   o   r
	//  o   c   o 	// a valley - left is greater than centre which is less than r
	//  o   o   o	
		
	
	#define layer(a,b,c) ( (a)*100+(b)*10+(c) )
	if(isnear(c,l,grain))
	{
		if(isnear(c,r,grain))
			return layer(1,1,1);
		else if(c<r)
			return layer(1,1,0);
		else
			return layer(1,1,2);
	}
	else if(l<c)
	{
		if(isnear(c,r,grain))
			return layer(0,1,1);
		else if(c<r)
			return layer(0,1,0);
		else
			return layer(0,1,2);
	}
	else
	{
		if(isnear(c,r,grain))
			return layer(2,1,1);
		else if(c<r)
			return layer(2,1,0);
		else
			return layer(2,1,2);
	}
}

int test_getbalance()
{
	// this is intended to check functionality of the getbalance routine
	// it also serves to demonstrate the expected api :)
	// idea: to scan source folders on building and try to encourage their formation
			const int grain=3;
			int bal = getbalance(40,50,60,grain);
			switch(bal)
			{
				case 012:	// hill
					break;
				case 210:	// valley
					break;
				case 111:	// flat
					break;
				case 010:	// slope climb "/"
					break;
				case 212:	// slope down "\"
					break;
			}
			return 0;
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
			
			int bal = getbalance(ply,pcy,pry,grain);
			
			//liqapp_log("bal (%d,%d,%d)=%d",ply,pcy,)
			xsurface_drawpset_grey(self,x,y,bal);
			
			switch(bal)
			{
				case 012:	// hill
					//xsurface_drawpset_grey(self,x,y,255);
					break;
				case 210:	// valley
					//xsurface_drawpset_grey(self,x,y,128);
					break;
				//case 111:	// flat
					//xsurface_drawpset_grey(self,x,y,128);
				//	break;

				//case 010:	// slope climb "/"
				//	break;
				//case 212:	// slope down "\"
				//	break;
				default:
					//xsurface_drawpset_grey(self,x,y,0);
					break;
			}
	/*		
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
			//	xsurface_drawpset_grey(self,x,y,128);
			}
	*/
			
			   
		//	if( isnear(pu,128,12) && isnear(pv,128,12))
		}

	}
    
    liqapp_log("barcode complete");
}

#ifdef __cplusplus
}
#endif

