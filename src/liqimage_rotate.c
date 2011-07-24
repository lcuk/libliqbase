
#include <stdlib.h>
#include <string.h>


#include "liqcamera.h"
#include "liqapp.h"
#include "liqimage.h"
#include "liqcanvas.h"
#include "liqcliprect.h"

#include "liq_xsurface.h"




// simple function to rotate an liqimage into a different orientation (90degrees only for now)
// this is specifically high level image only and is only for arbritary positions


// i have retained most of the different versions of this function
// to see its evolution

// i returned to the chunky 2 planar version though :)



int liqimage_rotate(liqimage *self,liqimage *original,int angle)
{
	//if(angle==0) return 0;
	
	
	// for now, ignore the angle..
    // liqapp_log("liqimage_rotate starting");
	// perform a sanity check against the image boundaries
	
	if(self->width != original->height || self->height != original->width)
	{
		liqapp_log("liqimage_rotate invalid dimensions self wh %d,%d    orig wh %d,%d",self->width,self->height,    original->width,original->height);
		return -1;
	}
	register int x=0;
	register int y=0;
	
	
	
	
	

// v1   23fps
#ifdef V1
	// read the bytes and write them individually
	// actually this isnt v1 but i havent kept the really slow ones..

	register unsigned char *sdata;
	sdata = &self->data[ self->offsets[0] + (self->width*y) ];
	for(y=0;y<self->height;y++)
	//for(y=self->height-1;y>=0;y--)
	{
		//register unsigned char *sdata;
		//sdata = &self->data[ self->offsets[0] + (self->width*y) ];
		for(x=0;x<self->width;x++)
		//for(x=self->width-1;x>=0;x--)
		{
			*sdata++ = original->data[ original->offsets[0] + (original->width*x + (original->width-1-y)) ];;
		}
	}
#endif


// v2  27fps
#ifdef V2

	// more advanced, bulk 4 reads into a single longint write

	int origoffset = original->offsets[0];
	int origwidth = original->width;
	int origwidthsub1 = origwidth-1;
	int selfwidth = self->width;


	register unsigned long *sdata;
	register unsigned long sdataitem=0;
	register unsigned long sdatacnt=0;
	sdata = (unsigned long *)&self->data[ self->offsets[0] + (self->width*y) ];
	for(y=0;y<self->height;y++)
	//for(y=self->height-1;y>=0;y--)
	{
		//register unsigned char *sdata;
		//sdata = &self->data[ self->offsets[0] + (self->width*y) ];
		for(x=0;x<selfwidth;x++)
		//for(x=self->width-1;x>=0;x--)
		{
			
			if(sdatacnt++<4)
			{
				sdataitem = (sdataitem >> 8) | original->data[ origoffset + (origwidth*x + (origwidthsub1-y)) ] << 24;
			}
			else
			{
				// optimise - if we have data, then write it, otherwise leave it..
				// (saves a memory write..)
				if(sdataitem)
					*sdata++ = sdataitem;
				else
					sdata++;
					
				sdataitem=  original->data[ origoffset + (origwidth*x + (origwidthsub1-y)) ] << 24;
				sdatacnt=1;
			}
		}
	}
#endif



	
// v3 27fps and even less when other hacks in place - not moving forwards yet..
#ifdef V3

	// speaking to spyro, he mentioned loop unrolling
	// mentioned count down to zero also (no comparison against value == faster due to z flag)
	// attempting hacks
	// Y
	{
		int origoffset = original->offsets[0];
		int origwidth = original->width;
		int origwidthm4 = original->width*4;
		int selfwidth = self->width;
		int selfheight=self->height;
		register unsigned long *sdata = (unsigned long *)&self->data[ self->offsets[0]  ];
		for(y=0;y<selfheight;y++)
		{
			register int origwidthsub1suby = origwidth-1-y;
		//	register unsigned char *o0 = original->data + origoffset + (origwidth*(  0  ) + (origwidthsub1suby));
		//	register unsigned char *o1 = original->data + origoffset + (origwidth*(  1  ) + (origwidthsub1suby));
		//	register unsigned char *o2 = original->data + origoffset + (origwidth*(  2  ) + (origwidthsub1suby));
		//	register unsigned char *o3 = original->data + origoffset + (origwidth*(  3  ) + (origwidthsub1suby));
			for(x=0;x<selfwidth;x+=4)
			{
				*sdata++  = original->data[ origoffset + (origwidth*(x  ) + (origwidthsub1suby)) ]       |
						    original->data[ origoffset + (origwidth*(x+1) + (origwidthsub1suby)) ] << 8  |
							original->data[ origoffset + (origwidth*(x+2) + (origwidthsub1suby)) ] << 16 |
							original->data[ origoffset + (origwidth*(x+3) + (origwidthsub1suby)) ] << 24 ;
			}
		}
	}
#endif


 
/*
 
 // small testing block to work out endianness and masks required below
#define maska 0xff000000
#define maskd 0x000000ff
unsigned char buf[4]={0};
	buf[0] = 1;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	unsigned long ib = *(unsigned long*)&buf[0];
	
	// shows ib == 1
	// shows buf 1,0,0,0
	liqapp_log("before:");
	liqapp_log("buf %d,%d,%d,%d",buf[0],buf[1],buf[2],buf[3]);
	liqapp_log("ib=%d",ib);
	
	ib = (ib & maskd) << 24;
	
	// shows ib == 16777216
	liqapp_log("during:");
	liqapp_log("ib=%d",ib);
	
unsigned long *tt = (unsigned long *)buf;
	*tt = ib;
	
	// shows ib == 16777216
	// shows buf 0,0,0,1

	liqapp_log("final:");
	liqapp_log("ib=%d",ib);
	liqapp_log("buf %d,%d,%d,%d",buf[0],buf[1],buf[2],buf[3]);
	
	exit(0);
	
 */


// v4 40fps!!!  down to ~37 when using full color all 3 planes..
#ifdef V4

	// trying in cubes of longs
	// this is based on amiga chunky 2 planar thinking (rotate the planes)
	// read 4 pixels each from 4 lines
	// bitshift and or into correct destinations
	// job done
	
	#define maska 0xff000000
	#define maskb 0x00ff0000
	#define maskc 0x0000ff00
	#define maskd 0x000000ff

	// YUV
	int uv=0;
	for(uv=0;uv<3;uv++)
	{
		
		int origoffset = original->offsets[uv];
		int origwidth = original->width;
		int selfwidth = self->width;
		int selfheight=self->height;
		if(uv>0)
		{
			origwidth>>=1;
			selfwidth>>=1;
			selfheight>>=1;
		}
		for(y=0;y<selfheight;y+=4)
		{
			// get points to the start of the next 4 destination lines
			register unsigned long *sdata0 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y)  ];
			register unsigned long *sdata1 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+1) ];
			register unsigned long *sdata2 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+2) ];
			register unsigned long *sdata3 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+3) ];

			register int origwidthsub1suby = origwidth-y-4;
			for(x=0;x<selfwidth;x+=4)
			{
				
				// read 16 pixels from 4 lines
				register unsigned long o0 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x    ) + (origwidthsub1suby)) ];
				register unsigned long o1 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+1  ) + (origwidthsub1suby)) ];
				register unsigned long o2 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+2  ) + (origwidthsub1suby)) ];
				register unsigned long o3 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+3  ) + (origwidthsub1suby)) ];
					
				*sdata3++ = (o0 & maskd)        |
							(o1 & maskd) << 8   |
							(o2 & maskd) << 16  |
							(o3 & maskd) << 24  ;

				*sdata2++ = (o0 & maskc) >> 8   |
							(o1 & maskc)        |
							(o2 & maskc) << 8   |
							(o3 & maskc) << 16  ;						 
						 
				*sdata1++ = (o0 & maskb) >> 16  |
							(o1 & maskb) >> 8   |
							(o2 & maskb)        |
							(o3 & maskb) << 8   ;						 

				*sdata0++ = (o0 & maska) >> 24  |
							(o1 & maska) >> 16  |
							(o2 & maska) >> 8   |
							(o3 & maska)        ;
			}
		}
	}


#endif


//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################

// # hacking session with ian molton
//

//#############################################################################


// v5
#ifdef V5

	// this is a wrapup after a discussion with ian
	
	// penalty for slipping
	//xsurface_drawclear_yuv( self, 0,128,128 );


	// trying in cubes of longs
	// this is based on amiga chunky 2 planar thinking (rotate the planes)
	// read 4 pixels each from 4 lines
	// bitshift and or into correct destinations
	// job done
	
	#define maska 0xff000000
	#define maskb 0x00ff0000
	#define maskc 0x0000ff00
	#define maskd 0x000000ff

	// YUV
	int uv=0;
	for(uv=0;uv<3;uv++)
	{
		
		int origoffset = original->offsets[uv];
		int origwidth = original->width;
		int selfwidth = self->width;
		int selfheight=self->height;
		if(uv>0)
		{
			origwidth>>=1;
			selfwidth>>=1;
			selfheight>>=1;
		}
		for(y=0;y<selfheight;y+=4)
		{
			// get points to the start of the next 4 destination lines

			//register unsigned long *sdata0 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y)  ];
			//register unsigned long *sdata1 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+1) ];
			//register unsigned long *sdata2 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+2) ];
			//register unsigned long *sdata3 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+3) ];

			// ian: optimisation via precalculating the data offset, could also be done with a pointer to similar effect
			unsigned int precalc = self->offsets[uv] + selfwidth * (y);
			register unsigned long *sdata0 = (unsigned long *)&self->data[ precalc  ];		precalc+=selfwidth;
			register unsigned long *sdata1 = (unsigned long *)&self->data[ precalc  ];		precalc+=selfwidth;
			register unsigned long *sdata2 = (unsigned long *)&self->data[ precalc  ];		precalc+=selfwidth;
			register unsigned long *sdata3 = (unsigned long *)&self->data[ precalc  ];		//precalc+=selfwidth;

			register int origwidthsub1suby = origwidth-y-4;
			for(x=0;x<selfwidth;x+=4)
			{
				
				// read 16 pixels from 4 lines
				//register unsigned long o0 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x    ) + (origwidthsub1suby)) ];
				//register unsigned long o1 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+1  ) + (origwidthsub1suby)) ];
				//register unsigned long o2 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+2  ) + (origwidthsub1suby)) ];
				//register unsigned long o3 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+3  ) + (origwidthsub1suby)) ];
				
				// ian: optimisation via precalculating the data offset, could also be done with a pointer to similar effect
				unsigned int precalc2 = origoffset + (origwidth*(  x    ) + (origwidthsub1suby));
				register unsigned long o0 = *(unsigned long *)&original->data[ precalc2 ]; precalc2+=origwidth;
				register unsigned long o1 = *(unsigned long *)&original->data[ precalc2 ]; precalc2+=origwidth;
				register unsigned long o2 = *(unsigned long *)&original->data[ precalc2 ]; precalc2+=origwidth;
				register unsigned long o3 = *(unsigned long *)&original->data[ precalc2 ]; //precalc2+=origwidth;
				
				// ian: you could go back and put the test back in.
				// ian: for all zero sources can skip a 4*4 block on destination
				// ian: potential on different kinds of images to save bigtime
				if(o0 || o1 || o2 || o3)
				{
						
					*sdata3++ = (o0 & maskd)        |
								(o1 & maskd) << 8   |
								(o2 & maskd) << 16  |
								(o3 & maskd) << 24  ;
	
					*sdata2++ = (o0 & maskc) >> 8   |
								(o1 & maskc)        |
								(o2 & maskc) << 8   |
								(o3 & maskc) << 16  ;						 
							 
					*sdata1++ = (o0 & maskb) >> 16  |
								(o1 & maskb) >> 8   |
								(o2 & maskb)        |
								(o3 & maskb) << 8   ;						 
	
					*sdata0++ = (o0 & maska) >> 24  |
								(o1 & maska) >> 16  |
								(o2 & maska) >> 8   |
								(o3 & maska)        ;
				}
				else
				//if(0)
				{
					// fill with grey to "see" how many.. this will be interesting
					//*sdata0++ = (255l << 24) | (0l << 16) | (255l << 8) | (0l);
					//*sdata1++ = (255l << 24) | (0l << 16) | (255l << 8) | (0l);
					//*sdata2++ = (255l << 24) | (0l << 16) | (255l << 8) | (0l);
					//*sdata3++ = (255l << 24) | (0l << 16) | (255l << 8) | (0l);
					// o_O on the text wall, this is most of the area.
					// this is quite good, changing over hte algorythm to simply skip without writing..
					// fault: it needs to be pre-cleared first
					// so prior to this operation, clear must be called
					// this is not good because it means some areas of this image will actually be written
					// twice..
					//sdata0++;
					//sdata1++;
					//sdata2++;
					//sdata3++;
					
					// trying alternative or still writing the data
					// but merely using simple 0 in the data
					// this means that the memclear is not required and also
					// all the shift and or operations are skipped
					// the memory must be written anyway.
					// so we should do it now once rather than clearing everything
					// then going over some of it with active data..
					*sdata0++ = 0;
					*sdata1++ = 0;
					*sdata2++ = 0;
					*sdata3++ = 0;
					// mental note, this is slower initially
					// i left the clear, hold on..
					
				}
			}
		}
	}
	
#endif



// v6 at ians morning!
#ifdef V6

	// trying in cubes of longs
	// this is based on amiga chunky 2 planar thinking (rotate the planes)
	// read 4 pixels each from 4 lines
	// bitshift and or into correct destinations
	
	// ians suggestions to use only one mask and adjust that going through (shifts are "free")
	// rather than storing all 4 constants
	// adjust inline and do 8bytes per loop instead of 4

	// YUV
	int uv=0;
	for(uv=0;uv<3;uv++)
	{
		
		int origoffset = original->offsets[uv];
		int origwidth = original->width;
		int selfwidth = self->width;
		int selfheight=self->height;
		if(uv>0)
		{
			origwidth>>=1;
			selfwidth>>=1;
			selfheight>>=1;
		}
		for(y=0;y<selfheight;y+=4)
		{
			// get points to the start of the next 4 destination lines
			register unsigned long *sdata0 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y)  ];
			register unsigned long *sdata1 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+1) ];
			register unsigned long *sdata2 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+2) ];
			register unsigned long *sdata3 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+3) ];

            register unsigned int mask = 0xff;
			register int origwidthsub1suby = origwidth-y-4;
			for(x=0;x<selfwidth;)
			{
				
				// read 16 pixels from 4 lines
				register unsigned long o0 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x++  ) + (origwidthsub1suby)) ];
				register unsigned long o1 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x++  ) + (origwidthsub1suby)) ];
				register unsigned long o2 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x++  ) + (origwidthsub1suby)) ];
				register unsigned long o3 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x++  ) + (origwidthsub1suby)) ];
					
				*sdata3++ = (o0 & mask)        |
							(o1 & mask) << 8   |
							(o2 & mask) << 16  |
							(o3 & mask) << 24  ;
                mask <<= 8;
				*sdata2++ = (o0 & mask) >> 8   |
							(o1 & mask)        |
							(o2 & mask) << 8   |
							(o3 & mask) << 16  ;						 
				mask <<= 8;		 
				*sdata1++ = (o0 & mask) >> 16  |
							(o1 & mask) >> 8   |
							(o2 & mask)        |
							(o3 & mask) << 8   ;						 
                mask <<= 8;
				*sdata0++ = (o0 & mask) >> 24  |
							(o1 & mask) >> 16  |
							(o2 & mask) >> 8   |
							(o3 & mask)        ;
				mask >>= 24;
				
				// read 16 pixels from 4 lines
				o0 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x++  ) + (origwidthsub1suby)) ];
				o1 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x++  ) + (origwidthsub1suby)) ];
				o2 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x++  ) + (origwidthsub1suby)) ];
				o3 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x++  ) + (origwidthsub1suby)) ];
					
				*sdata3++ = (o0 & mask)        |
							(o1 & mask) << 8   |
							(o2 & mask) << 16  |
							(o3 & mask) << 24  ;
                mask <<= 8;
				*sdata2++ = (o0 & mask) >> 8   |
							(o1 & mask)        |
							(o2 & mask) << 8   |
							(o3 & mask) << 16  ;						 
				mask <<= 8;		 
				*sdata1++ = (o0 & mask) >> 16  |
							(o1 & mask) >> 8   |
							(o2 & mask)        |
							(o3 & mask) << 8   ;						 
                mask <<= 8;
				*sdata0++ = (o0 & mask) >> 24  |
							(o1 & mask) >> 16  |
							(o2 & mask) >> 8   |
							(o3 & mask)        ;
				mask >>= 24;
			}
		}
	}

#endif













// v7 
//#ifdef V7

	// cleanup from the shorted cube mapped version
	// not noticing speedups like this cube in other variatons on n900
	// the added complexity removes the elegence of the algorithm
	// admires its awesomeness
	// gcc does a good job at this algorithm without making it unreadable
	
	// removed register prefix to see if it makes a difference - not really
	// changed the inner loop logic to hold/se a set of pointers
	
	// to be honest, adding complexity at expense of flexibity now is wrong
	// on the n900 at least this is enough as proof of concept.
	// the n8x0 is another story.  these must be tested and identified there
	// it uses an older version of gcc so likely will need the optimisations
	// looked at above.


	// trying in cubes of longs
	// this is based on amiga chunky 2 planar thinking
	// read 4 pixels each from 4 lines
	// bitshift and or into correct destinations
	// job done
	
	#define maska 0xff000000
	#define maskb 0x00ff0000
	#define maskc 0x0000ff00
	#define maskd 0x000000ff

	// YUV
	int uv=0;
	for(uv=0;uv<3;uv++)
	{
		
		int origoffset = original->offsets[uv];
		int origwidth = original->width;
		int selfwidth = self->width;
		int selfheight=self->height;
		if(uv>0)
		{
			origwidth>>=1;
			selfwidth>>=1;
			selfheight>>=1;
		}
		for(y=0;y<selfheight;y+=4)
		{
			// get points to the start of the next 4 destination lines
			register unsigned long *sdata0 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y)  ];
			register unsigned long *sdata1 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+1) ];
			register unsigned long *sdata2 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+2) ];
			register unsigned long *sdata3 = (unsigned long *)&self->data[ self->offsets[uv] + selfwidth * (y+3) ];

			register int origwidthsub1suby = origwidth-y-4;
			for(x=0;x<selfwidth;x+=4)
			{
				
				// read 16 pixels from 4 lines
				register unsigned long o0 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x    ) + (origwidthsub1suby)) ];
				register unsigned long o1 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+1  ) + (origwidthsub1suby)) ];
				register unsigned long o2 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+2  ) + (origwidthsub1suby)) ];
				register unsigned long o3 = *(unsigned long *)&original->data[ origoffset + (origwidth*(  x+3  ) + (origwidthsub1suby)) ];
					
				*sdata3++ = 		(o0 & maskd)        |
							(o1 & maskd) << 8   |
							(o2 & maskd) << 16  |
							(o3 & maskd) << 24  ;

				*sdata2++ = 		(o0 & maskc) >> 8   |
							(o1 & maskc)        |
							(o2 & maskc) << 8   |
							(o3 & maskc) << 16  ;						 
						 
				*sdata1++ = 		(o0 & maskb) >> 16  |
							(o1 & maskb) >> 8   |
							(o2 & maskb)        |
							(o3 & maskb) << 8   ;						 

				*sdata0++ = 		(o0 & maska) >> 24  |
							(o1 & maska) >> 16  |
							(o2 & maska) >> 8   |
							(o3 & maska)        ;
			}
		}
	}


//#endif




















	return 0;

	
	
}




