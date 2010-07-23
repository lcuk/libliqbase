


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liqcamera.h"
#include "liqapp.h"
#include "liqcell.h"
#include "liqcell_arrange.h"
#include "liqfont.h"
#include "liqimage.h"
#include "liqcanvas.h"
#include "liqcliprect.h"

#include "liq_xsurface.h"

#ifdef __cplusplus
extern "C" {
#endif



int hotspot_hitx = 0;
int hotspot_hity = 0;
int hotspot_hitsize = 0;
int hotspot_hitangledeg = 0;

int hotspot_hitshowmarking = 1;


#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
//#define ABS(x) (((x) >= 0) ? (x) : (-(x)))
//#define SGN(x) (((x) >= 0) ? 1 : -1)

//#####################################################################


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


//#####################################################################

static int getmid(unsigned const char *indat,int bitcount,unsigned char *outmin,unsigned char *outmax)
{
	// scan a range of bytes from indat, return the range and midpoint
	int idx;
	int min=0;
	int max=0;
	int mid=0;
	unsigned const char *src;
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
	//mid= min + (max-min) * 0.5;
	mid= min + (max-min) * 0.5;
	*outmin=min;
	*outmax=max;
	return mid;
}


//#####################################################################




static int liqimage_get_next_strip(liqimage *self, unsigned char *src,int x,unsigned char midgrey)
{
	// walk along the src data whilst it stays in one style

	int ssx=x;
	unsigned char sspy=0;

	int sex=x;
	unsigned char sepy=0;
	

	sspy = src[ssx];
	for(sex=ssx+1;sex<self->width;sex++)
	{
		sepy = src[sex];
		if((sspy>midgrey) && (sepy>midgrey)) 
		{
			// they are same style, continue
		}
		else
		{
			// not close - this span ends..
			break;
		}
	}

	//if(sex-ssx>8)	liqapp_log("%d x=%3d w=%3d py=%3d",ab,x,sex-ssx,sspy);
	return sex-ssx;
}


//#####################################################################


// hotspots are bright areas on the recorded image
// 

struct hotspot
{
	int t;
	int b;
	int l;
	int r;
	int w;
	int island;
};
#define hotspots_max 256
static struct hotspot hotspots[hotspots_max]={{0}};
static int hotspots_used=0;
static int hotspots_islands_used=0;

static int hotspot_island_merge(int hotspot_id)
{
	// find out which hotspots are to be combined together to form islands
	int other;
	for(other=hotspot_id-1;other>=0;other--)
	{
		int ydif = hotspots[hotspot_id].t - hotspots[other].t;
		if(ydif>1)
		{
			// there is a gap
			break;
		}
		if(ydif==1)
		{
			// adjacent lines
			if( ( hotspots[hotspot_id].r < hotspots[other].l ) ||
			    ( hotspots[hotspot_id].l > hotspots[other].r ) )
			{
				// missed the spot!
			}
			else
			{
				// touching!!
				// convert the island code to use lowest out of the pair
				// the higher code is replaced for all hotspots
				int lowisland  = MIN( hotspots[hotspot_id].island , hotspots[other].island );
				int highisland = MAX( hotspots[hotspot_id].island , hotspots[other].island );
				int stem;
				for(stem=0;stem<hotspots_used;stem++) if(hotspots[stem].island == highisland) hotspots[stem].island = lowisland;
			}
		}
	}
}

static int hotspot_add(int x,int y,int w)
{
	if(hotspots_used>=hotspots_max) return -1;
	hotspots[hotspots_used].t = y;
	hotspots[hotspots_used].b = y;
	hotspots[hotspots_used].l = x;
	hotspots[hotspots_used].r = x+w-1;
	hotspots[hotspots_used].w = w;
	hotspots[hotspots_used].island = ++hotspots_islands_used;
	hotspots_used++;
	hotspot_island_merge(hotspots_used-1);
}

//#####################################################################






//#####################################################################

int liqimagescan_hotspot_detect(liqimage *self)
{
    
   // liqapp_log("barcode starting");


	static liqfont *infofont=NULL;
	if(!infofont)
	{
		infofont = liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", 10, 0);
	}
	liqfont_setview(infofont, 1,1 );

		


	hotspots_used=0;
	
	
	
//	hotspot_hitx = 0;
//	hotspot_hity = 0;
//	hotspot_hitsize = 0;
//	hotspot_hitangledeg = 0;



	// put markers on barcode entries
	// ignore anytihng that is not a typical barcode
	// operate quickly and do not interupt user
	
	
	int foundcount=0;
	int usedcount=0;
	
	int y;
	for(y=0;y<self->height;y++)
	{
		//##################################### step 1:  identify the flats and tips of this line
		//                                               the sharp edges that our barcode is expected to have
		int x;
		
		
		//##################################### find variance within line
		unsigned char *src = &self->data[self->offsets[0] + (y*self->pitches[0]) ];
		unsigned char min;
		unsigned char max;
		unsigned char midgrey;
		if(y==0)
		{
			min=0;
			max=0;
			midgrey=0;
			midgrey=getmid(src,self->width * self->height,&min,&max);
			//midgrey=getmid(src,self->width,&min,&max);
		

			//midgrey=(min+max)/2;
			midgrey=min+(max-min)*0.4;
			//liqapp_log("grey %3d, %3d,%3d,%3d",y,min,max,midgrey);
		}
		//##################################### find variance within line
		
		
		for(x=0;x<self->width;)
		{
			// sc will always be 1..n - a single pixel is a strip also
			int sc = liqimage_get_next_strip(self, src,x,midgrey);
			if(src[x]>midgrey && sc>2)
			{
				// .. bright strip
				
				hotspot_add(x,y,sc);
				
				//while(sc--) { src[x++]=255; }
				x+=sc;
				foundcount++;
				
			}
			else
			{
				// .. dark strip
				//while(sc--) { src[x++]=0; }
				x+=sc;
			}
		}

	}
	
	//
	//void xsurface_drawrectwash_uv(   liqimage *surface,int x,int y,int w,int h, unsigned char u,unsigned char v);

//
struct blob
{
	int x;
	int y;
	int r;
};
#define blobs_max 32
struct blob blobs[blobs_max];
int blobs_used=0;


	int a,b;
	for(a=0;a<hotspots_used;a++)
	{
		if(hotspots[a].island>0)
		{
			int it = hotspots[a].t;
			int ib = hotspots[a].b;
			int il = hotspots[a].l;
			int ir = hotspots[a].r;
			for(b=a+1;b<hotspots_used;b++)
			{
				if( hotspots[a].island == hotspots[b].island )
				{
					if(hotspots[b].t < it)it=hotspots[b].t;
					if(hotspots[b].b > ib)ib=hotspots[b].b;
					if(hotspots[b].l < il)il=hotspots[b].l;
					if(hotspots[b].r > ir)ir=hotspots[b].r;
					hotspots[b].island=-hotspots[b].island;
				}
			}
			hotspots[a].island=-hotspots[a].island;
			
			int iw= (ir-il) ;
			int ih= (ib-it) ;
			
			if(blobs_used<blobs_max)
			{
				blobs[blobs_used].x=il+iw/2;
				blobs[blobs_used].y=it+ih/2;
				blobs[blobs_used].r=(int)sqrt( (float)((iw*iw)+(ih*ih))  );
				blobs_used++;
			}

			
			//if( (ir-il)>3 && isnear(ib-it,ir-il,5))
			{
				//xsurface_drawrectwash_uv(   self,il,it, ir-il+1, ib-it+1, (hotspots[a].island) % 4, (hotspots[a].island+3) % 8);
				//usedcount++;
			}
		}
	}
	
	if(blobs_used==4)
	{
		// this is difficult to consider and needs much work
		// i may have a simpler mechanism below
		
		// blobs are expected to be in a Y formation
		// A     B
		//
		//    C
		//
		//    D
		
		// create 2 virtual blobs, E and F
		// E == midpoint A..e..B
		// F == extension of C..D..f
		// if E ~= F then the blobs are correct
		// and we have recognised the orientation
		
		// alternatively - find the gravitational centrepoint
		// rotate the points to match and check alignment all of them
		
		// virtual blobs is simpler, but can potentially need to be run 16 times
		//int a=0;
		//int b=1;
		//int c=2;
		//int d=3;

		//int ex = ( blobs[a].x + blobs[b].x )/2;
		//int ey = ( blobs[a].y + blobs[b].y )/2;
		
		//int fx = ( blobs[c].x - (blobs[d].x-blobs[c].x) );
		//int fy = ( blobs[c].y - (blobs[d].y-blobs[c].y) );
	}


	if(blobs_used<5)	
	{
		// this seems simpler to do and logical to calculate
		// tho for many dots it will be slow (hence the limit on blob count!)
		
		
		// ok, 3 led detection
		//   N
		//
		// 
		// M   P
		// (N..M) == (N..P) and (M..P)*2 == (A..B)
		// thats it
		// just search for those ratios
		// no need for anything else..
		// distance between each
		int tt=0;
		int n,m,p;
		for(n=0;n<blobs_used;n++)
		{
			for(m=0;m<blobs_used;m++)
			if(m!=n)
			{
				// calculate length of vector N..M
				int nmx = ( blobs[m].x - blobs[n].x );
				int nmy = ( blobs[m].y - blobs[n].y );
				int nmd = (int)sqrt( (float) ((nmx*nmx)+(nmy*nmy)) );
				int nmr = nmd / 6;


				for(p=0;p<blobs_used;p++)
				if(p!=m)
				if(p!=n)
				{
					int npx = ( blobs[p].x - blobs[n].x );
					int npy = ( blobs[p].y - blobs[n].y );
					int npd = (int)sqrt( (float) ((npx*npx)+(npy*npy)) );
					
					if( isnear(nmd,npd,nmr) )
					{
						// ok, vector NM == vector NP!!!
						// calcul
						int mpx = ( blobs[m].x - blobs[p].x );
						int mpy = ( blobs[m].y - blobs[p].y );
						int mpd = (int)sqrt( (float) ((mpx*mpx)+(mpy*mpy)) );
						
						if( isnear(nmd,mpd*2,nmr) )
						{
							// WOOOOOOOOOOT
							
							if(hotspot_hitshowmarking)
							{

								xsurface_drawline_grey(self,  blobs[n].x,blobs[n].y, blobs[m].x,blobs[m].y, 128  );
								xsurface_drawline_grey(self,  blobs[n].x,blobs[n].y, blobs[p].x,blobs[p].y, 128  );
								xsurface_drawline_grey(self,  blobs[m].x,blobs[m].y, blobs[p].x,blobs[p].y, 128  );
								
								xsurface_drawrectwash_uv(   self,blobs[n].x-4,blobs[n].y-8, 16,16, (n) % 4, (n+3) % 8);
								xsurface_drawrectwash_uv(   self,blobs[m].x-4,blobs[m].y-6, 12,12, (n) % 4, (n+3) % 8);
								xsurface_drawrectwash_uv(   self,blobs[p].x-4,blobs[p].y-6, 12,12, (n) % 4, (n+3) % 8);
															
								
								int newhotspot_hitx = ( blobs[n].x + blobs[m].x + blobs[p].x ) / 3;
								int newhotspot_hity = ( blobs[n].y + blobs[m].y + blobs[p].y ) / 3;
								int newhotspot_hitsize = mpd;		// size is the size of the back line we have detected
								float dx = blobs[n].x - newhotspot_hitx;
								float dy = blobs[n].y - newhotspot_hity;
								int newhotspot_hitangledeg = atan2( dx,-dy  )*180.0/3.141592654;
								
								#define smoothfactor 0.4
								hotspot_hitx += ((float)(newhotspot_hitx - hotspot_hitx) * smoothfactor);
								hotspot_hity += ((float)(newhotspot_hity - hotspot_hity) * smoothfactor);
								hotspot_hitsize += ((float)(newhotspot_hitsize - hotspot_hitsize) * smoothfactor);
								hotspot_hitangledeg += ((float)(newhotspot_hitangledeg - hotspot_hitangledeg) * smoothfactor * 2);
								
								
							//	infosketch->angle = ((float)hotspot_hitangledeg) *  3.141592654 / 180.0;
								
								
								xsurface_drawline_grey(self,  hotspot_hitx,hotspot_hity, blobs[n].x,blobs[n].y, 255  );
	
		
	
								char buf[128];
								snprintf(buf,sizeof(buf),"xy(%3d,%3d) s(%3d) a(%3d)",hotspot_hitx,hotspot_hity,
																							  hotspot_hitsize,
																							  hotspot_hitangledeg
																							  );
								
								int hh=liqfont_textheight(infofont);
								int ww=liqfont_textwidth( infofont,buf);
								xsurface_drawtext_grey(self,infofont, hotspot_hitx-ww/2,hotspot_hity-hh/2,buf);
							}
								
							//liqapp_log("%s",buf);
							

							return 0;
							
							goto done;
							
							

						}
					}
				}
			
			}


		}
		
	}
	
done:
	
	if(0)
	{
		//
	}

	//	exit(0);
   // liqapp_log("barcode complete %d  :: used %d :: hs_used %d :: blobs used %d",foundcount,usedcount,hotspots_used,blobs_used);

	return -1;
}

#ifdef __cplusplus
}
#endif

