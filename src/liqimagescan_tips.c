
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


#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define ABS(x) (((x) >= 0) ? (x) : (-(x)))
#define SGN(x) (((x) >= 0) ? 1 : -1)


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



//#####################################################################

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
static int spanupto=0;

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


//#####################################################################


struct tip
{
	int x;			// position along the line
	int xdif;		// space between this and the previous tip
	int grey;		// the actual pcy value
	int tipcode;	// hill or valley
	int isguard;	// is a guard mark, requires 1 previous and one next tip instance to tell
	int ishot;
};
#define tipcount 1000
static struct tip tips[tipcount];
static int tipupto=0;

static void liqimage_tipmark(liqimage *self,int x,int grey,int tipcode)
{
	if(tipupto>=tipcount)return;
	tips[tipupto].x=x;
	if(tipupto>0)
		tips[tipupto].xdif=x-tips[tipupto-1].x;
	else
		tips[tipupto].xdif=0;		
	tips[tipupto].grey=grey;
	tips[tipupto].tipcode=tipcode;
	tips[tipupto].isguard=0;
	tips[tipupto].ishot=0;
	if(tipupto>=2)
	{
		// this is the 3rd tip we have added
		// only now can we start identifying if the previous is a guardbar!
			int n=tipupto-1;
			if( (tips[n-1].tipcode==1) && (tips[n].tipcode==0) && (tips[n+1].tipcode==1)    )
			{
				// 101, check widths match
				if(isnear(tips[n  ].xdif,tips[n+1].xdif,3)  && tips[n  ].xdif<3)
				{
					tips[n].isguard=1;
				}
			}
	}
	
	
	
	tipupto++;	

}
//#####################################################################







static void spanstretch(unsigned char *src,int srclen,unsigned char *dest,int destlen)
{
	// stretch srclen characters of src all the way along dest so they occupt destlen space
	
	
	
	
	int ppx;
	//for(ppx=0;ppx<destlen;ppx++)
	{
	//	*dest++ = *src++;
	}
	//return;
 
 
 
	for(ppx=0;ppx<destlen;ppx++)
	{
		int xx = ppx * (srclen-1) / (destlen-1);
		//*dest++=ppx;
		//if( src[xx]<32)
		//	*dest++ = 32;
		//else
			*dest++ = src[xx];
	}
	return;



	int dx=destlen;		// distance						// 96
	int dy=srclen;										// 200
	
	int dxabs=ABS(dx);	// absolute
	int dyabs=ABS(dy);
	int sdx=SGN(dx);	// sign (direction)
	int sdy=SGN(dy);
	int x=dyabs>>1;		// abs centre
	int y=dxabs>>1;
	int px=0;	//x1;		// start point
	int py=0;   //y1;
	int i;
	int pch=0;//src[py];
	int pcnt=0;//1;
	//interal_linepaintchar_invert(px,py,buffer,linewidth);
	//dest[px] = src[py];
	pch+=src[py]; pcnt++;
	
	if (dxabs>=dyabs)
	{
		// more source than dest - upscaling
		// Line runs hoizontally
    	for(i=0;i<dxabs;i++)
		{
			y+=dyabs;
			if (y>=dxabs)
			{
				if(pcnt){ dest[px] = pch/pcnt; pcnt=0; pch=0; }
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			//interal_linepaintchar_invert(px,py,buffer,linewidth);
			//dest[px] = src[py];
			pch+=src[py]; pcnt++;
		//	liqapp_log("spreadH %d,%d: %d,%d",dx,dy,px,py);
		}
		if(pcnt){ dest[px] = pch/pcnt; pcnt=0; pch=0; }
	}
	else
	{
		// more dest than source, downsizing
		// Line runs vertically
		for(i=0;i<dyabs;i++)
		{
			x+=dxabs;
			if (x>=dyabs)
			{
				if(pcnt){ dest[px] = pch/pcnt; pcnt=0; pch=0; }
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			//interal_linepaintchar_invert(px,py,buffer,linewidth);
			//dest[px] = src[py];
			pch+=src[py]; pcnt++;
		//	liqapp_log("spreadV %d,%d: %d,%d",dx,dy,px,py);
		}
		if(pcnt){ dest[px] = pch/pcnt; pcnt=0; pch=0; }
	}
 
 /*
	int IntPart = srclen / destlen;
	int FractPart = srclen % destlen;
	int E = 0;
	for(x=0;x<destlen;x++)
	{
		*dest++ = x % 255;//(*src);
		src+=IntPart;
		E+=FractPart;
		if(E>=destlen)
		{
			E-=destlen;
			src++;
		}
	}
 */
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
		
	
	#define layer(a,b,c) ( ((a)*100)+((b)*10)+(c) )
	if(isnear(c,l,grain))
	{
		// l==c
		if(isnear(c,r,grain))
			return layer(1,1,1);
		else if(c<r)
			return layer(1,1,0);
		else
			return layer(1,1,2);
	}
	else if(l<c)
	{
		// l<c
		if(isnear(c,r,grain))
			return layer(0,1,1);
		else if(c<r)
			return layer(0,1,0);
		else
			return layer(0,1,2);
	}
	else
	{
		// l>c
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




int upc_digitfrombitfield(int bitfield)
{
	
// Digit 	L Pattern 	R Pattern
//          64          64
//           32          32
//            16          16
//             8421        8421
// 0 		0001101 	1110010
// 1 		0011001 	1100110
// 2 		0010011 	1101100
// 3 		0111101 	1000010
// 4 		0100011 	1011100
// 5 		0110001 	1001110
// 6 		0101111 	1010000
// 7 		0111011 	1000100
// 8 		0110111 	1001000
// 9 		0001011 	1110100
int resl=-1;
int resr=-1;
	switch(bitfield)
	{
		
		case 5:   resl=99; resr=99;
				 break;
				
		case 114: resl=-1; resr=0;
				 break;
		case 102: resl=-1; resr=1;
				 break;
		case 108: resl=-1; resr=2;
				 break;
		case 66:  resl=-1; resr=3;
				 break;
		case 92:  resl=-1; resr=4;
				 break;
		case 78:  resl=-1; resr=5;
				 break;
		case 80:  resl=-1; resr=6;
				 break;
		case 68:  resl=-1; resr=7;
				 break;
		case 72:  resl=-1; resr=8;
				 break;
		case 116:  resl=-1; resr=9;
				 break;
		
		case 13: resl=0; resr=-1;
				 break;
		case 25: resl=1; resr=-1;
				 break;
		case 19: resl=2; resr=-1;
				 break;
		case 61: resl=3; resr=-1;
				 break;
		case 35: resl=4; resr=-1;
				 break;
		case 49: resl=5; resr=-1;
				 break;
		case 47: resl=6; resr=-1;
				 break;
		case 59: resl=7; resr=-1;
				 break;
		case 55: resl=8; resr=-1;
				 break;
		case 11: resl=9; resr=-1;
				 break;
		default:
				 resl=-1; resr=-1;
				 return 100;
	}
	if(resl>=0)
		return resl;
	else
		return -resr;



}





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
						mid= min + (max-min) * 0.9;
						src=indat;
						for(idx=0;idx<bitcount;idx++)
						{
							//if(idx>0)res=res+res;
							if(*src>=mid)
							{
								res=(res<<1) | 0;
								//*src=(idx==0)?'a':'b';
								//*src=255-*src;
							}
							else
							{
								res=(res<<1) | 1;
								//*src=(idx==0)?'X':'Y';
								//*src=255-src;
							}
							//if(*src<32)
							//	*src=32;
							src++;
						}
						return res;
					}
					int try(char *indat,int bitlength,char *kind)
					{
						int bitfield= tobinary(indat,bitlength);
						int upc=upc_digitfrombitfield(bitfield);
						//snprintf(indat,bitlength,"%d........",upc);
						//indat[ bitlength-1 ]='.';
						//while(strlen(indat)<bitlength){ indat[ strlen(indat)+1]='.'; }
						

						return upc;// bitfield;
						//app_log("%s = '%i'",kind,tobinary(indat,bitlength));
					}











void liqimage_mark_barcode(liqimage *self)
{
    
   // liqapp_log("barcode starting");

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
	
	
	foundcount=0;
			
		
	for(y=0;y<self->height;y+=1)
	{

		
		//##################################### step 1:  identify the tips of this line
		//                                               the sharp edges that our barcode is expected to have
		
		// reset tipcounter
		tipupto=0;
		
		
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
			//xsurface_drawpset_grey(self,x,y,255-bal);
			
			int ay = 127 * ((bal / 100));
			int au = 127 * ((bal % 100) / 10);
			int av = 127 * ((bal % 10));			
			//xsurface_drawpset_yuv(self,x,y,ay,au,av );
			xsurface_drawpset_yuv(self,x,y,pcy,128,128 );
			
			//xsurface_drawpset_grey(self,x,y,128);
			
			
			switch(bal)
			{
				case 012:	// hill
					//xsurface_drawpset_grey(self,x,y,255);
					
					//xsurface_drawpset_grey(self,x,y,0);
					liqimage_tipmark(self,x,pcy,0);
					//if(tipupto>2 && tips[tipupto-2].isguard)xsurface_drawpset_yuv(self,tips[tipupto-2].x,y,pcy,255,128 );
				//	xsurface_drawpset_yuv(self,x,y,pcy,0,255 );
					break;
				case 210:	// valley
					//xsurface_drawpset_grey(self,x,y,128);
					
					//xsurface_drawpset_grey(self,x,y,255);
					liqimage_tipmark(self,x,pcy,1);
					//if(tips[tipupto>2 && tipupto-2].isguard)xsurface_drawpset_yuv(self,tips[tipupto-2].x,y,pcy,0,128 );
				//	xsurface_drawpset_yuv(self,x,y,255-pcy,255,0 );
					break;
				case 111:	// flat
				//	liqimage_tipmark(self,x,pcy,2);
					//pcy=pcy;
					//int av = (ply + pcy + pry) / 3;
					//xsurface_drawpset_grey(self,x,y,pcy);
					//pcy=av;
					//xsurface_drawpset_yuv(self,x,y,pcy,255,255 );
					
					break;

				//case 010:	// slope climb "/"
				//	break;
				//case 212:	// slope down "\"
				//	break;
				default:
				//	xsurface_drawpset_yuv(self,x,y,pcy,128,128);
					break;
			}
		}
		liqimage_tipmark(self,self->width*2,0,0);
		liqimage_tipmark(self,self->width*2,0,0);
		
		//##################################### step 2:  now, identify the tip ranges
		//                                               101 ..... 101 ..... 101
		//                                               is ideal for us.
		int m,n,o;
		for(m=1;m<tipupto;m++)
		{
			if(tips[m].isguard)
			{
				for(n=m+2;n<tipupto;n++)
				{
					if(tips[n].isguard)
					{
						for(o=n+2;o<tipupto;o++)
						{
							int d1 =  tips[n].x-tips[m].x;
							int d2 =  tips[o].x-tips[n].x;
							if((tips[o].isguard) && (d1>80) && (isnear(d1,d2,4)) )
							{
								
								tips[m].ishot=1;
								tips[n].ishot=2;
								tips[o].ishot=3;
								
					//			liqapp_log("guard %3d %3d,%3d,%3d   %3d,%3d",y, tips[m].x,tips[n].x,tips[o].x, d1,d2    );
								// we have identified a triple of guard bars :)
								//for(x=tips[m-1].x-2;x<=tips[o+1].x+2;x++)
								{
								//	xsurface_drawpset_yuv(self,x,y,255,40,255 );
								}
								
								{
									int wid = ( (tips[o+1].x) - (tips[m-1].x) );
								}
								
								
								// 3 + (7*6) + 5 + (7*6) + 3
								
								
								
								// Paint on the items
								int wid = ( (tips[o+1].x) - (tips[m-1].x) );
								unsigned char *src = &self->data[self->offsets[0] + (y*self->pitches[0])  + (tips[m-1].x) ];
								unsigned char dest[95+1];
								spanstretch(src,wid+2,dest,95);
								dest[95]=0;
								
								
								//

								// ################################ convert to binary within local group
								//#define try(data,w,kind)
								int cgl=0;
								int cgc=0;
								int cgr=0;
								int codes[12];
								cgl=       try (&dest[0 ], 3, "bl ");
								
								codes[0] = try (&dest[3 ], 7, "l  ");
								codes[1] = try (&dest[10], 7, "l  ");
								
								codes[2] = try (&dest[17], 7, "l  ");
								codes[3] = try (&dest[24], 7, "l  ");
								
								codes[4] = try (&dest[31], 7, "l  ");
								codes[5] = try (&dest[38], 7, "l  ");
								
								cgc=       try (&dest[45], 5, "mid");
		
								codes[6] = try (&dest[50], 7, "r  ");
								codes[7] = try (&dest[57], 7, "r  ");
							
								codes[8] = try (&dest[64], 7, "r  ");
								codes[9] = try (&dest[71], 7, "r  ");
								
								codes[10] = try (&dest[78], 7,"r  ");
								codes[11] = try (&dest[85], 7,"r  ");
								
								cgr =       try (&dest[92], 3,"br ");
								
								//dest[95]='S';
								dest[95]=0;
								
								int codesx;
								
								
								//liqapp_log("b: %3d : %s, %3d : cg %d %d %d",y,dest, wid, cgl,cgc,cgr);// , tobinary(&dest[0],3));
								
								//if(cgl==99 && cgr==99) 
								{
								

								//	liqapp_log("b: %3d : %s, %d : %d %d %d %d %d %d",y,dest, wid, codes[0],codes[1],codes[2],codes[3],codes[4],codes[5]);
	
	
	
									foundcount++;
									if((foundcount)<(y))
									{
										int r=0;
	
										src = &self->data[self->offsets[0] + ((foundcount)*self->pitches[0])];
										for(r=0;r<95;r++)
										{
											*src++ = dest[r];
											*src++ = dest[r];
											*src++ = dest[r];
										}
	
									}								
								}
							}
						}
					}
				}
			}
		}


		for(m=0;m<tipupto;m++)
		{
			if(tips[m].ishot)
			{
				for(x=tips[m-1].x;x<=tips[m+1].x;x++)
				{
					xsurface_drawpset_yuv(self,x,y,0,tips[m].ishot * 80,40 );
				}
			}
			else
			{
				xsurface_drawpset_yuv(self,tips[m].x,y,tips[m].grey,128 + tips[m].tipcode * 64,240 );			
			}
		}
		
nextline:
		x=0;
		//foundcount=0;
	}
    
  //  liqapp_log("barcode complete");
}

#ifdef __cplusplus
}
#endif

