


#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "liqbase.h"
#include "liqx11info.h"
#include "liqx11overlay.h"

// YUV overlay to RGB Conversion function
// to be used as a cover screen when disabling the overlay
// MASSIVE thanks to <KotCzarny> and <AStorm> for getting me started :)

//############################################################# 



XImage *liqimage_convert_to_ximage(liqimage *self, Display *dis, int screen)
{
	XImage *res = NULL;

	int     resdepth;
	Visual *resvis;
    int     reswidth;
    int     resheight;
    
	double  rRatio;
	double  gRatio;
	double  bRatio;
    
	int     outIndex = 0;
    int     x,y;
		
	resdepth = DefaultDepth( dis, screen);
	resvis   = DefaultVisual(dis, screen);
    
    reswidth = self->width;
    resheight= self->height;

	rRatio = resvis->red_mask / 255.0;
	gRatio = resvis->green_mask / 255.0;
	bRatio = resvis->blue_mask / 255.0;
    
    
    liqapp_log("mask R %06x, ratio %3.3f",resvis->red_mask,   rRatio);
    liqapp_log("mask G %06x, ratio %3.3f",resvis->green_mask, gRatio);
    liqapp_log("mask B %06x, ratio %3.3f",resvis->blue_mask,  bRatio);
    
    // 
    // http://wapedia.mobi/en/List_of_monochrome_and_RGB_palettes?t=3.
    
	// ####################################### convert YUV image into RGB
	liqcliprect *icr=liqcliprect_newfromimage(self);
    if(resdepth>=24)
    {
        liqapp_log("converting yuv to rgb32");
        u_int32_t *newBuf = malloc( 4 * reswidth * resheight );
        for(y=0;y<self->height;y++)
        {
            for(x=0;x<self->width;x++)
            {
                // this is v slow method but is quick and dirty enough to work, will improve later
                unsigned char iy,iu,iv;
                liqcliprect_drawpgetcolor(icr,x,y,&iy,&iu,&iv);
                if(iu==0)iu=128;
                if(iv==0)iv=128;
                // convert YUV -> RGB
                //http://msdn.microsoft.com/en-us/library/ms893078.aspx
                // yes, microsoft are useful :)
                inline unsigned char clip(int indat){ return (indat<0) ? 0 : ( (indat>255) ? 255 : indat) ;  }
                int           ic = (((int)iy)) - 16;
                int           id = (((int)iu)) - 128;
                int           ie = (((int)iv)) - 128;
                unsigned char outr = clip(( 298 * ic            + 409 * ie + 128) >> 8);
                unsigned char outg = clip(( 298 * ic - 100 * id - 208 * ie + 128) >> 8);
                unsigned char outb = clip(( 298 * ic + 516 * id            + 128) >> 8);

                //outr=outr*rRatio;
                //outg=outg*gRatio;
                //outb=outb*bRatio;
                
                outr&=resvis->red_mask;
                outg&=resvis->green_mask;
                outb&=resvis->blue_mask;
                
                newBuf[ outIndex++ ] = outr | outg | outb;
                
            }
        }
 		res = XCreateImage( dis, CopyFromParent, resdepth, ZPixmap, 0, (char *)newBuf,	reswidth, resheight,	32, 0 );
    }
    
    else
    if(resdepth>=15)
    {
        liqapp_log("converting yuv to rgb16");
        u_int16_t *newBuf = malloc( 2 * reswidth * resheight );
        for(y=0;y<self->height;y++)
        {
            for(x=0;x<self->width;x++)
            {
                // this is v slow method but is quick and dirty enough to work, will improve later
                unsigned char iy,iu,iv;
                liqcliprect_drawpgetcolor(icr,x,y,&iy,&iu,&iv);
                if(iu==0)iu=128;
                if(iv==0)iv=128;
                // convert YUV -> RGB
                //http://msdn.microsoft.com/en-us/library/ms893078.aspx
                // yes, microsoft are useful :)
                inline unsigned char clip(int indat){ return (indat<0) ? 0 : ( (indat>255) ? 255 : indat) ;  }
                int           ic = (((int)iy)) - 16;
                int           id = (((int)iu)) - 128;
                int           ie = (((int)iv)) - 128;
                unsigned char outr = clip(( 298 * ic            + 409 * ie + 128) >> 8);
                unsigned char outg = clip(( 298 * ic - 100 * id - 208 * ie + 128) >> 8);
                unsigned char outb = clip(( 298 * ic + 516 * id            + 128) >> 8);

                outr=outr >> 2;
                outg=outg >> 3;
                outb=outb >> 2;


                //outr=outr*rRatio;
                //outg=outg*gRatio;
                //outb=outb*bRatio;
                
                //outr&=resvis->red_mask;
                //outg&=resvis->green_mask;
                //outb&=resvis->blue_mask;
                
                newBuf[ outIndex++ ] = (outb << 11) | (outg << 6) | outr;
                
            }
        }
 		res = XCreateImage( dis, CopyFromParent, resdepth, ZPixmap, 0, (char *)newBuf,	reswidth, resheight,	16, 0 );        
    }
    else
    {
        // arghhh!
        liqapp_log("liqx11_cover invalid res depth");
        return NULL;
    }
	liqcliprect_release(icr);

	XInitImage(res);
	res->byte_order = LSBFirst;
	//res->byte_order = MSBFirst;
    
	res->bitmap_bit_order = LSBFirst;
	//res->bitmap_bit_order = MSBFirst;

	return res;
}	

