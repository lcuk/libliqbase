
#include <stdlib.h>
#include <string.h>


#include "liqcamera.h"
#include "liqapp.h"
#include "liqimage.h"
#include "liqcanvas.h"
#include "liqcliprect.h"

#include "liq_xsurface.h"

//const int grain=3;


// this is the "micro" variant
// i should make this a library selectable plugin soon


#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define ABS(x) (((x) >= 0) ? (x) : (-(x)))
#define SGN(x) (((x) >= 0) ? 1 : -1)

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

static inline int yuv_isgrey(unsigned char y,unsigned char u,unsigned char v)
{
//	if(isnear(u,128,64) && isnear(v,128,64))
	{
		// greyscale
		return 1;
	}
	// colored
	return 0;
}

//#####################################################################

struct span
{
	int l;
	int r;
	int w;
	int py;
	int pu;
	int pv;
};


//#####################################################################
static void spanstretch_dumb(unsigned char *src,int srclen,unsigned char *dest,int destlen)
{
	// stretch srclen characters of src all the way along dest so they occupy destlen space
	// quick integrater scaling
	int ppx;
	for(ppx=0;ppx<destlen;ppx++)
	{
		int xx = ppx * (srclen-1) / (destlen-1);
		//*dest++ = src[ppx];//xx];
		*dest++ = src[xx];
	}
	return;
}

static void spanstretch_urg(unsigned char *src,int srclen,unsigned char *dest,int destlen,unsigned char mid,unsigned char min,unsigned char max)
{
	// stretch srclen characters of src all the way along dest so they occupy destlen space
	// faster general
	int ppx;
	const int grain=10;
	for(ppx=0;ppx<destlen;ppx++)
	{
		int xx = ppx * (srclen-1) / (destlen-1);
		//*dest++ = src[ppx];//xx];
		
		int l=src[xx-1];
		int c=src[xx];
		int r=src[xx+1];
		int pix=0;

		if(isnear(c,l,grain))
		{
			// l==c
			if(isnear(c,r,grain))
				pix = c>=mid?max:min;
			else if(c<r)
				pix = c>=mid?max:min;
			else
				pix = c>=mid?max:min;
		}
		else if(l<c)
		{
			// l<c
			if(isnear(c,r,grain))
				pix = c>=mid?max:min;
			else if(c<r)
				pix = c>=mid?max:min;
			else
				pix = max;
		}
		else
		{
			// l>c
			if(isnear(c,r,grain))
				pix = c>=mid?max:min;
			else if(c<r)
				pix = min;
			else
				pix = c>=mid?max:min;
		}
		//pix = (l+c+r)/3;
		pix=c>=mid?max:min;
		//pix=c;

		
		*dest++ = pix;//src[xx];
	}
	return;
}



unsigned int bucket[96]={0};


static void spanstretch(unsigned char *src,int srclen,unsigned char *dest,int destlen,unsigned char mid,unsigned char min,unsigned char max)
{
	// stretch srclen characters of src all the way along dest so they occupy destlen space

	// urg, this is a heavy function involving rational fractions and downsampling
	
	if(destlen!=95)
	{
		liqapp_log("going dumb %d",destlen);
		spanstretch_dumb(src,srclen,dest,destlen);
		return;
	}
	
	
	int bucketfactor=0;
	int bucketrpt=0;

	//liqapp_log("stage1");

		// get the factor require (number of repeats per pixel)
		int n;
		for(n=1;n<=95;n++)
		{
			int xn = srclen*n;
			if(xn % 95 == 0)
			{
				//liqapp_log("factor %3d/95 :: %3d == %3d/%3d",x,n,xn,95*n );
				bucketfactor = n;
				bucketrpt = xn/95;
				break;
			}
		}
		//liqapp_log("stage2 fac=%d rpt=%d",bucketfactor,bucketrpt);

		
		int p;
		// clear the buckets
		for(p=0;p<95;p++) bucket[p]=0;
		// now fill the buckets with values from source data
		
		unsigned int *bp = bucket;
		unsigned char *sp = src;
		for(p=1;p<1+(srclen*bucketfactor);p++)
		{
			//liqapp_log("bitspread %5d, %3d to 95,  bfac=%d,brep=%d   bp[%d] += sp[%d]",p,srclen,bucketfactor,bucketrpt,  bp-bucket,  sp-src );
			//int c=*sp>=mid?max:min;;
			*bp += *sp;
			if((p % bucketrpt   )==0)bp++;
			if((p % bucketfactor)==0)sp++;
		}
		//liqapp_log("stage3 %d",bucketrpt);
		// now finally fill in the destination data
		//int ahem = mid * bucketrpt;
		for(p=0;p<95;p++)
		{
			//if(bucket[p]>ahem)
			//	dest[p] = max;
			//else
			//	dest[p] = min;
			dest[p] = bucket[p] / bucketrpt;
		}
		//liqapp_log("stage4");
}


//#####################################################################

int triplet_getbalance(int l,int c,int r,int grain)
{
	// return integer indicating the balance of a triplet
	// given the 3 values, assess whether they are a straight line or a slope
	// the index returned indicates the motion
	
	// [ ] [ ] [ ]
	//  o   o   o
	//  l   c   r   // 111: a flat section, left is ~= centre is ~= right (amount of fuzzy grain allowed)
	//  o   o   o


	//  o   o   o
	//  o   c   o 	// 012: a hill - left is less than centre which is greater than r
	//  l   o   r	
	

	//  l   o   r
	//  o   c   o 	// 210: a valley - left is greater than centre which is less than r
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

int test_triplet_getbalance()
{
	// this is intended to check functionality of the getbalance routine
	// it also serves to demonstrate the expected api :)
	// idea: to scan source folders on building and try to encourage their formation
			const int grain=3;
			int bal = triplet_getbalance(40,50,60,grain);
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


//########################################################################################################
//########################################################################################################
//######################################################################################################## ean decode
//########################################################################################################
//########################################################################################################


//#####################################################################

#define HEX__(n) 0x##n##LU

/* 8-bit conversion function */
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

/* *** user macros *** */

/* for upto 8-bit binary constants */
#define B8(d) ((unsigned char)B8__(HEX__(d)))


int ean_digitfrombitfield(char LGR,unsigned char bitfield)
{
//Digit 	L-code 	G-code 	R-code
//0 	0001101 	0100111 	1110010
//1 	0011001 	0110011 	1100110
//2 	0010011 	0011011 	1101100
//3 	0111101 	0100001 	1000010
//4 	0100011 	0011101 	1011100
//5 	0110001 	0111001 	1001110
//6 	0101111 	0000101 	1010000
//7 	0111011 	0010001 	1000100
//8 	0110111 	0001001 	1001000
//9 	0001011 	0010111 	1110100

//Digit 	L-code 	G-code 	R-code
#define ean_try(bitfield,LGR,digit,L,G,R) \
	{ \
		if(LGR=='L') if(bitfield==B8(L)) return digit; \
		if(LGR=='G') if(bitfield==B8(G)) return digit; \
		if(LGR=='R') if(bitfield==B8(R)) return digit; \
	}

	// L4	   G1      G0      L1      G3      G3
	// 0100011,0110011,0100111,0011001,0100001,0100001


// |0| 0|000|| 0||00|| 0|00||| 00||00| 00|||00|||000|0|0|0|00||00||000||||00||0||00||0|00|||0|0000|0|0|, 117 : 4 1 -1 1 -1 -1
// GGG444444411111110000000

	ean_try(bitfield,LGR,0, 	0001101, 	0100111, 	1110010)
	ean_try(bitfield,LGR,1, 	0011001, 	0110011, 	1100110)
	ean_try(bitfield,LGR,2, 	0010011, 	0011011, 	1101100)
	ean_try(bitfield,LGR,3, 	0111101, 	0100001, 	1000010)
	ean_try(bitfield,LGR,4, 	0100011, 	0011101, 	1011100)
	ean_try(bitfield,LGR,5, 	0110001, 	0111001, 	1001110)
	ean_try(bitfield,LGR,6, 	0101111, 	0000101, 	1010000)
	ean_try(bitfield,LGR,7, 	0111011, 	0010001, 	1000100)
	ean_try(bitfield,LGR,8, 	0110111, 	0001001, 	1001000)
	ean_try(bitfield,LGR,9, 	0001011, 	0010111, 	1110100)
	return -1;
}


int ean_decode(int* bitfields12, char *resbuf14)
{
	char *resbuf=resbuf14;
	// using the array of 12 digits 0..11
	// decode the data and fill in 13digit+NULL resbuf
	int  digits12[12];		// store the digits returned
	char digitpar[6];		// store the parity results for the first 6
	int idx;
	int dig;
	
	resbuf[0]='X';		// blank space for the digit parity
	resbuf[1]=0;
	
	for(idx=0;idx<6;idx++)
	{
		dig = ean_digitfrombitfield('L',bitfields12[idx]);
		if(dig>=0)
		{
			// valid using L
			resbuf[1+idx]='0'+dig;  resbuf[1+idx+1]=0;
			digitpar[idx]='L';
		}
		else
		{
			dig = ean_digitfrombitfield('G',bitfields12[idx]);
			if(dig>=0)
			{
				// valid using G
				resbuf[1+idx]='0'+dig;  resbuf[1+idx+1]=0;
				digitpar[idx]='G';
			}
			else
			{
				// INVALID!!
				//liqapp_log("ean_decode failed idx=%d",idx);
				return 0;
			}
		}
	}
	for(idx=6;idx<12;idx++)
	{
		dig = ean_digitfrombitfield('R',bitfields12[idx]);
		if(dig>=0)
		{
			// valid using R
			resbuf[1+idx]='0'+dig;  resbuf[1+idx+1]=0;
			//digitpar[idx]='R';
		}
		else
		{
			// INVALID!!
			//liqapp_log("ean_decode failed idx=%d",idx);
			return 0;
		}
	}
	//liqapp_log("ean_decode got to end! ... %d %d %d %d %d %d ..... %d %d %d %d %d %d ... %s", digits12[0],digits12[1],digits12[2],digits12[3],digits12[4],digits12[5],digits12[6],digits12[7],digits12[8],digits12[9],digits12[10],digits12[11],resbuf );

	// now, work out the checksum!
}



//########################################################################################################
//########################################################################################################
//######################################################################################################## upc decode
//########################################################################################################
//########################################################################################################



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


//#####################################################################

					int getmid(unsigned char *indat,int bitcount,unsigned char *outmin,unsigned char *outmax)
					{
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
						//mid= min + (max-min) * 0.5;
						mid= min + (max-min) * 0.5;
						*outmin=min;
						*outmax=max;
						return mid;
					}
					

					int tobinary(unsigned char *indat,int bitcount)
					{
						int res=0;
						int idx;
						//int mid=tobinary_mid;// getmid(indat,bitcount);
						unsigned char min=0,max=0;
						int mid=getmid(indat,bitcount,&min,&max);
						
						mid= min + (max-min) * 0.6;
						char *src;
						src=indat;
						for(idx=0;idx<bitcount;idx++)
						{
							//if(idx>0)res=res+res;
							if(*src>=mid)
							{
								res=(res<<1) | 0;
								//*src=(idx==0)?'0':'o';
								*src=255;//255-*src;
							}
							else
							{
								res=(res<<1) | 1;
								//*src=(idx==0)?'X':'x';
								*src=0;//255-src;
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
						return bitfield;
						//int upc=upc_digitfrombitfield(bitfield);
						//snprintf(indat,bitlength,"%d........",upc);
						//indat[ bitlength-1 ]='.';
						//while(strlen(indat)<bitlength){ indat[ strlen(indat)+1]='.'; }
						

						//return upc;// bitfield;
						//app_log("%s = '%i'",kind,tobinary(indat,bitlength));
					}







//#####################################################################


void hmm_removeme()
{
int dstw=5;
int srcw=7;
int x,y;
float frac=(float)srcw/(float)dstw;
	liqapp_log("float pixels each %3.3f (should ALWAYS be >=1.0)",frac);
	
float pxf=0.0;
int   pxi=0;
	
// 142
// 144
// ish

// 95

	for(x=95;x<400;x++)
	{
		// for each X, find the multiplier required
		int n;
		for(n=1;n<100;n++)
		{
			int xn = x*n;
			if(xn % 95 == 0)
			{
				liqapp_log("factor %3d/95 :: %3d rpt %3d == %3d/%3d",x,n,xn/95,xn,95*n );
				break;
			}
		}
		if(n==100) liqapp_log("factor %d/95 :: %3d == NOT FOUND", x,n );
	}
	
	for(x=1;x<20;x++)
	{
		// for each X, find the multiplier required
		int n;
		for(n=1;n<100;n++)
		{
			int xn = x*n;
			if(xn % 5 == 0)
			{
				liqapp_log("75factor %3d/5 :: %3d rpt %d == %3d/%3d", x,n,xn/5,xn,5*n );
				break;
			}
		}
		if(n==100) liqapp_log("75factor %d/95 :: %3d == NOT FOUND", x,n );
	}
	
	
	exit(0);


}














static int xadj=-20;
static int xdir=1;


void liqimage_mark_barcode(liqimage *self)
{
    
   // liqapp_log("barcode starting");



	// put markers on barcode entries
	// ignore anytihng that is not a typical barcode
	// operate quickly and do not interupt user
	

	xadj+=xdir;
	if(ABS(xadj) >= 20){ xdir=-xdir; }  // xadj=-15;

	
	int liqimage_get_white_strip(unsigned char *src,int x,unsigned char midgrey,int ab)
	{

		int ssx=x;
		unsigned char sspy=0;

		int sex=x;
		unsigned char sepy=0;
		
	//	int midgrey = ((int)mingrey + (int)maxgrey) / 2;

		sspy = src[ssx];
		if(sspy>midgrey)
		{
			for(sex=ssx+1;sex<self->width;sex++)
			{
				sepy = src[sex];
				if(isnear(sspy,sepy,5) && sepy>midgrey) 
				{
					// they are close, continue
				}
				else
				{
					// not close - this span ends..
					break;
				}
			}
		}
		//if(sex-ssx>8)	liqapp_log("%d x=%3d w=%3d py=%3d",ab,x,sex-ssx,sspy);
		return sex-ssx;
	}
		
		
	int foundcount=0;	
	
	unsigned char py=0,pu=0,pv=0;

	int y;
	for(y=0;y<self->height;y++)
	{
		//##################################### step 1:  identify the flats and tips of this line
		//                                               the sharp edges that our barcode is expected to have
		int x;
		int sc;
		
		
		//##################################### find variance within line
		unsigned char *src = &self->data[self->offsets[0] + (y*self->pitches[0]) ];
		unsigned char min;
		unsigned char max;
		unsigned char midgrey;
		min=0;
		max=0;
		midgrey=0;
	    midgrey=getmid(src,self->width,&min,&max);

		//midgrey=(min+max)/2;
		midgrey=min+(max-min)*0.95;
		//liqapp_log("grey %3d, %3d,%3d,%3d",y,min,max,midgrey);
		
		//##################################### find variance within line
		
		
		for(x=0;x<self->width;x++)
		{
			// could optimize the hell out of this by removing the pget
			// would be useful for more advanced algorithms
			
			int sc = liqimage_get_white_strip(src,x,midgrey,1);
			if(sc>8)
			{
				
				int ex;
				int ec;
				for(ex=x+sc+90;ex<self->width;ex++)
				{
					int ec=liqimage_get_white_strip(src,ex,midgrey,2);
					if(ec>8 && isnear(sc,ec,8))
					{
						// w00t!
						//xsurface_drawpset_yuv(self,x,y,128,255,0);
						//xsurface_drawpset_yuv(self,ex,y,128,0,255);
						
						unsigned char bcmin=0,bcmax=0;
						
						// xadj hits better on 1
						int bcwid=(ex-1)-(x+sc+1) +1;//xadj;//xadj;// 1;
						unsigned char *bcsrc = &src[x+sc + 1  ];//1];			// +1
						unsigned char buf[95+1];
						int bcgrey=getmid(bcsrc,bcwid,&bcmin,&bcmax);
						bcgrey=bcmin+(bcmax-bcmin)*0.6;
						spanstretch(bcsrc,bcwid, buf,95,bcgrey,bcmin,bcmax);
						
						//liqapp_log("huh?");
						//tobinary(buf,95);
						//tobinary_mid = getmid(buf,95);
						
						// VALID...?
					
						int cgl=0;
						int cgc=0;
						int cgr=0;
						int codes[12];
						
						
						cgl=       try (&buf[0 ], 3, "bl ");
						
						codes[0] = try (&buf[3 ], 7, "l  ");
						codes[1] = try (&buf[10], 7, "l  ");
						
						codes[2] = try (&buf[17], 7, "l  ");
						codes[3] = try (&buf[24], 7, "l  ");
						
						codes[4] = try (&buf[31], 7, "l  ");
						codes[5] = try (&buf[38], 7, "l  ");
						
						cgc=       try (&buf[45], 5, "mid");

						codes[6] = try (&buf[50], 7, "r  ");
						codes[7] = try (&buf[57], 7, "r  ");
					
						codes[8] = try (&buf[64], 7, "r  ");
						codes[9] = try (&buf[71], 7, "r  ");
						
						codes[10] = try (&buf[78], 7,"r  ");
						codes[11] = try (&buf[85], 7,"r  ");
						
						cgr =       try (&buf[92], 3,"br ");
						
						
						

						if(buf[  0]==0 && buf[  1]==255 && buf[  2]==0  &&
						   buf[ 92]==0 && buf[ 93]==255 && buf[ 94]==0    
						   )
						{

								char eanres[14]={0};
								ean_decode(codes,eanres);


								if(foundcount<y)
								{
									unsigned char *dst = &self->data[self->offsets[0] + (foundcount * self->pitches[0]) ];
									//liqapp_log("hola1");
									spanstretch_dumb(buf,95, dst,95);
									dst[95]=foundcount*8;
									dst[96]=foundcount*8;
									dst[97]=foundcount*8;
									dst[98]=foundcount*8;

								}


								
								int xx;
								for(xx=0;xx<95;xx++)
									buf[xx] = buf[xx]==0 ? '|' : '0';
									
								#define hf(xx) {buf[xx]=(buf[xx]=='|' ? 'I' : '8');}
								hf(0);
								
								hf(3);
								hf(10);
								hf(17);
								hf(24);
								hf(31);
								hf(38);
								
								hf(45);
								
								hf(50);
								hf(57);
								hf(64);
								hf(71);
								hf(78);
								hf(85);
								
								hf(92);
								
								buf[95]=0;



							
								liqapp_log("b: %3d (%3d) : %s, %d : %s",y,xadj,&buf[0], bcwid, eanres);
	
											
						
							if(foundcount<y)
							{
								int xx;
								for(xx=x+sc+1;xx<ex;xx++)
								{
								//	xsurface_drawpget_yuv(self,xx  ,y,&py,&pu,&pv);
								//	xsurface_drawpset_yuv(self,xx-x,foundcount,py,pu,pv);
								//	src[xx]=255-src[xx];
								}
								//unsigned char *src = &self->data[self->offsets[0] + (y          * self->pitches[0])  + (x+sc) ];
								
								
	
									
								//unsigned char *dst = &self->data[self->offsets[0] + (foundcount * self->pitches[0]) ];
								//liqapp_log("hola1");
								//spanstretch(bcsrc,bcwid, dst,95,bcgrey,bcmin,bcmax);
								//liqapp_log("hola2");
								//tobinary(dst,95);
								
								//dst[95]=foundcount*8;
								//dst[96]=foundcount*8;
								//dst[97]=foundcount*8;
								//dst[98]=foundcount*8;

								
								
								
								
							}
								for(xx=0;xx<bcwid;xx++)
									bcsrc[xx]=255-bcsrc[xx];
								
								x=ex+ec-sc;
							foundcount++;
							//x=ex+ec;
							break;
						}
						// invalid, so lets skip the next n pixels..
						ex+=ec;
					}
				}
				x+=sc;
				
				
				
			}
		}

	}

	//	exit(0);
  //  liqapp_log("barcode complete");
}


