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
 * Routines which write data onto an liqimage surface
 *
 */


//20080726:gb:fixed up edge cases for glyph rendering  (offscreen left/right was just bailing rather than drawing a partial)




// do some alpha blending :)
// i see now how it can work
//http://www.phatcode.net/articles.php?id=233
//DIM alpha AS LONG
//alpha = 30   '30 percent
//blendpix.Red = (srcpixone.Red - srcpixtwo.Red) * alpha + srcpixtwo.Red) \ 100


// 20090709_001938 lcuk : added to test the thickline and antialiased line capability
// 20090709_001954 lcuk : must extend checking, i think its viable :)
// 20090709_002008 lcuk : todo formulate parameter modifications and entry points
// 20090709_002033 lcuk : decide whether to allow native current single line sources to use standard unalised sections
// 20090709_002053 lcuk : allow sketches themselves to choose aa or not, or rather give the user the option to choose for them

//#define USE_AA_THICKLINE 1
//#define USE_AA 1

// 20091230_0417 lcuk : thick lines work for felt tip coloring, todo, thread a standard thickline api round here and cliprect/graph and adapt this correctly


#include <memory.h>
#include <stdlib.h> 


// http://www.x.org/docs/Xv/video
// todo:lcuk: damn all this time i have been calculating planes and offsets and sizes i could have used properties from the liqimage itself.
// what a plonker i am, ahhh well too late to adjust right now, that will come later

#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define ABS(x) (((x) >= 0) ? (x) : (-(x)))
#define SGN(x) (((x) >= 0) ? 1 : -1)

#include "liqapp.h"
#include "liq_xsurface.h"			// include available workhorse functions
#include "liqfont.h"


#ifdef __cplusplus
extern "C" {
#endif

//########################################################################
//######################################################################## quick blitter
//########################################################################




static inline unsigned int doblendy(int s,int t, int blend)
{
	//int s=*Source;
	//int t=*Target;
	//if(!s)s=128;
	//if(!t)t=128;
	//int a=*Src_alphachanneldoubleres;
	//*Target++ = t+((s-t)*a)/256;
	int r=  t+((s-t)*blend)/256;

	//if(!r)r=1;
	return r;//t+((s-t)*blend*a)/65536;

}

static inline unsigned int doblenduv(int s,int t, int blend)
{
	//int s=*Source;
	//int t=*Target;
	if(!s)s=128;
	if(!t)t=128;
	//int a=*Src_alphachanneldoubleres;
	//*Target++ = t+((s-t)*a)/256;
	int r=  t+((s-t)*blend)/256;

	if(!r)r=1;
	return r;//t+((s-t)*blend*a)/65536;

}

void xsurface_drawstrip_colortest1(
	register unsigned int  linecount,
	register unsigned int  charsperline,
	register unsigned char *srcdataptr,
	register unsigned char *dstdataptr,
	register unsigned int  srclinejump,
	register unsigned int  dstlinejump,

	liqimage *destimage,
	unsigned char col_y,
	unsigned char col_u,
	unsigned char col_v,
	int dsx,int dsy
	)
{

	int y=dsy;






	if(charsperline<=0) return;
	register int charwidth;
	while(linecount--)
	{
					int uvy = y >> 1;

		int x=dsx;


		charwidth=(int)charsperline;

		while(charwidth)
		{



			unsigned char bright = *srcdataptr++;
			if(bright)
			{
				// bright is how far from old to new we must travel.
				// if bright==0 then we stay at old
				// if bright==255 then we end up with a value of col_y

				unsigned int grey;// = (    (      ((int)bright) * ((int)col_y)    ) >> 8 );


				grey = doblendy(col_y,*dstdataptr,bright);

				//grey=255;

				*dstdataptr++ = grey;

				// HUGE optimization hit here because we are generating 1 color pixel for every grey pixel, this should be 1x color per 4 grey..

				//if(((x&1)) && ((y&1)))
				if((y&1))
				{
					// (gulp! deal with the UV planes)

					int uvx = x >> 1;




					unsigned char *dstu = & destimage->data[ destimage->offsets[1] + uvy * destimage->pitches[1] + uvx ];
					unsigned int uu ;//= 128 + (    (      ((int)bright) * (((int)col_u)-128)    ) >> 8 );

					uu = doblenduv(col_u,*dstu,bright);

					*dstu = uu;





					unsigned char *dstv = & destimage->data[ destimage->offsets[2] + uvy * destimage->pitches[2] + uvx ];
					unsigned int vv; // = 128 + (    (      ((int)bright) * (((int)col_v)-128)    ) >> 8 );

					vv=doblenduv(col_v,*dstv,bright);
					*dstv = vv;


				}

			}
			else
			{
				dstdataptr++;
			}

			x++;

			charwidth--;
		}

		// line skipper

		y++;


		srcdataptr+=srclinejump;
		dstdataptr+=dstlinejump;
	}
}









void xsurface_drawstrip_or(
	register unsigned int  linecount,
	register unsigned int  charsperline,
	register unsigned char *srcdataptr,
	register unsigned char *dstdataptr,
	register unsigned int  srclinejump,
	register unsigned int  dstlinejump)
{

	if(charsperline<=0) return;
	register int charwidth;
	while(linecount--)
	{
		charwidth=(int)charsperline;
		// unwind the loop and handle the big byte blocks first :) thanks ssvb for prompting

	/*	//############################ handle the int64 blocks using standard c: try using the drawstrip
		if(charwidth>7)
		{
			// this fails as well, ill look at it another time again,
			// but now i have failed to write >32bits in one chunk via any recognised method
			// something is strange
			struct drawstrip_64
			{
				int a;
				int b;
			};
			struct drawstrip_64 *src = (struct drawstrip_64 *)srcdataptr;
			struct drawstrip_64 *dst = (struct drawstrip_64 *)dstdataptr;
			while((charwidth)>7)
			{
				// *dst++ = *src++;
				struct drawstrip_64 a;//=*src;
				*dst=a;
				dst++; src++;
				charwidth-=8;
			}
			srcdataptr = (unsigned char*)src;
			dstdataptr = (unsigned char*)dst;
		}
				 */
		//############################ handle the int64 blocks using standard c doubles
	/*	if(charwidth>7)
		{
			double *src = (double *)srcdataptr;		// why wont this work with doubles?
			double *dst = (double *)dstdataptr;
			while((charwidth)>7)
			{
				*dst++ = *src++;
				charwidth-=8;
			}
			srcdataptr = (unsigned char*)src;
			dstdataptr = (unsigned char*)dst;
		}
	*/
		//############################ handle the int64 blocks using standard c ull
	/*	if(charwidth>7)
		{
			unsigned long long *src = (unsigned long long *)srcdataptr;		// why wont this work with ull?
			unsigned long long *dst = (unsigned long long *)dstdataptr;
			while((charwidth)>7)
			{
				*dst++ = *src++;
				charwidth-=8;
			}
			srcdataptr = (unsigned char*)src;
			dstdataptr = (unsigned char*)dst;
		}
	*/


		//############################ handle the int64 blocks using ASM - fails. it locks like all other >32bit options attempted
	/*
// lets see, RST says asm is good..
//http://code.google.com/p/arm1136j-s/source/browse/trunk/graphics/inline-asm-mem-16bpp.c

#define load2int_wb(addr, int1, int2) \
        __asm__("ldmia %[address]!, {%[tmp1], %[tmp2]}":\
              [tmp1] "=r" (int1), [tmp2] "=r" (int2),\
              [address] "+r" (addr))

#define store2int_wb(addr, int1, int2) \
        __asm__("stmia %[address]!, {%[tmp1], %[tmp2]}":\
              [address] "+r" (addr): [tmp1] "r" (int1), [tmp2] "r" (int2)\
              : "memory")

	 	if(charwidth>7)
		{
			//unsigned long long *src = (unsigned long long *)srcdataptr;
			//unsigned long long *dst = (unsigned long long *)dstdataptr;
			while((charwidth)>7)
			{
				register unsigned tmp1 __asm__("r4");
				register unsigned tmp2 __asm__("r5");

				load2int_wb(srcdataptr, tmp1, tmp2);
				store2int_wb(dstdataptr, tmp1, tmp2);

				charwidth-=8;
			}
			//srcdataptr = (unsigned char*)src;
			//dstdataptr = (unsigned char*)dst;
		}
	*/

		//################################################################### memcpy version
		// 7.5 on the speedtest for all this below here with nothing above
		// 5.9 on the speedtest for the OR | variation.  this is acceptable forthe quality improvement for now..
		// convert the |= back to = for speedup, but graphically it looks bad

		//############################ handle the int32 blocks
		if(charwidth>3)
		{
			register unsigned int *src = (unsigned int*)srcdataptr;
			register unsigned int *dst = (unsigned int*)dstdataptr;
			while((charwidth)>3)
			{
				*dst++ |= *src++;
				charwidth-=4;
			}
			srcdataptr = (unsigned char*)src;
			dstdataptr = (unsigned char*)dst;
		}
		//############################ handle the trailing int16 block
		if(charwidth>1)
		{
			*(short *)dstdataptr |= *(short *)srcdataptr;
			dstdataptr+=2;
			srcdataptr+=2;
			charwidth-=2;
		}
		//############################ handle the final int8 block
		if(charwidth)
		{
			*dstdataptr++ |= *srcdataptr++;
		}

		//################################################################### memcpy version
		// 3.5 on the speedtest
		//memcpy(dstdataptr,srcdataptr,charwidth);
		//dstdataptr+=charwidth;
		//srcdataptr+=charwidth;

		srcdataptr+=srclinejump;
		dstdataptr+=dstlinejump;
	}
}





void xsurface_drawstrip(
	register unsigned int  linecount,
	register unsigned int  charsperline,
	register unsigned char *srcdataptr,
	register unsigned char *dstdataptr,
	register unsigned int  srclinejump,
	register unsigned int  dstlinejump)
{

	if(charsperline<=0) return;
	register int charwidth;
	while(linecount--)
	{
		charwidth=(int)charsperline;
		// unwind the loop and handle the big byte blocks first :) thanks ssvb for prompting

	/*	//############################ handle the int64 blocks using standard c: try using the drawstrip
		if(charwidth>7)
		{
			// this fails as well, ill look at it another time again,
			// but now i have failed to write >32bits in one chunk via any recognised method
			// something is strange
			struct drawstrip_64
			{
				int a;
				int b;
			};
			struct drawstrip_64 *src = (struct drawstrip_64 *)srcdataptr;
			struct drawstrip_64 *dst = (struct drawstrip_64 *)dstdataptr;
			while((charwidth)>7)
			{
				// *dst++ = *src++;
				struct drawstrip_64 a;//=*src;
				*dst=a;
				dst++; src++;
				charwidth-=8;
			}
			srcdataptr = (unsigned char*)src;
			dstdataptr = (unsigned char*)dst;
		}
				 */
		//############################ handle the int64 blocks using standard c doubles
	/*	if(charwidth>7)
		{
			double *src = (double *)srcdataptr;		// why wont this work with doubles?
			double *dst = (double *)dstdataptr;
			while((charwidth)>7)
			{
				*dst++ = *src++;
				charwidth-=8;
			}
			srcdataptr = (unsigned char*)src;
			dstdataptr = (unsigned char*)dst;
		}
	*/
		//############################ handle the int64 blocks using standard c ull
	/*	if(charwidth>7)
		{
			unsigned long long *src = (unsigned long long *)srcdataptr;		// why wont this work with ull?
			unsigned long long *dst = (unsigned long long *)dstdataptr;
			while((charwidth)>7)
			{
				*dst++ = *src++;
				charwidth-=8;
			}
			srcdataptr = (unsigned char*)src;
			dstdataptr = (unsigned char*)dst;
		}
	*/


		//############################ handle the int64 blocks using ASM - fails. it locks like all other >32bit options attempted
	/*
// lets see, RST says asm is good..
//http://code.google.com/p/arm1136j-s/source/browse/trunk/graphics/inline-asm-mem-16bpp.c

#define load2int_wb(addr, int1, int2) \
        __asm__("ldmia %[address]!, {%[tmp1], %[tmp2]}":\
              [tmp1] "=r" (int1), [tmp2] "=r" (int2),\
              [address] "+r" (addr))

#define store2int_wb(addr, int1, int2) \
        __asm__("stmia %[address]!, {%[tmp1], %[tmp2]}":\
              [address] "+r" (addr): [tmp1] "r" (int1), [tmp2] "r" (int2)\
              : "memory")

	 	if(charwidth>7)
		{
			//unsigned long long *src = (unsigned long long *)srcdataptr;
			//unsigned long long *dst = (unsigned long long *)dstdataptr;
			while((charwidth)>7)
			{
				register unsigned tmp1 __asm__("r4");
				register unsigned tmp2 __asm__("r5");

				load2int_wb(srcdataptr, tmp1, tmp2);
				store2int_wb(dstdataptr, tmp1, tmp2);

				charwidth-=8;
			}
			//srcdataptr = (unsigned char*)src;
			//dstdataptr = (unsigned char*)dst;
		}
	*/

		//################################################################### memcpy version
		// 7.5 on the speedtest for all this below here with nothing above

		//############################ handle the int32 blocks
		if(charwidth>3)
		{
			register unsigned int *src = (unsigned int*)srcdataptr;
			register unsigned int *dst = (unsigned int*)dstdataptr;
			while((charwidth)>3)
			{
				*dst++ = *src++;
				charwidth-=4;
			}
			srcdataptr = (unsigned char*)src;
			dstdataptr = (unsigned char*)dst;
		}
		//############################ handle the trailing int16 block
		if(charwidth>1)
		{
			*(short *)dstdataptr = *(short *)srcdataptr;
			dstdataptr+=2;
			srcdataptr+=2;
			charwidth-=2;
		}
		//############################ handle the final int8 block
		if(charwidth)
		{
			*dstdataptr++ = *srcdataptr++;
		}

		//################################################################### memcpy version
		// 3.5 on the speedtest
		//memcpy(dstdataptr,srcdataptr,charwidth);
		//dstdataptr+=charwidth;
		//srcdataptr+=charwidth;

		srcdataptr+=srclinejump;
		dstdataptr+=dstlinejump;
	}
}







void xdata_drawimage_grey(unsigned char *surfdata,int surfw,int surfh,   unsigned char *imgdata,int imgw,int imgh,int x,int y)
{
	// cleanse this function now, it requires TLC
	// font has all changed and there is no longer a single contigious glyph buffer
	// this is better memory wise, but its more fragmented
	// the offsets are simpler
	if(x+imgw<0)return;
	//if(font->glyphdata[glyph]==NULL) return;
	int gw  = imgw;
	int gh  = imgh;
	int gtw = gw;//font->glyphtilew;
	//int gth = gh;//font->glyphmaxh;

	int sw  = surfw;
	int sh  = surfh;

	if(x>sw)return;
	if(x+gw>sw)
	{
		gw=sw-x;
	}

	if(x<0)
	{
		if(x<-gw) return;
		imgdata = &imgdata[-x];
		gw+=x;
		x=0;
	}



	if(y+gh<0)return;
	if(y+gh>sh)
	{
		if(y>=sh) return;
		gh=(sh-y);
	}

	unsigned int goff = 0;//((font->glyphtilesize) * (int)(glyph));
	unsigned int gskip = gtw-gw;

	unsigned int poff = sw * y + x;
	unsigned int pskip = sw-gw;
//---------------------------------------
	unsigned char *pdata;
	unsigned char *gdata;
	if(y<0)
	{
		y=-y;
		goff+=gtw*y;
		poff+=sw*y;
		gh-=y;
		y=0;
	}


	gdata = & ((unsigned char*)imgdata)                [ goff ] ;
	pdata = & ((unsigned char*)surfdata)               [ poff ];
	//liqapp_log("strip.. gh=%i, gw=%i,  gd=%i,pd=%i gskip=%i,pskip=%i",gh,gw,(int)gdata,(int)pdata,gskip,pskip);
	xsurface_drawstrip(gh,gw,gdata,pdata,gskip,pskip);
}


void xsurface_drawimage_color(liqimage *surface,liqimage *image,int x,int y)
{
	int surfw=surface->width;
	int surfh=surface->height;
	int imgw =image->width;
	int imgh =image->height;
	xdata_drawimage_grey(&surface->data[surface->offsets[0]],surface->width,surface->height,  &image->data[image->offsets[0]],image->width,image->height,   x,y);

	x>>=1;
	y>>=1;
	surfw>>=1;
	surfh>>=1;
	imgw>>=1;
	imgh>>=1;

	xdata_drawimage_grey(&surface->data[surface->offsets[1]],surfw,surfh,  &image->data[image->offsets[1]],imgw,imgh,   x,y);
	xdata_drawimage_grey(&surface->data[surface->offsets[2]],surfw,surfh,  &image->data[image->offsets[2]],imgw,imgh,   x,y);
}



//#######################################################################################
//####################################################################################### ScaleLine variations Std
//#######################################################################################


void ScaleLine_grey_slow(unsigned char *Target, unsigned char *Source, int SrcWidth, int TgtWidth,int TgtDrawStartOffset, int TgtDrawPixelCount)
{
  int NumPixels = TgtDrawPixelCount;//TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;
  for(NumPixels=0;NumPixels<TgtDrawPixelCount;NumPixels++)
  {
    if(NumPixels>=TgtDrawStartOffset)
	{
		*Target++ = *Source;
	}
	else
	{
		Target++;
	}
    Source += IntPart;

    E += FractPart;
    if (E >= TgtWidth) {
      E -= TgtWidth;
      Source++;
    }
  }
}





//################################################################### attempt at faster..

void ScaleLine_grey(unsigned char *Target, unsigned char *Source, int SrcWidth, int TgtWidth,int TgtDrawStartOffset, int TgtDrawPixelCount)
{
	// todo: important: make this routine fast
	// it must end up using shorts and longs
  int NumPixels = TgtDrawPixelCount;//TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;
  //unsigned int tbuf=0;
  //int tbufused=0;
  for(NumPixels=0;NumPixels<TgtDrawPixelCount;NumPixels++)
  {
    if(NumPixels>=TgtDrawStartOffset)
	{
		*Target++ = *Source;
	/*	switch(tbufused)
		{
			case 0:
				tbuf=(unsigned int)*Source;
				tbufused++;
				break;
			case 1:
				tbuf=(tbuf<<8) | *Source;
				tbufused++;
				break;
			case 2:
				tbuf=(tbuf<<8) | *Source;
				tbufused++;
				break;
			case 3:

				tbuf=(tbuf<<8) | *Source;

				*(unsigned int*)Target = tbuf;
				Target+=4;
				tbufused=0;
				break;
		}*/

	}
	else
	{
		Target++;
	}
    Source += IntPart;

    E += FractPart;
    if (E >= TgtWidth)
	{
      E -= TgtWidth;
      Source++;
    }
  }
	/*	switch(tbufused)
		{
			case 0:
				// nothing to do
				break;
			case 1:
				*Target++ = tbuf;
				break;
			case 2:
				*(unsigned short*)Target = (unsigned short)tbuf;
				Target+=2;
				break;
			case 3:
				*Target++ = (tbuf>>16);

				*(unsigned short*)Target = (unsigned short)tbuf;
				Target+=2;
				break;

		}*/
}



void ScaleLine_uv(unsigned char *Target,unsigned  char *Source, int SrcWidth, int TgtWidth,int TgtDrawStartOffset, int TgtDrawPixelCount)
{
  int NumPixels = TgtDrawPixelCount;//TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;
  //unsigned int tbuf=0;
  //int tbufused=0;
  for(NumPixels=0;NumPixels<TgtDrawPixelCount;NumPixels++)
  {
    if(NumPixels>=TgtDrawStartOffset)
	{
		*Target++ = *Source;
		/*switch(tbufused)
		{
			case 0:
				tbuf=(unsigned int)*Source;
				tbufused++;
				break;
			case 1:
				tbuf=(tbuf<<8) | *Source;
				tbufused++;
				break;
			case 2:
				tbuf=(tbuf<<8) | *Source;
				tbufused++;
				break;
			case 3:

				tbuf=(tbuf<<8) | *Source;

				*(unsigned int*)Target = tbuf;
				Target+=4;
				tbufused=0;
				break;
		}*/
	}
	else
	{
		Target++;
	}
    Source += IntPart;

    E += FractPart;
    if (E >= TgtWidth)
	{
      E -= TgtWidth;
      Source++;
    }
  }
	/*	switch(tbufused)
		{
			case 0:
				// nothing to do
				break;
			case 1:
				*Target++ = tbuf;
				break;
			case 2:
				*(unsigned short*)Target = (unsigned short)tbuf;
				Target+=2;
				break;
			case 3:
				*Target++ = (tbuf>>16);

				*(unsigned short*)Target = (unsigned short)tbuf;
				Target+=2;
				break;

		}*/
}

//#######################################################################################
//####################################################################################### ScaleLine variations Alpha
//#######################################################################################


void ScaleLine_alphablend_grey(unsigned char *Target, unsigned char *Source, int SrcWidth, int TgtWidth,int TgtDrawStartOffset, int TgtDrawPixelCount,unsigned char *Src_alphachannelfullres,unsigned char blend)
{
  int NumPixels = TgtDrawPixelCount;//TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;
  //while (NumPixels-- > 0)
  for(NumPixels=0;NumPixels<TgtDrawPixelCount;NumPixels++)
  {
    if(NumPixels>=TgtDrawStartOffset)
	{
		// alpha blending from an actual alpha channel
		int s=*Source;
		int t=*Target;
		int a=*Src_alphachannelfullres;
		//*Target++ = t+((s-t)*a)/256;
		*Target++ = t+(((s-t)*blend*a)  >>16);// /65536;
	}
	else
	{
		Target++;
	}
    Source += IntPart;
	Src_alphachannelfullres += IntPart;

    E += FractPart;
    if (E >= TgtWidth)
	{
      E -= TgtWidth;
      Source++;
	  Src_alphachannelfullres++;
    }
  }
}




void ScaleLine_alphablend_uv(unsigned char *Target, unsigned char *Source, int SrcWidth, int TgtWidth,int TgtDrawStartOffset, int TgtDrawPixelCount,unsigned char *Src_alphachanneldoubleres,unsigned char blend)
{
  int NumPixels = TgtDrawPixelCount;//TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;

  for(NumPixels=0;NumPixels<TgtDrawPixelCount;NumPixels++)
  {
    if(NumPixels>=TgtDrawStartOffset)
	{
		// alpha blending from an actual alpha channel
		int s=*Source;
		int t=*Target;
		if(!s)s=128;
		if(!t)t=128;
		int a=*Src_alphachanneldoubleres;
		//*Target++ = t+((s-t)*a)/256;
		int r=  t+( ((s-t)*blend*a)  >>16) ;//  /65536;
		if(!r)r=1;
		*Target++ = r;//t+((s-t)*blend*a)/65536;

	}
	else
	{
		Target++;
	}
    Source += IntPart;
	Src_alphachanneldoubleres+=(IntPart*2);

    E += FractPart;
    if (E >= TgtWidth)
	{
		E -= TgtWidth;
		Source++;
		Src_alphachanneldoubleres+=2;
    }
  }
}




//#######################################################################################
//####################################################################################### ScaleLine variations Standard Blend
//#######################################################################################




void ScaleLine_blend_grey(unsigned char *Target, unsigned char *Source, int SrcWidth, int TgtWidth,int TgtDrawStartOffset, int TgtDrawPixelCount,unsigned char blend)
{
  int NumPixels = TgtDrawPixelCount;//TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;
  //while (NumPixels-- > 0)
  for(NumPixels=0;NumPixels<TgtDrawPixelCount;NumPixels++)
  {
    if(NumPixels>=TgtDrawStartOffset)
	{
		// simple blending
		int s=*Source;
		int t=*Target;
		*Target++ = (t+((s-t)*blend)/256);

		// blend blending from an actual blend channel
		//int a=*Source;
		//*Target++ = t+((s-t)*blend*a)/65536;

		// original none blended code
		//*Target++ = *Source;
	}
	else
	{
		Target++;
	}
    Source += IntPart;

    E += FractPart;
    if (E >= TgtWidth) {
      E -= TgtWidth;
      Source++;
    }
  }
}



void ScaleLine_blend_uv(unsigned char *Target, unsigned char *Source, int SrcWidth, int TgtWidth,int TgtDrawStartOffset, int TgtDrawPixelCount,unsigned char blend)
{
  int NumPixels = TgtDrawPixelCount;//TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;

  for(NumPixels=0;NumPixels<TgtDrawPixelCount;NumPixels++)
  {
    if(NumPixels>=TgtDrawStartOffset)
	{
		// do some blend blending :)
		int s=*Source;
		int t=*Target;
		if(!s)s=128;
		if(!t)t=128;

		*Target++ = t+((s-t)*blend)/256;

		// blend blending from an actual blend channel
		//int a=*Source;
		//*Target++ = t+((s-t)*blend*a)/65536;

		// original none blended code
		//*Target++ = *Source;
	}
	else
	{
		Target++;
	}
    Source += IntPart;

    E += FractPart;
    if (E >= TgtWidth) {
      E -= TgtWidth;
      Source++;
    }
  }
}

//#######################################################################################
//####################################################################################### drawzoomimage_blend_raw
//#######################################################################################

static inline void _blend_raw(

				unsigned char *srcmem,
				int smw,int smh,
				int six,int siy,		// SrcImgPos
				int siw,int sih, 		// SrcImgSize

				unsigned char *dstmem,
				int dmw,int dmh,
				int dix,int diy,		// DstImgPos
				int diw,int dih, 		// DstImgSize
				int plane_is_uv,
				unsigned char *srcalphamem,
				unsigned char blend
				)
{
	if(!siw || !sih) return;
	if(!diw || !dih) return;
	if(diy+dih<0)return;
	if(diy>=dmh)return;
	if(dix+diw<0)return;
	if(dix>=dmw)return;
	if(!blend) return;

int dso;
	if(dix>=0)
		dso=0;
	else
		dso=-dix;

int dpc;
	if(dix+diw<=dmw)
		dpc=diw;
	else
		dpc=dmw-dix;


	unsigned char *dstdataptr = &dstmem[  diy * dmw + dix ];
	unsigned char *srcdataptr = &srcmem[  siy * smw + six ];

    unsigned char *srcalphachannel = NULL;

    if(srcalphamem)
    {

			if(plane_is_uv)
				srcalphachannel = &srcalphamem[  siy * (smw*4) + six ];
			else
				srcalphachannel = &srcalphamem[  siy * (smw  ) + six ];

 	}


	int yNumPixels = dih-1;
	int yIntPart = sih / dih;
	int yFractPart = sih % dih;
	int yE = 0;
	int y=0;
	while (yNumPixels-- >= 0)
	{
		//===================
		//draw a line
		//===================
		int yy=diy+y;

		if(yy>=0 && yy<dmh)
		{

			if(srcalphachannel)
			{
				//liqapp_log("blend");
				if(!plane_is_uv)
					ScaleLine_alphablend_grey(
							  dstdataptr,
							  srcdataptr,
							  siw,
							  diw,
							  dso,
							  dpc,
							  srcalphachannel,
							  blend
							 );
				else
					ScaleLine_alphablend_uv(
							  dstdataptr,
							  srcdataptr,
							  siw,
							  diw,
							  dso,
							  dpc,
							  srcalphachannel,
							  blend
							 );
			}
			else
			if(blend<255)
			{
				if(!plane_is_uv)
					ScaleLine_blend_grey(
							  dstdataptr,
							  srcdataptr,
							  siw,
							  diw,
							  dso,
							  dpc,
							  blend
							 );
				else
					ScaleLine_blend_uv(
							  dstdataptr,
							  srcdataptr,
							  siw,
							  diw,
							  dso,
							  dpc,
							  blend
							 );
			}
			else
			{
				if(!plane_is_uv)
					ScaleLine_grey(
							  dstdataptr,
							  srcdataptr,
							  siw,
							  diw,
							  dso,
							  dpc
							 );
				else
					ScaleLine_uv(
							  dstdataptr,
							  srcdataptr,
							  siw,
							  diw,
							  dso,
							  dpc
							 );
			}


		}
			dstdataptr+=dmw;


		//move source along if required
		srcdataptr += (yIntPart*smw);

		if(srcalphachannel)
		{

			if(plane_is_uv)
				srcalphachannel+=(yIntPart*smw*4);
			else
				srcalphachannel+=(yIntPart*smw);
		}

		yE += yFractPart;
		if (yE >= dih)
		{
			// ready to skip a line
			yE -= dih;
			// skip a line
			srcdataptr+=smw;

			if(srcalphachannel)
			{

				if(plane_is_uv)
					srcalphachannel+=(smw*4);
				else
					srcalphachannel+=(smw);
			}
		}
		y++;
	}
}








void xsurface_drawzoomblendimage(

										liqimage *srcimage,
										int six,int siy,		// SrcImgPos
										int siw,int sih, 		// SrcImgSize

										liqimage *dstimage,
										int dix,int diy,		// DstImgPos
										int diw,int dih, 		// DstImgSize

										unsigned char blend
										)
{
	int smw=srcimage->width;		// SrcMemW
	int smh=srcimage->height;		// SrcMemH

	int dmw=dstimage->width;		// DstMemW
	int dmh=dstimage->height;		// DstMemH

	//liqapp_log("blend: %d, sm:%i,%i  di:%i,%i",blend,smw,smh,diw,dih);




	//liqapp_log("blend: %d, s:xy(%i,%i),wh(%i,%i),im(%i,%i),pc(%i) \t\t  d:xy(%i,%i),wh(%i,%i),im(%i,%i),pc(%i)",blend,  six,siy,siw,sih,smw,smh,srcimage->num_planes,    dix,diy,diw,dih,dmw,dmh,dstimage->num_planes);


	if(diy+dih<0)return;
	if(diy>=dmh)return;

	if(dix+diw<0)return;
	if(dix>=dmw)return;

	unsigned char *alphachannel = NULL;




	if(srcimage->num_planes==2)
	{
		// grey with alpha :)
		alphachannel = &srcimage->data[srcimage->offsets[1]];
	}
	else
	if(srcimage->num_planes==4)
	{
		alphachannel = &srcimage->data[srcimage->offsets[3]];
	}


	// jump if expected dimensions match image dimensions and we are not doing any blending

	if(smw==diw && smh==dih && blend==255 && srcimage->num_planes==3
	   && six==0 && siy==0 && siw==diw && sih==dih)			// 20090812_235925 lcuk : extra round of checks to prevent exactly screensized images from being scaled
	//if(siw==diw && sih==dih && blend==255 && srcimage->num_planes==3)
	{

		// todo allow this bailout to also take an offset into the src image
		// that would allow a 1:1 quick draw operation which is a cut into a window
		// this would improve performance of the default map glide

		// can only quickdraw YUV only images, no alpha support
        // 20090419_225827 lcuk : dont need to log all the time :D
		//liqapp_log("quickdraw..");
		xsurface_drawimage_color(dstimage,srcimage,dix,diy);
		//inline void xsurface_drawimage_color(   liqimage *surface,liqimage *image,int x,int y);
		return;
	}
 		//liqapp_log("slowdraw..");



	_blend_raw(
								&srcimage->data[srcimage->offsets[0]],
								smw,smh,
								six,siy,
								siw,sih,

								&dstimage->data[dstimage->offsets[0]],
								dmw,dmh,
								dix,diy,
								diw,dih,
								0,

								alphachannel,

								blend
							  );
	//return;
	// move onto the color portion
	smw>>=1;
	smh>>=1;
	six>>=1;
	siy>>=1;
	siw>>=1;
	sih>>=1;

	dmw>>=1;
	dmh>>=1;
	dix>>=1;
	diy>>=1;
	diw>>=1;
	dih>>=1;

	if(!siw || !sih) return;
	if(!diw || !dih) return;

	// eliminate greys (they might have had alpha)
	if(srcimage->num_planes<3) return;

	_blend_raw(
								&srcimage->data[srcimage->offsets[1]],
								smw,smh,
								six,siy,
								siw,sih,

								&dstimage->data[dstimage->offsets[1]],
								dmw,dmh,
								dix,diy,
								diw,dih,
								1,
								alphachannel,
								blend
							  );

	_blend_raw(
								&srcimage->data[srcimage->offsets[2]],
								smw,smh,
								six,siy,
								siw,sih,

								&dstimage->data[dstimage->offsets[2]],
								dmw,dmh,
								dix,diy,
								diw,dih,
								1,
								alphachannel,
								blend
							  );



}













void xsurface_drawzoomimage(

										liqimage *srcimage,
										int six,int siy,		// SrcImgPos
										int siw,int sih, 		// SrcImgSize

										liqimage *dstimage,
										int dix,int diy,		// DstImgPos
										int diw,int dih 		// DstImgSize

										)
{
	xsurface_drawzoomblendimage(
										srcimage,
										six,siy,
										siw,sih,
										dstimage,
										dix,diy,
										diw,dih,
										255
										);
}



//########################################################################
//######################################################################## draw a single glyph onto the surface
//########################################################################


void xsurface_drawglyph_grey(liqimage *surface,liqfont *font,int x,int y,unsigned char glyph)
{
	// cleanse this function now, it requires TLC
	// font has all changed and there is no longer a single contigious glyph buffer
	// this is better memory wise, but its more fragmented
	// the offsets are simpler

	//if(x<0)return;

liqfontglyph *g = liqfont_getglyph(font,glyph);
	if(!g)return;
	//if(font->glyphdata[glyph]==NULL) return;
	int gw  =g->glyphw; // font->glyphwidths[glyph];
	int gh  =g->glyphh; // font->glyphheights[glyph];

	int gtw = gw;//font->glyphtilew;
	//int gth = gh;//font->glyphmaxh;

	int sw  = surface->width;
	int sh  = surface->height;

	unsigned int goff = 0;


	if(x<0)
	{
		if(x<-gw) return;
		gw+=x;
		goff-=x;
		x=0;
	}

	unsigned int gskip = gtw-gw;

	//void page_rendertocanvas(PAGE *self,int l,int t,int w,int h)
	//if(page)
	//{
	//	page_rendertocanvas(page,x,y,gw,gh);
	//	return;
	//}

	if(y+gh<0)return;

	if(x+gw>sw)
	{
		if(x>=sw)return;
		gskip+=(x+gw)-(sw);
		gw=(sw-x);
	}

	if(y+gh>sh)
	{
		if(y>=sh) return;
		gh=(sh-y);
	}


	unsigned int poff = sw * y + x;
	unsigned int pskip = sw-gw;
//---------------------------------------
	unsigned char *pdata;
	unsigned char *gdata;
	if(y<0)
	{
		y=-y;
		goff+=gtw*y;
		poff+=sw*y;
		gh-=y;
		y=0;
	}

	gdata = & ((unsigned char*)g->glyphdata) [ goff ] ;
	pdata = & ((unsigned char*)&surface->data[surface->offsets[0]])          [ poff ];

	xsurface_drawstrip_or(gh,gw,gdata,pdata,gskip,pskip);


}

//########################################################################
//######################################################################## draw text as fast as possible onto xv surface :)
//########################################################################

int xsurface_drawtext_grey(liqimage *surface,liqfont *font,int xs,int ys,char *data)
{
	int x=xs;
	unsigned char ch;
	while ( (ch=*data++) )
	{
		xsurface_drawglyph_grey(surface,font,x,ys, ch );
		x+=liqfont_getglyphwidth(font,ch);//font->glyphwidths[ch];
	}
	return x;
}

int xsurface_drawtextn_grey(liqimage *surface,liqfont *font,int xs,int ys,char *data,int datalen)
{
	int x=xs;
	unsigned char ch;
	if(datalen<=0)return x;
	while(datalen--)
	{
		ch=*data++;
		xsurface_drawglyph_grey(surface,font,x,ys, ch );
		x+=liqfont_getglyphwidth(font,ch);//font->glyphwidths[ch];
	}
	return x;
}

//########################################################################
//######################################################################## clear canvass
//########################################################################

void xsurface_drawclear_grey(liqimage *surface,unsigned char grey)
{
	int uo = surface->width*surface->height;
	//int vo = uo + (uo >> 2);
	int uvplanesize = (uo >> 2);
	memset(&surface->data[surface->offsets[0]]      ,grey,surface->width*surface->height);
	memset(&surface->data[surface->offsets[1]] ,128 ,uvplanesize);
	memset(&surface->data[surface->offsets[2]] ,128 ,uvplanesize);
}

void xsurface_drawclear_yuv(liqimage *surface,unsigned char grey,unsigned char u,unsigned char v)
{
	int uo = surface->width*surface->height;
	//int vo = uo + (uo >> 2);
	int uvplanesize = (uo >> 2);
	memset(&surface->data[surface->offsets[0]]      ,grey,surface->width*surface->height);
	memset(&surface->data[surface->offsets[1]] ,u   ,uvplanesize);
	memset(&surface->data[surface->offsets[2]] ,v   ,uvplanesize);
}











//########################################################################
//######################################################################## drawfadeoutrect - aim to fade out a block and end up near the target color with a spread
//########################################################################



void xsurface_drawfadeoutrect_yuv(liqimage *surface,int x,int y,int w,int h, unsigned char grey,unsigned char u,unsigned char v,unsigned char spread)
{
	if(x+w<0)return;
	if(y+h<0)return;
	if(w<=0)return;
	if(h<=0)return;
	if(y<0)	{		h=h+y;		y=0;	}
	if(x<0)	{		w=w+x;		x=0;	}
	if(x+w>=surface->width)
	{
		if(x>=surface->width) return;
		w=(surface->width-x)-1;
	}
	if(y+h>=surface->height)
	{
		if(y>=surface->height) return;
		h=(surface->height-y)-1;
	}
    
//unsigned char spreaddiv2 = spread/2;
    
	

//unsigned int grey4;
//	grey4=grey<<24 | grey<<16 | grey<<8 | grey;
	register unsigned int xx,yy;
	register unsigned char *pdata;
	//register unsigned int *epdata;
	for (yy = y; yy < (y+h); yy++)
	{
		pdata = &surface->data[ surface->offsets[0] + (surface->width*yy) + x ];
		//epdata=(unsigned int*)pdata;
		//for (xx = x; (xx+4) < (x+w); xx+=4)
		//{
		//	*epdata++ = grey4;
		//}
        
		// 20090615_193133 lcuk : hmmm this should not be here, how did this run at first i wonder..
		//pdata=(unsigned char *)epdata;
		// 20090615_193146 lcuk : and this, it wasn't initialized..
        xx=x;
		while((xx) <= (x+w)) 					// 25jan2009:gb was <
		{
			xx++;
			//*pdata++ = (unsigned char)grey;
            //*pdata++ = y + (((((int)*pdata) * 255) / spread)-spreaddiv2);
			{
				int s=grey;
				int t=*pdata;
				//if(!s)s=128;
				//if(!t)t=128;
				
				*pdata++ = t+((s-t)*spread)/256;
			}
		}
		
		
		
		
		

		
		
		
		

	}


	y>>=1;
	x>>=1;
	w>>=1;
	h>>=1;
	if(w<1 || h<1) return;
	// same process now for the color ranges, but we will use shorts

	unsigned short u2;
	unsigned short v2;
	u2=u<<8 | u;
	v2=v<<8 | v;
	register unsigned char *udata;
	//register unsigned short *eudata;
	register unsigned char *vdata;
	//register unsigned short *evdata;
	
	unsigned int pw = surface->width;
	unsigned int ph = surface->height;
	unsigned int uo = surface->offsets[1];//pw*ph;
	unsigned int vo = surface->offsets[2];//uo + (uo >> 2);
	pw>>=1;
	ph>>=1;
	for (yy = y; yy < (y+h); yy++)
	{
		udata = &surface->data[ uo+(pw*yy) + x ];
		vdata = &surface->data[ vo+(pw*yy) + x ];

		//eudata=(unsigned short*)udata;
		//evdata=(unsigned short*)vdata;
		//for (xx = x; (xx+2) < (x+w); xx+=2)
		//{
		//	//liqapp_log("xy(%i,%i),   %i",xx,yy,(unsigned int)epdata);
		//	*eudata++ = u2;
		//	*evdata++ = v2;
		//}
		//udata=(unsigned char *)eudata;
		//vdata=(unsigned char *)evdata;
		
		
		xx = x;
		while((xx) <= (x+w))                  // 25jan2009:gb was <
		{
			xx++;
			
			//*udata++ = (unsigned char)u;
			//*vdata++ = (unsigned char)v;
            
            //*udata++ = u + (((((int)*udata) * 255) / spread)-spreaddiv2);
            //*vdata++ = v + (((((int)*vdata) * 255) / spread)-spreaddiv2);
            
			{
				int s=u;
				int t=*udata;
				if(!s)s=128;
				if(!t)t=128;	
				*udata++ = t+((s-t)*spread)/256;
			}
			{
				int s=v;
				int t=*vdata;
				if(!s)s=128;
				if(!t)t=128;	
				*vdata++ = t+((s-t)*spread)/256;
			}

		}
	}
}






//########################################################################
//######################################################################## drawrect color optimised (older than other fns)
//########################################################################
//fixed:todo: fix bug with unaligned widths, at present it does not fill upto 3 right hand side pixels

void xsurface_drawrect_yuv(liqimage *surface,int x,int y,int w,int h, unsigned char grey,unsigned char u,unsigned char v)
{
	if(x+w<0)return;
	if(y+h<0)return;
	if(w<=0)return;
	if(h<=0)return;
	if(y<0)	{		h=h+y;		y=0;	}
	if(x<0)	{		w=w+x;		x=0;	}
	if(x+w>=surface->width)
	{
		if(x>=surface->width) return;
		w=(surface->width-x)-1;
	}
	if(y+h>=surface->height)
	{
		if(y>=surface->height) return;
		h=(surface->height-y);
	}

unsigned int grey4;
	grey4=grey<<24 | grey<<16 | grey<<8 | grey;
	register unsigned int xx,yy;
	register unsigned char *pdata;
	register unsigned int *epdata;
	for (yy = y; yy < (y+h); yy++)
	{
		pdata = &surface->data[ surface->offsets[0] + (surface->width*yy) + x ];
		epdata=(unsigned int*)pdata;
		for (xx = x; (xx+4) < (x+w); xx+=4)
		{
			*epdata++ = grey4;
		}

		pdata=(unsigned char *)epdata;
		while((xx) <= (x+w)) 					// 25jan2009:gb was <
		{
			xx++;
			*pdata++ = (unsigned char)grey;
		}

	}

	y>>=1;
	x>>=1;
	w>>=1;
	h>>=1;
	if(w<1 || h<1) return;
	// same process now for the color ranges, but we will use shorts

	unsigned short u2;
	unsigned short v2;
	u2=u<<8 | u;
	v2=v<<8 | v;
	register unsigned char *udata;
	register unsigned short *eudata;
	register unsigned char *vdata;
	register unsigned short *evdata;
	unsigned int pw = surface->width;
	unsigned int ph = surface->height;
	unsigned int uo = surface->offsets[1];//pw*ph;
	unsigned int vo = surface->offsets[2];//uo + (uo >> 2);
	pw>>=1;
	ph>>=1;
	for (yy = y; yy < (y+h); yy++)
	{
		udata = &surface->data[ uo+(pw*yy) + x ];
		eudata=(unsigned short*)udata;
		vdata = &surface->data[ vo+(pw*yy) + x ];
		evdata=(unsigned short*)vdata;
		for (xx = x; (xx+2) < (x+w); xx+=2)
		{
			//liqapp_log("xy(%i,%i),   %i",xx,yy,(unsigned int)epdata);
			*eudata++ = u2;
			*evdata++ = v2;
		}
		udata=(unsigned char *)eudata;
		vdata=(unsigned char *)evdata;
		while((xx) <= (x+w))                  // 25jan2009:gb was <
		{
			xx++;
			*udata++ = (unsigned char)u;
			*vdata++ = (unsigned char)v;
		}
	}
}

//########################################################################
//######################################################################## drawrectwash uv wash
//########################################################################
//fixed:todo: fix bug with unaligned widths, at present it does not fill upto 3 right hand side pixels
void xsurface_drawrectwash_uv(liqimage *surface,int x,int y,int w,int h, unsigned char u,unsigned char v)
{
	if(x+w<0)return;
	if(y+h<0)return;
	if(w<=0)return;
	if(h<=0)return;
	if(y<0)	{		h=h+y;		y=0;	}
	if(x<0)	{		w=w+x;		x=0;	}
	if(x+w>=surface->width)
	{
		if(x>=surface->width) return;
		w=(surface->width-x);
	}
	if(y+h>=surface->height)
	{
		if(y>=surface->height) return;
		h=(surface->height-y);
	}
	if(y&1){ y--;h++; }
	if(h&1){     h++; }
/*
unsigned int grey4;
	grey4=grey<<24 | grey<<16 | grey<<8 | grey;

*/
	register unsigned int xx,yy;
	//register unsigned char *pdata;
	//register unsigned int *epdata;
/*
	for (yy = y; yy < (y+h); yy++)
	{
		pdata = &surface->data[ (surface->width*yy) + x ];
		epdata=(unsigned int*)pdata;
		for (xx = x; xx < (x+w); xx+=4)
		{
			//liqapp_log("xy(%i,%i),   %i",xx,yy,(unsigned int)epdata);
			*epdata++ = grey4;
		}
	}
*/
	y>>=1;
	x>>=1;
	w>>=1;
	h>>=1;
	if(w<1 || h<1) return;
	// same process now for the color ranges, but we will use shorts

unsigned short u2;
	u2=u<<8 | u;
unsigned short v2;
	v2=v<<8 | v;
	register unsigned char *udata;
	register unsigned short *eudata;
	register unsigned char *vdata;
	register unsigned short *evdata;
	unsigned int pw=surface->width;
	unsigned int ph=surface->height;
	unsigned int uo = surface->offsets[1];//pw*ph;
	unsigned int vo = surface->offsets[2];//uo + (uo >> 2);
	pw>>=1;
	ph>>=1;
	for (yy = y; yy < (y+h); yy++)
	{
		udata = &surface->data[ uo+(pw*yy) + x ];
		eudata=(unsigned short*)udata;
		vdata = &surface->data[ vo+(pw*yy) + x ];
		evdata=(unsigned short*)vdata;
		for (xx = x; (xx+2) < (x+w); xx+=2)
		{
			//liqapp_log("xy(%i,%i),   %i",xx,yy,(unsigned int)epdata);
			*eudata++ = u2;
			*evdata++ = v2;
		}


		udata=(unsigned char *)eudata;
		vdata=(unsigned char *)evdata;
		while((xx) < (x+w))
		{
			xx++;
			*udata++ = (unsigned char)u;
			*vdata++ = (unsigned char)v;
		}


	}
}

//########################################################################
//######################################################################## drawrect grey optimised
//########################################################################
//fixed:todo: fix bug with unaligned widths, at present it does not fill upto 3 right hand side pixels
void xsurface_drawrect_grey(liqimage *surface,int x,int y,int w,int h, unsigned char grey)
{
	if(x+w<0)return;
	if(y+h<0)return;
	if(w<=0)return;
	if(h<=0)return;
	if(y<0)	{		h=h+y;		y=0;	}
	if(x<0)	{		w=w+x;		x=0;	}
	if(x+w>=surface->width)
	{
		if(x>=surface->width) return;
		w=(surface->width-x);
	}
	if(y+h>=surface->height)
	{
		if(y>=surface->height) return;
		h=(surface->height-y);
	}

unsigned int grey4;
	grey4=grey<<24 | grey<<16 | grey<<8 | grey;
	register unsigned int xx,yy;
	register unsigned char *pdata;
	register unsigned int *epdata;
	for (yy = y; yy < (y+h); yy++)
	{
		pdata = &surface->data[ surface->offsets[0] + (surface->width*yy) + x ];
		epdata=(unsigned int*)pdata;
		for (xx = x; (xx+4) < (x+w); xx+=4)
		{
			//liqapp_log("xy(%i,%i),   %i",xx,yy,(unsigned int)epdata);
			*epdata++ = grey4;
		}
		pdata=(unsigned char *)epdata;
		while((xx) < (x+w))
		{
			xx++;
			*pdata++ = (unsigned char)grey;
		}
	}
}

//########################################################################
//######################################################################## pset color
//########################################################################

void xsurface_drawpset_yuv(liqimage *surface,int x,int y,char grey,char u,char v)
{
	if(x<0)return;
	if(y<0)return;
	if(x>=surface->width)return;
	if(y>=surface->height)return;
//	int uo = surface->width*surface->height;
//	int vo = uo + (uo >> 2);

	unsigned int uo = surface->offsets[1];//pw*ph;
	unsigned int vo = surface->offsets[2];//uo + (uo >> 2);

	int uvw = surface->width>>1;
	surface->data[ surface->offsets[0] + (surface->width*y+ x)] = grey;
	x>>=1;
	y>>=1;
	surface->data[uo + (uvw*y+ x)] = u;
	surface->data[vo + (uvw*y+ x)] = v;

}

//########################################################################
//######################################################################## pset grey
//########################################################################

void xsurface_drawpset_grey(liqimage *surface,int x,int y,char grey)
{
	if(x<0)return;
	if(y<0)return;
	if(x>=surface->width)return;
	if(y>=surface->height)return;
	surface->data[ surface->offsets[0] + (surface->width*y+ x)] = grey;
}

//########################################################################
//######################################################################## pget color
//########################################################################

#define interal_getchar(x,y,buffer,linewidth)  (buffer)[ (linewidth) * (y) + (x) ]

void xsurface_drawpget_yuv(liqimage *surface,int x1, int y1, unsigned char *grey,unsigned char *u,unsigned char *v)
{
unsigned int pw=surface->width;
//unsigned int ph=surface->height;
//unsigned int uo = pw*ph;
//unsigned int vo = uo + (uo >> 2);
	unsigned int uo = surface->offsets[1];//pw*ph;
	unsigned int vo = surface->offsets[2];//uo + (uo >> 2);


//int px=x1,py=y1;
	grey[0]=interal_getchar(x1,y1,&surface->data[0 ],pw);
	x1>>=1;
	y1>>=1;
	pw>>=1;
	u[0]=interal_getchar(x1,y1,&surface->data[uo],pw);
	v[0]=interal_getchar(x1,y1,&surface->data[vo],pw);
}









//########################################################################
//######################################################################## line internal AA thickline
//########################################################################



 // dont like this though, its close but still feels more messy than direct pixels
 // might do something about that
 // also, ensure only the grey channel goes through here, theres no real need to smear the UV any further


// from a vb class here:
// http://www.bigresource.com/VB-Anti-aliasing-text-8vUifLfKBe.html

/*
#define kkointeral_linepaintchar_alpha(x,y,c,alpha,buffer,linewidth) \
{ int off = (linewidth) * (y) + (x);  int b=buffer[ off ]; int cc=(c); float a=(alpha); buffer[ off ] = (b) + ((float)((cc)-(b)) * (a)); }



#define interal_linepaintchar_alpha(x,y,c,alpha,buffer,linewidth) \
{ int off = (linewidth) * (y) + (x);  int b=buffer[ off ]; int cc=(c); float a=(alpha); buffer[ off ] = (b*(1.0-a)) + (cc*(a)); }


#define llinteral_linepaintchar_alpha(x,y,c,alpha,buffer,linewidth) \
{ int off = (linewidth) * (y) + (x);  int b=buffer[ off ]; buffer[ off ] = (c) ; }
 */

#define interal_linepaintchar_alpha(x,y,c,alpha,buffer,linewidth) \
{ int off = (linewidth) * (y) + (x);  int b=(unsigned char)buffer[ off ]; int cc=(unsigned char)(c); float a=(alpha); buffer[ off ] = b + (cc-b)*a; }

static inline float fracpart(float a)
{
	float b=((int)(a));
      
	return a - b;
}






void xsurface_interalline_aa(liqimage *surface,int x1, int y1, int x2, int y2, char grey,char *buffer,int linewidth,unsigned char thickness)
{
float deltax, deltay;
int loopc;
int start , finish;
float dx, dy, dydx;

	//grey=255;


//char LR , LG, LB;
	//	xsurface_interalline_noaa(surface,x1,y1,x2,y2,grey/2,buffer,linewidth);

	deltax = abs(x2 - x1); //' Calculate deltax and deltay for initialisation
	deltay = abs(y2 - y1);


		//LR = (AColor And &HFF&)
		//LG = (AColor And &HFF00&) / &H100&
		//LB = (AColor And &HFF0000) / &H10000
		if (deltax > deltay) //' horizontal or vertical
		{
			if (y2 > y1) //' determine rise and run
			{
				dydx = -(deltay / deltax);
			}
			else
			{
				dydx = deltay / deltax;
			}
			if (x2 < x1)
			{
				start = x2; //' right to left
				finish = x1;
				dy = y2;
			}
			else
			{
				start = x1; //' left to right
				finish = x2;
				dy = y1;
				dydx = -dydx; //' inverse slope
			}
			
			
			

					
				//liqapp_log("dy=%3.5f  (1.0-f(dy))=%3.5f  f(dy)=%3.5f",dy,1.0-fracpart(dy),fracpart(dy));
				//AlphaBlendPixel (hdc, loopc, CInt(dy - 0.5), LR, LG, LB, 1 - FracPart(dy));
				//AlphaBlendPixel (hdc, loopc, CInt(dy - 0.5) + 1, LR, LG, LB, FracPart(dy));
				
			if(thickness==1)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(loopc,((int)(dy))    ,grey,1.0               ,buffer,linewidth);					
					dy = dy + dydx; //' next point
				}
			}
			else
			if(thickness==2)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(loopc,((int)(dy))    ,grey,1.0 - fracpart(dy),buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy)) + 1,grey,      fracpart(dy),buffer,linewidth);
					dy = dy + dydx; //' next point
				}
			}
			else
			if(thickness==3)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(loopc,((int)(dy)) - 1,grey,1.0 - fracpart(dy),buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy))    ,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy)) + 1,grey,      fracpart(dy),buffer,linewidth);
					dy = dy + dydx; //' next point
				}
			}
			else
			if(thickness==4)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(loopc,((int)(dy)) - 1,grey,1.0 - fracpart(dy),buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy))    ,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy)) + 1,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy)) + 2,grey,      fracpart(dy),buffer,linewidth);
					dy = dy + dydx; //' next point
				}
			}
			else
			//if(thickness==5)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(loopc,((int)(dy)) - 2,grey,1.0 - fracpart(dy),buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy)) - 1,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy))    ,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy)) + 1,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy)) + 2,grey,      fracpart(dy),buffer,linewidth);
					dy = dy + dydx; //' next point
				}
			}			
			
			
			
			

		}
		else
		{
			if(x2 > x1)  //' determine rise and run
			{
				dydx = -(deltax / deltay);
			}
			else
			{
				dydx = deltax / deltay;
			}
			if(y2 < y1)
			{
				start = y2; //' right to left
				finish = y1;
				dx = x2;
			}
			else
			{
				start = y1; //' left to right
				finish = y2;
				dx = x1;
				dydx = -dydx; //' inverse slope
			}
			
			
			if(thickness==1)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(((int)(dx))    ,loopc,grey,1.0               ,buffer,linewidth);
					dx = dx + dydx; //' next point
				}
			}
			else
			if(thickness==2)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(((int)(dx))    ,loopc,grey,1.0 - fracpart(dx),buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) + 1,loopc,grey,      fracpart(dx),buffer,linewidth);
					dx = dx + dydx; //' next point
				}
			}
			else
			if(thickness==3)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(((int)(dx)) - 1,loopc,grey,1.0 - fracpart(dx),buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx))    ,loopc,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) + 1,loopc,grey,      fracpart(dx),buffer,linewidth);
					dx = dx + dydx; //' next point
				}
			}
			else
			if(thickness==4)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(((int)(dx)) - 1,loopc,grey,1.0 - fracpart(dx),buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx))    ,loopc,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) + 1,loopc,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) + 2,loopc,grey,      fracpart(dx),buffer,linewidth);
					dx = dx + dydx; //' next point
				}
			}
			else
			//if(thickness==4)
			{
				for(loopc = start; loopc<finish;loopc++)
				{
					interal_linepaintchar_alpha(((int)(dx)) - 2,loopc,grey,1.0 - fracpart(dx),buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) - 1,loopc,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx))    ,loopc,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) + 1,loopc,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) + 2,loopc,grey,      fracpart(dx),buffer,linewidth);
					dx = dx + dydx; //' next point
				}
			}
		}


}



/*


inline void xsurface_interalline_aa_uv(liqimage *surface,int x1, int y1, int x2, int y2, char grey,char *buffer,int linewidth,unsigned char thickness)
{
float deltax, deltay;
int loopc;
int start , finish;
float dx, dy, dydx;

	//grey=255;
	linewidth/=2;
	if(linewidth<=0)linewidth=1;


//char LR , LG, LB;
	//	xsurface_interalline_noaa(surface,x1,y1,x2,y2,grey/2,buffer,linewidth);

	deltax = abs(x2 - x1); //' Calculate deltax and deltay for initialisation
	deltay = abs(y2 - y1);
	// 20090708_211634 lcuk : do all right now during test


		//LR = (AColor And &HFF&)
		//LG = (AColor And &HFF00&) / &H100&
		//LB = (AColor And &HFF0000) / &H10000
		if (deltax > deltay) //' horizontal or vertical
		{
			if (y2 > y1) //' determine rise and run
			{
				dydx = -(deltay / deltax);
			}
			else
			{
				dydx = deltay / deltax;
			}
			if (x2 < x1)
			{
				start = x2; //' right to left
				finish = x1;
				dy = y2;
			}
			else
			{
				start = x1; //' left to right
				finish = x2;
				dy = y1;
				dydx = -dydx; //' inverse slope
			}
			for(loopc = start; loopc<finish;loopc++)
			{
				//liqapp_log("dy=%3.5f  (1.0-f(dy))=%3.5f  f(dy)=%3.5f",dy,1.0-fracpart(dy),fracpart(dy));
				//AlphaBlendPixel (hdc, loopc, CInt(dy - 0.5), LR, LG, LB, 1 - FracPart(dy));
				//AlphaBlendPixel (hdc, loopc, CInt(dy - 0.5) + 1, LR, LG, LB, FracPart(dy));
				if(thickness==1)
				{
					interal_linepaintchar_alpha(loopc,((int)(dy))    ,grey,1.0               ,buffer,linewidth);
				}
				else
 				if(thickness==2)
				{
					interal_linepaintchar_alpha(loopc,((int)(dy))    ,grey,1.0 - fracpart(dy),buffer,linewidth);
 					interal_linepaintchar_alpha(loopc,((int)(dy)) + 1,grey,      fracpart(dy),buffer,linewidth);
				}
				else
				{
					interal_linepaintchar_alpha(loopc,((int)(dy)) - 1,grey,1.0 - fracpart(dy),buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy))    ,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(loopc,((int)(dy)) + 1,grey,      fracpart(dy),buffer,linewidth);
				}

				dy = dy + dydx; //' next point
			}

		}
		else
		{
			if(x2 > x1)  //' determine rise and run
			{
				dydx = -(deltax / deltay);
			}
			else
			{
				dydx = deltax / deltay;
			}
			if(y2 < y1)
			{
				start = y2; //' right to left
				finish = y1;
				dx = x2;
			}
			else
			{
				start = y1; //' left to right
				finish = y2;
				dx = x1;
				dydx = -dydx; //' inverse slope
			}
			for(loopc = start; loopc<finish;loopc++)
			{
				//AlphaBlendPixel hdc, CInt(dx - 0.5), loopc, LR, LG, LB, 1 - FracPart(dx)
				//AlphaBlendPixel hdc, CInt(dx - 0.5) + 1, loopc, LR, LG, LB, FracPart(dx)

				if(thickness==1)
				{
					interal_linepaintchar_alpha(((int)(dx))    ,loopc,grey,1.0               ,buffer,linewidth);
				}
				else
 				if(thickness==2)
				{
					interal_linepaintchar_alpha(((int)(dx))    ,loopc,grey,1.0 - fracpart(dx),buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) + 1,loopc,grey,      fracpart(dx),buffer,linewidth);
				}
				else
				{
					interal_linepaintchar_alpha(((int)(dx)) - 1,loopc,grey,1.0 - fracpart(dx),buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx))    ,loopc,grey,1.0               ,buffer,linewidth);
					interal_linepaintchar_alpha(((int)(dx)) + 1,loopc,grey,      fracpart(dx),buffer,linewidth);
				}


				dx = dx + dydx; //' next point
			}

		}


}


*/







//#############################################################################
//#############################################################################
//#############################################################################
//#############################################################################


void xsurface_drawthickline_yuv(liqimage *surface,int x1, int y1, int x2, int y2,unsigned char thickness, char grey,char u,char v)
{
	// if you call this, even with thickness 1, it will still run through the entire set
	// this maybe slightly worrysome
	if(x1<0)return;
	if(y1<0)return;
	if(x1>=surface->width)return;
	if(y1>=surface->height)return;
	if(x2<0)return;
	if(y2<0)return;
	if(x2>=surface->width)return;
	if(y2>=surface->height)return;


unsigned int pw=surface->width;
//unsigned int ph=surface->height;
//unsigned int uo = pw*ph;
//unsigned int vo = uo + (uo >> 2);
	unsigned int uo = surface->offsets[1];//pw*ph;
	unsigned int vo = surface->offsets[2];//uo + (uo >> 2);
	xsurface_interalline_aa(surface,x1,y1,   x2,y2,grey, (char *)&surface->data[surface->offsets[0]], pw, thickness);
	x1>>=1;
	y1>>=1;
	x2>>=1;
	y2>>=1;
	pw>>=1;
	if(thickness&1)thickness++;
	thickness/=2;
	if(thickness==0)thickness=1;
	//xsurface_interalline_aa_uv(surface,x1,y1,   x2,y2,u   ,(char *)&surface->data[uo], pw, thickness);
	//xsurface_interalline_aa_uv(surface,x1,y1,   x2,y2,v   ,(char *)&surface->data[vo], pw, thickness);
	xsurface_interalline_aa(surface,x1,y1,   x2,y2,u   ,(char *)&surface->data[uo], pw, thickness);
	xsurface_interalline_aa(surface,x1,y1,   x2,y2,v   ,(char *)&surface->data[vo], pw, thickness);
}


















//########################################################################
//######################################################################## line internal
//########################################################################

#define interal_linepaintchar(x,y,c,buffer,linewidth)  buffer[ (linewidth) * (y) + (x) ] = (c)

void xsurface_interalline_noaa(liqimage *surface,int x1, int y1, int x2, int y2, char grey,char *buffer,int linewidth)
{
                // 20090520_012704 lcuk : boundary is already confirmed as existing within the buffer
                // 20090520_012727 lcuk : this is a greyscale line and it exhibits aliasing effects
                // 20090520_012833 lcuk : should be smooth and blended with the surrounding contents
 	int dx=x2-x1;		// distance
	int dy=y2-y1;
	int dxabs=ABS(dx);	// absolute
	int dyabs=ABS(dy);
	int sdx=SGN(dx);	// sign (direction)
	int sdy=SGN(dy);
	int x=dyabs>>1;		// abs centre
	int y=dxabs>>1;
	int px=x1;			// start point
	int py=y1;
	int i;

	interal_linepaintchar(px,py,grey,buffer,linewidth);
	if (dxabs>=dyabs)
	{
		// Line runs hoizontally
    	for(i=0;i<dxabs;i++)
		{
			y+=dyabs;
			if (y>=dxabs)
			{
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			interal_linepaintchar(px,py,grey,buffer,linewidth);
		}
	}
	else
	{
		// Line runs vertically
		for(i=0;i<dyabs;i++)
		{
			x+=dxabs;
			if (x>=dyabs)
			{
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			interal_linepaintchar(px,py,grey,buffer,linewidth);
		}
	}
}






























#define interal_linepaintchar_invert(x,y,buffer,linewidth)  { unsigned char c=buffer[ (linewidth) * (y) + (x) ];buffer[ (linewidth) * (y) + (x) ] = 255-c; }

void xsurface_interalline_invert(liqimage *surface,int x1, int y1, int x2, int y2, char *buffer,int linewidth)
{
	int dx=x2-x1;		// distance
	int dy=y2-y1;
	int dxabs=ABS(dx);	// absolute
	int dyabs=ABS(dy);
	int sdx=SGN(dx);	// sign (direction)
	int sdy=SGN(dy);
	int x=dyabs>>1;		// abs centre
	int y=dxabs>>1;
	int px=x1;			// start point
	int py=y1;
	int i;

	interal_linepaintchar_invert(px,py,buffer,linewidth);
	if (dxabs>=dyabs)
	{
		// Line runs hoizontally
    	for(i=0;i<dxabs;i++)
		{
			y+=dyabs;
			if (y>=dxabs)
			{
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			interal_linepaintchar_invert(px,py,buffer,linewidth);
		}
	}
	else
	{
		// Line runs vertically
		for(i=0;i<dyabs;i++)
		{
			x+=dxabs;
			if (x>=dyabs)
			{
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			interal_linepaintchar_invert(px,py,buffer,linewidth);
		}
	}
}

//########################################################################
//######################################################################## linecolor
//########################################################################


void xsurface_drawline_yuv(liqimage *surface,int x1, int y1, int x2, int y2, char grey,char u,char v)
{
	if(x1<0)return;
	if(y1<0)return;
	if(x1>=surface->width)return;
	if(y1>=surface->height)return;
	if(x2<0)return;
	if(y2<0)return;
	if(x2>=surface->width)return;
	if(y2>=surface->height)return;


unsigned int pw=surface->width;
//unsigned int ph=surface->height;
//unsigned int uo = pw*ph;
//unsigned int vo = uo + (uo >> 2);
	unsigned int uo = surface->offsets[1];//pw*ph;
	unsigned int vo = surface->offsets[2];//uo + (uo >> 2);
// if changing to aa, just take the grey for now, the uv variation fails
    // 20090708_203046 lcuk : try this once again
	xsurface_interalline_noaa(surface,x1,y1,   x2,y2,grey, (char *)&surface->data[surface->offsets[0]], pw);
	//xsurface_interalline_noaa(surface,x1,y1,   x2,y2,grey, (char *)&surface->data[surface->offsets[0]], pw);
	x1>>=1;
	y1>>=1;
	x2>>=1;
	y2>>=1;
	pw>>=1;
	//xsurface_interalline_noaa(surface,x1,y1,   x2,y2,u   ,(char *)&surface->data[uo], pw);
	//xsurface_interalline_noaa(surface,x1,y1,   x2,y2,v   ,(char *)&surface->data[vo], pw);

	xsurface_interalline_noaa(surface,x1,y1,   x2,y2,u   ,(char *)&surface->data[uo], pw);
	xsurface_interalline_noaa(surface,x1,y1,   x2,y2,v   ,(char *)&surface->data[vo], pw);
}




//########################################################################
//######################################################################## linegrey
//########################################################################

void xsurface_drawline_grey(liqimage *surface,int x1, int y1, int x2, int y2, char grey)
{
	if(x1<0)return;
	if(y1<0)return;
	if(x1>=surface->width)return;
	if(y1>=surface->height)return;
	if(x2<0)return;
	if(y2<0)return;
	if(x2>=surface->width)return;
	if(y2>=surface->height)return;


unsigned int pw=surface->width;
//unsigned int ph=surface->height;
//unsigned int uo = pw*ph;
//unsigned int vo = uo + (uo >> 2);

// if changing to aa, just take the grey for now, the uv variation fails
	//xsurface_interalline(surface,x1,y1,   x2,y2,grey, &surface->data[surface->offsets[0]], pw);
	xsurface_interalline_noaa(surface,x1,y1,   x2,y2,grey, (char *)&surface->data[surface->offsets[0]], pw);
}

//########################################################################
//######################################################################## linegreyinvert
//########################################################################

void xsurface_drawline_greyinv(liqimage *surface,int x1, int y1, int x2, int y2)
{
	if(x1<0)return;
	if(y1<0)return;
	if(x1>=surface->width)return;
	if(y1>=surface->height)return;
	if(x2<0)return;
	if(y2<0)return;
	if(x2>=surface->width)return;
	if(y2>=surface->height)return;


unsigned int pw=surface->width;
//unsigned int ph=surface->height;
	xsurface_interalline_invert(surface,x1,y1,   x2,y2, (char *)&surface->data[surface->offsets[0]], pw);
}

//########################################################################
//######################################################################## linegreyinvert
//########################################################################

void interal_linepaintcharf(int x, int y,char c,char *buffer,int linewidth,int numlines)
{
	if(x<0)return;
	if(y<0)return;
	if(x>=linewidth)return;
	if(y>=numlines)return;
	interal_linepaintchar(x,y,                         c,buffer,linewidth);

}

#define interal_circlepaintchar(cx,cy,x,y,c,buffer,linewidth,numlines)  \
		{ \
			interal_linepaintcharf(cx+x,cy+y,                         c,buffer,linewidth,numlines); \
			interal_linepaintcharf(cx+x,cy-y,                         c,buffer,linewidth,numlines); \
			interal_linepaintcharf(cx-x,cy+y,                         c,buffer,linewidth,numlines); \
			interal_linepaintcharf(cx-x,cy-y,                         c,buffer,linewidth,numlines); \
			interal_linepaintcharf(cx+y,cy+x,                         c,buffer,linewidth,numlines); \
			interal_linepaintcharf(cx+y,cy-x,                         c,buffer,linewidth,numlines); \
			interal_linepaintcharf(cx-y,cy+x,                         c,buffer,linewidth,numlines); \
			interal_linepaintcharf(cx-y,cy-x,                         c,buffer,linewidth,numlines); \
		}

void xsurface_interalcircle(int cx, int cy, int r,char grey,char *buffer,int linewidth,int numlines)
{
int d=3-(2*r);
int x=0;
int y=r;
	for(x=0;x<y;x++)
	{
		interal_circlepaintchar(cx,cy,x,y,grey,buffer,linewidth,numlines);
		if(d<0)
			d+=(x<<2)+6;
		else
		{
			d+=((x-y)<<2)+10;
			y--;
		}
	}
}

//########################################################################
//######################################################################## circlegrey
//########################################################################

void xsurface_drawcircle_grey(liqimage *surface,int cx, int cy, int r,unsigned char grey)
{
unsigned int pw=surface->width;
unsigned int ph=surface->height;
//unsigned int uo = pw*ph;
//unsigned int vo = uo + (uo >> 2);
	xsurface_interalcircle(cx,cy,r,grey, (char *)&surface->data[surface->offsets[0]], pw,ph);
}

#ifdef __cplusplus
}
#endif

