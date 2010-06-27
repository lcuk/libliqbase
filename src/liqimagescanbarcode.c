
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
	unsigned char py;
	unsigned char pu;
	unsigned char pv;
	
	int spanw=0;
	int spanxs=0;
	int spanxe=0;
	int spanpys=0;
	int spanpye=0;
	
	for(y=0;y<self->height;y+=1)
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
            
			if( isnear(pu,128,12) && isnear(pv,128,12))
			{
				// only want greys :)				
				//if(spanw==0 || (isnear(py,spanpye,32)==0) )    // very "chatty"
				if(spanw==0 || (isnear(py,spanpys,32)==0) )		// alt attempt
				{
					// we are wildly different to an ongoing span, or one has not been started
					//liqimage_drawpsetcolor(self,x,y,0,128,128);
					if(spanw>0)
					{
						// mark the existing as completed
						liqimage_spanmark(self,y,spanxs,spanxe,spanpys,spanpye);
						spanw=0;
					}
					// start a new span
					spanxs=x;
					spanxe=x;
					spanpys=py;
					spanpye=py;
					spanw=1;
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
				//liqimage_drawpsetcolor(self,x,y,0,128,128);
				// colorful, if we have an ongoing span in progress we complete it here
					if(spanw>0)
					{
						// mark the existing as completed
						liqimage_spanmark(self,y,spanxs,spanxe,spanpys,spanpye);
						spanw=0;
					}
			}
		}
		// finished the row, if we have an ongoing span in progress we complete it here
					if(spanw>0)
					{
						// mark the existing as completed
						liqimage_spanmark(self,y,spanxs,spanxe,spanpys,spanpye);
						spanw=0;
					}
					
					
		//################################################################# stage 2	
		// i should now be examining the spans and attempt to locate barcode boundary marks
		// these are indicated by a pair of similar sized white elements
		// the data between them is considered to be the barcode
		// if there are NOT 2 close items then there is no barcode
		int n=0;
		int m=0;
		int isfirstmatchthisx=0;
		for(n=0;n<spanupto;n++)
		{
            
            
            
            
			isfirstmatchthisx=1;
			for(m=n+1;m<spanupto;m++)
			{
				int wid=spans[m].xs - spans[n].xe;
				
				if( (spans[n].w>=4) && (spans[m].w>=4) && (wid>=80) && isnear( spans[n].w , spans[m].w , 3 ) ) // && isnear( spans[n].pys , spans[m].pys , 128 ) )
				{
					// close in brightness
					// close in size :) good, are they also grey?
					int iscolor=0;
					
					
					int r=spans[n].xe;
			
			
			
					while(r<=spans[m].xs)
					{
						//liqimage_drawpgetcolor(self,r,y,&py,&pu,&pv);
                        //xsurface_drawpget_yuv(self,r,y,&py,&pu,&pv);
						if(isnear(pu,128,32) && isnear(pv,128,32))
						{
							// grey
						}
						else
						{
							// color, NO bad...
							iscolor=1;
							break;
						}
						//liqimage_spandraw(self,r);
						r++;
						
					}
					//app_log("barcode possible: %i",spans[m].xs - spans[n].xe);
					
					
			
			
					int tobinary(unsigned char *indat,int bitcount)
					{
						int res=0;
						int idx;
						int min=0;
						int max=0;
						int mid=0;
						char *src;
						src=indat;
						for(idx=0;idx<bitcount;idx++)
						{
							if(idx==0)
							{
								min=*src;
								max=*src;
							}
							else
							{
								if(*src<min)min=*src;
								if(*src>max)max=*src;
							}
							src++;
						}
						mid=max-min/2;
						src=indat;
						for(idx=0;idx<bitcount;idx++)
						{
							if(idx>0)res=res+res;
							if(*src>=mid)
							{
								res+=1;
								//*src=255;
							}
							else
							{
								//*src=0;
							}
							src++;
						}
						return res;
					}
					void try(char *indat,int bitlength,char *kind)
					{
						tobinary(indat,bitlength);
						//app_log("%s = '%i'",kind,tobinary(indat,bitlength));
					}
			
			
					if(iscolor==0)
					{
						// we have greyscale all the way between :)
						// GO GO GO!!!
						// does this matter?
						// http://en.wikipedia.org/wiki/Universal_Product_Code
						// 3 + 6*7 + 5 + 6*7 + 3  =  95     minimum unaliased pixels
						r=n;
						//while(r<=m)
						//{
						//	liqimage_spandraw(self,r++);
						//}
						// i dont have to draw all of them, simply those which deserve it
						
						
						liqimage_spandraw(self,n);
						
						liqimage_spandraw(self,m);
						
						//static void spanstretch(char *src,int srclen,char *dest,int destlen)
						
						char *src = &self->data[self->offsets[0] + (y*self->pitches[0]) + spans[n].xe];
						char dest[86];
						
						spanstretch(src,wid,dest,85);
						dest[85]=0;
						
					
						// ################################ convert to binary within local group
						try (&dest[0 ], 3, "bl ");
					
					 	
						try (&dest[3 ], 7, "l  ");
						try (&dest[10], 7, "l  ");
						
						try (&dest[17], 7, "l  ");
						try (&dest[24], 7, "l  ");
						
						try (&dest[31], 7, "l  ");
						try (&dest[38], 7, "l  ");
						
						try (&dest[45], 5, "mid");

						try (&dest[50], 7, "r  ");
						try (&dest[57], 7, "r  ");
						
						try (&dest[64], 7, "r  ");
						try (&dest[71], 7, "r  ");
						
						try (&dest[78], 7, "r  ");
						try (&dest[85], 7, "r  ");
						
						try (&dest[92], 3, "br");
						
						
						// if this is the second item found with this same initial start point, just override the total
						
						if(isfirstmatchthisx)
						{
							foundcount++;
						}
						else
						{
							// leave index where it is and overwrite
						}
						isfirstmatchthisx=0;
						if(foundcount<self->height)
						{
							src = &self->data[self->offsets[0] + (foundcount*self->pitches[0])];
							
							r=0;
							for(r=0;r<86;r++)
							{
							//	*src++ = dest[r];
							//	*src++ = dest[r];
							}
						}
						
						//
						
							
					}
				}
			}
		}
	}
    
    liqapp_log("barcode complete");
}

#ifdef __cplusplus
}
#endif

