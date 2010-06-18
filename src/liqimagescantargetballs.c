
#include <stdlib.h>
#include <string.h>


#include "liqcamera.h"
#include "liqapp.h"
#include "liqimage.h"
#include "liqcanvas.h"
#include "liqcliprect.h"

#include "liq_xsurface.h"


#define ABS(x) (((x)<0) ? (-(x)) : (x) )


static inline int isnear(int test,int centre,int range)
{
    if(range==0) range=test/2;
    
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
    int spancolor;
};
#define spancount 1000
static struct span spans[spancount];
int spanupto=0;

static void liqimage_spanmark(liqimage *self,int y,int xs,int xe,int pys,int pye,int spancolor)
{
	if((xe-xs)<4) return;
	
	if(spanupto>=spancount)return;
	spans[spanupto].y=y;
	spans[spanupto].xs=xs;
	spans[spanupto].xe=xe;
	spans[spanupto].w=xe-xs;
	spans[spanupto].pys=pys;
	spans[spanupto].pye=pye;
    spans[spanupto].spancolor=spancolor;
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
        xsurface_drawpset_yuv(self,xs++,y,255-pys,12,12);//pu,pv);
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
	unsigned char py;
	unsigned char pu;
	unsigned char pv;
	
	int spanw=0;
	int spanxs=0;
	int spanxe=0;
	int spanpys=0;
	int spanpye=0;
    int spancolor=0;
	
	for(y=0;y<self->height*0.75;y+=1)
	{
		// identify the range
		
		// restart spancounter
		spanupto=0;
		
		//################################################################# stage 1
		// identify the boundary markers
		// white sections with GREYS only inside
		// ignore any color markers
		for(x=0;x<self->width;x++)
		{
			//liqimage_drawpgetcolor(self,x,y,&py,&pu,&pv);
            xsurface_drawpget_yuv(self,x,y,&py,&pu,&pv);
            
            int isbw=isnear(pu,128,24) && isnear(pv,128,24);
            
            py = (py<128) ? 0 : 255;
            
            
			//if( isnear(pu,128,12) && isnear(pv,128,12))
			//if( isnear(pu,128,24) && isnear(pv,128,24))
            if(1)
			{
                
                xsurface_drawpset_yuv(self,x,y,(py<128) ? 0 : 255,128,128);
                
				// only want greys :)				
				//if(spanw==0 || (isnear(py,spanpye,32)==0) )    // very "chatty"
				//if(spanw==0 || (isnear(py,spanpys,32)==0) )		// alt attempt
                //if(spanw==0 || (isnear(py,spanpye,32)==0) )    //
                
                if(spanw==0 || ( (spanpys>=128) ? (py<128):(py>=128) ) )    //
                
				{
					// we are wildly different to an ongoing span, or one has not been started
					//liqimage_drawpsetcolor(self,x,y,0,128,128);
					if(spanw>0)
					{
						// mark the existing as completed
						liqimage_spanmark(self,y,spanxs,spanxe,spanpys,spanpye,spancolor);
						spanw=0;
					}
					// start a new span
					spanxs=x;
					spanxe=x;
					spanpys=py;
					spanpye=py;
					spanw=1;
                    spancolor=0;
				}
				else
				{
					// we are similar to our neighbour, extend existing span
					
					
					spanxe=x;
					spanpye=py;
					spanw++;
					
				}
				
			}
			else
            
			{
                spancolor=1;
				//xsurface_drawpset_yuv(self,x,y,0,255,128);
				// colorful, if we have an ongoing span in progress we complete it here
					if(spanw>0)
					{
						// mark the existing as completed
						liqimage_spanmark(self,y,spanxs,spanxe,spanpys,spanpye,spancolor);
						spanw=0;
                        spancolor=0;
					}
			}
		}
		// finished the row, if we have an ongoing span in progress we complete it here
					if(spanw>0)
					{
						// mark the existing as completed
						liqimage_spanmark(self,y,spanxs,spanxe,spanpys,spanpye,spancolor);
						spanw=0;
                        spancolor=0;
					}
					
					
		//################################################################# stage 2	
		// i should now be examining the spans and attempt to locate barcode boundary marks
		// these are indicated by a pair of similar sized white elements
		// the data between them is considered to be the barcode
		// if there are NOT 2 close items then there is no barcode
		int n=0;
		int m=0;
		int isfirstmatchthisx=0;
		for(n=0;n<spanupto-5;n++)
		{
            if( spans[n+0].pys > 128  ) continue;
            if( spans[n+0].spancolor ) continue;
            if( spans[n+1].spancolor ) continue;
            if( spans[n+2].spancolor ) continue;
            if( spans[n+3].spancolor ) continue;
            if( spans[n+4].spancolor ) continue;
/*
            
            if( isnear( spans[n+0].w   , spans[n+1].w   , 0 ) &&
                isnear( spans[n+1].w*2 , spans[n+2].w   , 0 ) &&
                isnear( spans[n+2].w   , spans[n+3].w*2 , 0 ) &&
                isnear( spans[n+3].w   , spans[n+4].w   , 0 ) )
*/
            
            if( isnear( spans[n+0].w   , spans[n+4].w   , 0 ) &&
                isnear( spans[n+1].w   , spans[n+3].w   , 0 ) &&
                
                      ( spans[n+1].w   < spans[n+2].w      )  &&
                      ( spans[n+2].w   > spans[n+3].w      )
                 )

            {
                liqimage_spandraw(self,n+0);
                liqimage_spandraw(self,n+1);
                liqimage_spandraw(self,n+2);
                liqimage_spandraw(self,n+3);
                liqimage_spandraw(self,n+4);
            }
        }

	}
    
    liqapp_log("barcode complete");
}


