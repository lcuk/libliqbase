

#include <stdlib.h>
#include <stdio.h>

#include <memory.h>


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>


#include <png.h>
#include <setjmp.h>
#include <jpeglib.h>



#include "liqapp.h"
#include "liqimage.h"
#include "liqcanvas.h"
#include "liqcliprect.h"



//####################################################################
//####################################################################
//####################################################################


static struct
{
	char *key;
	liqimage *data;	
}
			cachestack[4096];	// 20090619_132600 lcuk : mmm todo call back here
static int  cachemax=4096-1;
static int  cacheused=0;


static int liqimage_cache_clean_unused(int maxremove)
{
	// cleans all images not in use (upto maxremove count)
	int f=0;
	int removedcount=0;
	for(f=0;f<cacheused;f++)
	{
		if(cachestack[f].data->usagecount==1)
		{
			//liqapp_log("releasing %s",cachestack[f].key);
			// not used by anyone other than cache anymore..
			// remove it from the stack
			// and if we are not the final item, move the final item to here
			free(cachestack[f].key);
			liqimage_release(cachestack[f].data);
			
			cachestack[f].key =NULL;				
			cachestack[f].data=NULL;				
			//liqapp_log("released, adjusting stack");
			
			int g;
			for(g=f+1;g<cacheused;g++)
			{
				//cachestack[g-1] = cachestack[g];
				cachestack[g-1].key =cachestack[g].key;				
				cachestack[g-1].data=cachestack[g].data;				
			}
		/*	
			cachestack[f].key =NULL;				
			cachestack[f].data=NULL;				
			if(f<(cacheused-1))
			{
				cachestack[f].key  = cachestack[cacheused-1].key;
				cachestack[f].data = cachestack[cacheused-1].data;
				f--;	// move backwards slightly to ensure we rescan the moved item
			}
		*/
			//liqapp_log("released, done");
			if(f<(cacheused-1))
				f--;
				
			cacheused--;
			removedcount++;
			if(removedcount>=maxremove) break;
		}
	}
	//liqapp_log("removed %i",removedcount);
	return removedcount;
}


//####################################################################
//####################################################################
//####################################################################




liqimage *liqimage_cache_lookuponly(char *filename,int maxw,int maxh,int allowalpha)
{
	//
	liqimage *self=NULL;
	char cachekey[256];
	int f;
	snprintf(cachekey,256,"image:%s,%i,%i,%i",filename,maxw,maxh,allowalpha);
	//liqapp_log( "image cache seeking %s", cachekey );
	if(cacheused>=cachemax)
	{
		//liqapp_log( "image cache cleaning %s", cachekey );
		if(liqimage_cache_clean_unused(8)==0)
		{
			// all image slots actively in use
			// error in app or just very varied
	        liqapp_log( "Image cache full %s", cachekey );
			return NULL;
		}
	}
	//for(f=0;f<cacheused;f++)
	for(f=cacheused-1;f>=0;f--)
	{
		if(strcmp(cachestack[f].key,cachekey)==0)
		{
			// no differences..
			//liqapp_log( "image cache matched %s %i", cachekey ,cachestack[f].data->usagecount);
			self = cachestack[f].data;
			//self->usagecount++;
			liqimage_hold(self);
			//liqapp_log("found %s", cachekey );
			return self;
		}
		// whilst I am searching, perhaps I should be moving 0 rated items to the bottom of the stack
	}
	return NULL;
}



liqimage *liqimage_cache_getfile(char *filename,int maxw,int maxh,int allowalpha)
{
	//
	liqimage *self=NULL;
	char cachekey[256];
	int f;
	snprintf(cachekey,256,"image:%s,%i,%i,%i",filename,maxw,maxh,allowalpha);
	//liqapp_log( "image cache seeking %s", cachekey );
	if(cacheused>=cachemax)
	{
		//liqapp_log( "image cache cleaning %s", cachekey );
		if(liqimage_cache_clean_unused(8)==0)
		{
			// all image slots actively in use
			// error in app or just very varied
	        liqapp_log( "Image cache full %s", cachekey );
			return NULL;
		}
	}
	//for(f=0;f<cacheused;f++)
	for(f=cacheused-1;f>=0;f--)
	{
		if(strcmp(cachestack[f].key,cachekey)==0)
		{
			// no differences..
			//liqapp_log( "image cache matched %s %i", cachekey ,cachestack[f].data->usagecount);
			self = cachestack[f].data;
			//self->usagecount++;
			liqimage_hold(self);
			//liqapp_log("found %s", cachekey );
			return self;
		}
		// whilst I am searching, perhaps I should be moving 0 rated items to the bottom of the stack
	}
		//liqapp_log("not found %s", cachekey );
	// not yet in the cache
	liqapp_log( "image cache creating %s", cachekey );


	if(liqapp_filesize(filename)<=0)
	{
		// invalid file
        liqapp_log( "liqimage invalid file (<=0 size) %s", cachekey );
		return NULL;	
	}

//int err=0;
	self=liqimage_newfromfile(filename,maxw,maxh,allowalpha);
	if(!self)
	{
        liqapp_log( "liqimage couldn't create %s", cachekey );
		return NULL;		
	}

	// simply add our own lock onto this image handle :)
	
	liqimage_hold(self);
	
	//self->usagecount=1;

	//liqapp_log( "TTF cache inserting %s", cachekey );

	f=cacheused;
	cachestack[f].key  = strdup(cachekey);
	cachestack[f].data = self;
	cacheused++;
	////liqapp_log( "TTF cache completed %s", cachekey );
	return self;
}





void liqimage_cache_release(liqimage *self)
{
	// shouldnt actually do anything with this yet
	// we have an extra lock on the data
	// i hope to god the user doesnt do extra holds/releases, but i suppose thats the same with anything
	liqimage_release(self);
}




//####################################################################
//####################################################################
//####################################################################




liqimage *  liqimage_newatsize(          int w,int h,int allowalpha)
{
		//app.infologgingenabled=0;
		liqimage *self=liqimage_new();
		if(!self)
		{
			liqapp_log("liqimage_newatsize error init");
			return NULL;
		}

		liqimage_pagedefine(self, w,h,liqcanvas_getdpix(),liqcanvas_getdpiy(), allowalpha );

		return self;	
}


liqimage *liqimage_newfromfile(char *filename,int maxw,int maxh,int allowalpha)
{
	
	char *ext = liqapp_filename_walktoextension(filename);
	if(!ext || !*ext)
	{
		liqapp_log("liqimage_newfromfile invalid filename '%s'",filename);
		return NULL;
	}
	
	
	
	
	if(
		strcasecmp(ext,"gif")==0  ||
		strcasecmp(ext,"bmp")==0
	  )
	{

		liqapp_log("liqimage_newfromfile no image parser available '%s'",filename);
		return NULL;
	}
		
		
	
	liqimage *self=liqimage_new();
	if(!self)
	{
		liqapp_log("liqimage_newfromfile error init '%s'",filename);
		return NULL;
	}
	
	
	
	
	if(
		strcasecmp(ext,"jpg")==0  ||
		strcasecmp(ext,"jpeg")==0
	  )
	{
		if(liqimage_pageloadjpeg(self,filename,maxw,maxh)!=0)
		{
			liqapp_log("liqimage_newfromfile error loading jpeg '%s'",filename);
			liqimage_free(self);
			return NULL;
		}
		return self;
	}
	
	
	
	if(
		strcasecmp(ext,"png")==0
	  )
	{
		if(liqimage_pageloadpng(self,filename,0,0,allowalpha)!=0)
		{
			liqapp_log("liqimage_newfromfile error loading png '%s'",filename);
			liqimage_free(self);
			return NULL;
		}
		return self;
	}
	
	

	
	// try and sniff?
	
	FILE * fp=fopen(filename,"r");
	if(fp)
	{
		char buffer[12];
		memset(buffer,0,sizeof(buffer));
		
		int result = fread(buffer,1,sizeof(buffer),fp);
		
		fclose(fp);
		if (result != sizeof(buffer))
		{
			liqapp_log("Reading error",stderr);
		}
		
		if(strncmp(&buffer[6],"JFIF",4)==0)
		{
			// its a jpeg
			if(liqimage_pageloadjpeg(self,filename,maxw,maxh)!=0)
			{
				liqapp_log("liqimage_newfromfile error loading jpeg '%s'",filename);
				liqimage_free(self);
				return NULL;
			}
			return self;
		}
		if(strncmp(&buffer[1],"PNG",3)==0)
		{
			// its a png
			if(liqimage_pageloadpng(self,filename,0,0,allowalpha)!=0)
			{
				liqapp_log("liqimage_newfromfile error loading png '%s'",filename);
				liqimage_free(self);
				return NULL;
			}
			return self;
		}		

	
	}
	
	
	{
		liqapp_log("liqimage_newfromfile invalid filename '%s'",filename);
		liqimage_free(self);
		return NULL;
	}
	
}




liqimage *liqimage_newfromfilejpeg(char *filename)
{

		//app.infologgingenabled=0;
		liqimage *self=liqimage_new();
		if(liqimage_pageloadjpeg(self,filename,0,0)!=0)
		{
			// failed..
			liqimage_free(self);
			//app.infologgingenabled=1;
			return NULL;
		}

		// ok
		return self;
		//app.infologgingenabled=1;
}


liqimage *liqimage_newfromfilepng(char *filename,int allowalpha)
{

		//app.infologgingenabled=0;
		liqimage *self=liqimage_new();
		if(liqimage_pageloadpng(self,filename,0,0,allowalpha)!=0)
		{
			// failed..
			liqimage_free(self);
			//app.infologgingenabled=1;
			return NULL;
		}

		// ok
		return self;
		//app.infologgingenabled=1;
}


//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################



liqimage *liqimage_new()
{
	liqimage *self = (liqimage *)calloc(sizeof(liqimage),1);
	if(self==NULL) {  liqapp_errorandfail(-1, "liqimage creation failed" ); return NULL; }
	// NULL everything
	//memset((char *)self,0,sizeof(liqimage));
	self->usagecount=1;
	
	return self;
}

liqimage * liqimage_hold(liqimage *self)
{
	// use this to hold onto an object which someone else created
	//liqapp_log( "liqimage hold  (%i,%i) %i", self->width,self->height,self->usagecount );
	if(self)self->usagecount++;
	return self;
}

void liqimage_release(liqimage *self)
{
	// use this when you are finished with an object
	if(!self) return;
	//liqapp_log( "liqimage release  (%i,%i) %i", self->width,self->height,self->usagecount );
	
	self->usagecount--;
	if(!self->usagecount) liqimage_free(self);
}

void liqimage_free(liqimage *self)
{
	//liqapp_log("liqimage free");
	liqimage_pagereset(self);
	free(self);
}



//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################


void liqimage_pagereset(liqimage *self)
{
	liqapp_log("liqimage pagereset");
	
	
	
	
	
	if(self->XVImageSource)
	{
		// data comes from xv, only release the offsets and pitches :)
		if(self->offsets)free(self->offsets);
		if(self->pitches)free(self->pitches);
	}
	else
	{
		// release everything
		if(self->offsets)free(self->offsets);
		if(self->pitches)free(self->pitches);
		if(self->data)   free(self->data);		
	}
	// 20090624_020851 lcuk : GOTCHA!  you were being blanked to 0 every time this was called and not retaining the usagecount
int uc=self->usagecount;
	memset((char *)self,0,sizeof(liqimage));
	self->usagecount=uc;
}

void liqimage_pagedefine(liqimage *self,int w,int h,int dpix,int dpiy,int hasalpha)	// result plane count: 3=YUV, 4=YUVA
{
	//if(picbuff_ready) return;
	
	liqapp_log("liqimage pagedefine(%i,%i) dpi(%i,%i) hasalpha=%i",w,h,dpix,dpiy,hasalpha);
	
	liqimage_pagereset(self);
	
int num_planes;

	if(hasalpha)
		num_planes=4;
	else
		num_planes=3;

int *picoffsets=malloc(sizeof(int)*num_planes);
	if(!picoffsets)
	{
		liqapp_log("image: page defined could not alloc offsets");
		return;
	}

		

	picoffsets[0] = 0;
	picoffsets[1] = w * h;
	picoffsets[2] = picoffsets[1] + ((w/2) * (h/2));
	if(hasalpha)
		picoffsets[3] = picoffsets[1] + ((w/2) * (h/2)) * 2;
	
int *picpitches=malloc(sizeof(int)*num_planes);

	if(!picpitches)
	{
		liqapp_log("image: page defined could not alloc pitches");
		return;
	}
	
	picpitches[0] = w;
	picpitches[1] = w/2;
	picpitches[2] = w/2;
	if(hasalpha)
		picpitches[3] = w;
	
	self->width      = w;
	self->height     = h;
	self->data_size  = (w * h) + ( 2 * ((w/2) * (h/2)) ) + (hasalpha ? (w * h) : 0);
	self->num_planes = num_planes;
	self->offsets    = picoffsets;
	self->pitches    = picpitches;
	self->data       = malloc(self->data_size);
	if(!self->data)
	{
		liqapp_log("image: page defined could not alloc plane data");
		return;
	}
	
	memset((char *)self->data,0,self->data_size);

	
	self->XVImageSource = NULL;
	self->dpix = dpix;
	self->dpiy = dpiy;

	//liqapp_log("liqimage pagedefine end");
	
	
}

void liqimage_pagedefinefromXVImage(liqimage *self,void *XvImagePtr,int dpix,int dpiy)
{
XvImage *XvImage=XvImagePtr;
	//if(picbuff_ready) return;
	
	liqapp_log("liqimage pagedefinefromxv");
	
	liqimage_pagereset(self);
	
	liqapp_log("liqimage pagedefinefromxv2");
	
int w=XvImage->width;
int h=XvImage->height;
int num_planes=XvImage->num_planes;
	if(num_planes>3)num_planes=3;
	liqapp_log("liqimage pagedefinefromxv3 planes=%i  hmm=%i",num_planes,sizeof(int)*num_planes);

int *picoffsets=malloc(sizeof(int)*num_planes);
	liqapp_log("liqimage pagedefinefromxv3.5");
int *picpitches=malloc(sizeof(int)*num_planes);
int i;

	liqapp_log("liqimage pagedefinefromxv4");

	for(i=0;i<num_planes;i++)
	{	
		picoffsets[i] = XvImage->offsets[i];
		picpitches[i] = XvImage->pitches[i];
	}


	liqapp_log("liqimage pagedefinefromxv5");


	self->width      = w;
	self->height     = h;
	self->data_size  = XvImage->data_size;
	self->num_planes = num_planes;
	self->offsets    = picoffsets;
	self->pitches    = picpitches;
	self->data       = (unsigned char *)XvImage->data;
	self->XVImageSource = XvImagePtr;
	self->dpix = dpix;
	self->dpiy = dpiy;
	


	liqapp_log("liqimage pagedefinefromxv end");
	
}


/******************** JPEG DECOMPRESSION SAMPLE INTERFACE *******************/
//have a look here for example of saving: http://local.wasp.uwa.edu.au/~pbourke/libraries/jpeg.c
// and this for optimizations: http://mail.kde.org/pipermail/digikam-devel/2007-February/010596.html

//http://download.gna.org/pdbv/demo_html/demo_2.0.10/package/libjpeg62-dev_6b-9.html
//https://stage.maemo.org/svn/maemo/projects/haf/trunk/osso-af-utils/src/fb-progress.c
//http://www.koders.com/c/fid52761211B5999D23A81D29A394002B18BA57AE58.aspx
// has png inside...


struct liqimage_jpeg_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};
typedef struct liqimage_jpeg_error_mgr * liqimage_jpeg_error_ptr;

void liqimage_jpeg_error_exit(j_common_ptr cinfo)
{
	liqimage_jpeg_error_ptr myerr = (liqimage_jpeg_error_ptr) cinfo->err;
	//(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}




int liqimage_pageloadjpeg(liqimage *self,char * filename,int maxw,int maxh)
{
	// why did this take so much code for such a straight forward job?
	// will I have to go through similar for other image formats?
	// todo: add new image import formats as required.
struct jpeg_decompress_struct 	cinfo;
struct liqimage_jpeg_error_mgr  jerr;
FILE 							*infile;
unsigned char 							*buffer;
int 							row_stride;
	liqapp_log("jpeg.opening '%s'",filename);
	if ((infile = fopen(filename, "rb")) == NULL)
	{
		liqapp_log("jpeg.open failed %s", filename);
		return -1;
	}
	

	
	liqapp_log("jpeg.init jpeglib");
	
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = liqimage_jpeg_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		liqapp_log("jpeg.failed '%s'", filename);
		return -1;
	}	
	 	
	jpeg_create_decompress(&cinfo);
	
	

	
	liqapp_log("jpeg.init src");
	jpeg_stdio_src(&cinfo, infile);
	liqapp_log("jpeg.read header");
	int JHR = jpeg_read_header(&cinfo, TRUE);
	if(JHR != JPEG_HEADER_OK )
	{
		// jpeg error
		liqapp_log("jpeg.read header failed '%s' JHR %i", filename,JHR);
		return -1;		
	}
	
	switch (cinfo.jpeg_color_space)
	{
		case JCS_YCbCr:
			liqapp_log("jpeg.YUV colorspace detected."); 
			break;
		case JCS_GRAYSCALE:
			liqapp_log("jpeg.Grayscale colorspace detected."); 
			break;
		default:
			liqapp_log("jpeg.Unsupported colorspace detected.");
			return -1;
			break;
	}
	
	// and this for optimizations: http://mail.kde.org/pipermail/digikam-devel/2007-February/010596.html
    // libjpeg supports 1/1, 1/2, 1/4, 1/8
    int scale=1;
	if(maxw && maxh)
	{
		while(scale<8 && (cinfo.image_width>maxw || cinfo.image_height>maxh)) // maximumSize*scale*2<=imgSize)
		{
			scale*=2;
		}
	}
    //if(scale>8) scale=8;
    cinfo.scale_num=1;
    cinfo.scale_denom=scale;
	
	liqapp_log("jpeg.header original image %i,%i",cinfo.image_width,cinfo.image_height);

	
	
	liqapp_log("jpeg.forcing decompress colorspace to yuv");
	cinfo.out_color_space  = JCS_YCbCr;
	liqapp_log("jpeg.start decompress");
	jpeg_start_decompress(&cinfo);
	liqapp_log("jpeg.header output image %i,%i",cinfo.output_width,cinfo.output_height);
	if (cinfo.output_components != 3 && cinfo.out_color_space == JCS_YCbCr)
	{
		liqapp_log("jpeg.expecting 3 planes for YUV");
		return -1;
	}
	if (cinfo.output_components != 1 && cinfo.out_color_space == JCS_GRAYSCALE)
	{
		liqapp_log("jpeg.expecting 1 plane for Greyscale");
		return -1;
	}
	/*
	if(maxw && maxh)
	{
		while(cinfo.output_width>maxw)
		{
			cinfo.output_width/=2;
			cinfo.output_height/=2;
		}
		while(cinfo.output_height>maxh)
		{
			cinfo.output_width/=2;
			cinfo.output_height/=2;
		}
	}
	*/
	liqimage_pagedefine(self, cinfo.output_width, cinfo.output_height, liqcanvas_getdpix(),liqcanvas_getdpiy(),  0 );
	row_stride = cinfo.output_width * cinfo.output_components;
	int x=0;
	int y=0;
	buffer = malloc( row_stride * sizeof(char) );		// allocate a line block
	int i;
	for (i = 0; i < cinfo.output_components; i++)
	{
		int sh= cinfo.comp_info[i].h_samp_factor;
		int sv= cinfo.comp_info[i].v_samp_factor;
		liqapp_log("samp factor %i h=%i v=%i",i,sh,sv);
	}
	liqapp_log("jpeg max_v_samp_factor= %i, DCTSIZE = %i    *= %i",cinfo.max_v_samp_factor, DCTSIZE,cinfo.max_v_samp_factor * DCTSIZE);
	liqapp_log("jpeg.reading data, at %i of %i",cinfo.output_scanline,cinfo.output_height);

	while (cinfo.output_scanline < cinfo.output_height)
	{
	    jpeg_read_scanlines(&cinfo, (JSAMPARRAY) (void *)(&buffer), 1);
		if(cinfo.output_components==1)
		{
			
			for(x=0;x<cinfo.output_width;x++)
			{
				int dstoff;
				dstoff=self->offsets[0] + (y * (self->pitches[0]) + x);
				self->data[ dstoff ] = buffer[x*cinfo.output_components];
			}
		}
		else
		{
			for(x=0;x<cinfo.output_width;x++)
			{
				int dstoff;
				dstoff=self->offsets[0] + (y * (self->pitches[0]) + x);
				self->data[ dstoff ] = buffer[x*cinfo.output_components];
				if( ((y&1)) && ((x&1)) )
				{
					// handle the color components (at half resolution in this case)
					int xx=x>>1;
					int yy=y>>1;
					dstoff=self->offsets[1] + (yy * (self->pitches[1]) + xx);
					self->data[ dstoff ] = buffer[x*cinfo.output_components+2];
					dstoff=self->offsets[2] + (yy * (self->pitches[2]) + xx);
					self->data[ dstoff ] = buffer[x*cinfo.output_components+1];
				}
			}			
		}
		y++;
		if(y>self->height)break;
	}
	liqapp_log("jpeg.cleanup");
	free(buffer);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
	liqapp_log("jpeg.complete");
	return 0;
}








//#if 0




/******************** PNG DECOMPRESSION SAMPLE INTERFACE *******************/

// this does not work yet, and will require additional tinkering
//http://www.libpng.org/pub/png/libpng-manual.txt



//https://stage.maemo.org/svn/maemo/projects/haf/trunk/osso-af-utils/src/fb-progress.c


// a further example is here http://www.zarb.org/~gc/html/libpng.html


// more inside enlightenment :)
// http://trac.enlightenment.org/e/browser/trunk/PROTO/enesim/examples/image.c?rev=35582

//static image_info_t *decompress_png(const char *filename)
int liqimage_pageloadpng(liqimage *self,char * filename,int maxw,int maxh,int allowalpha)
{
	return liqimage_pageloadpng_memstream(self,filename,NULL,0,maxw,maxh,allowalpha);
}


struct liqimage_png_membuffer
{
	int offset;
	int size;
	char *data;
};

// use the png_membuffer to supply data as expected to png library
static void png_read_data(png_structp png_ptr, png_bytep area, png_size_t size)
{
	png_byte *n_data = (png_byte *)area;
	
	
	struct liqimage_png_membuffer *src;
	src = (struct liqimage_png_membuffer *)png_get_io_ptr(png_ptr);
	
	
	
	//char mem[5];
	//mem[0]=src->data[src->offset+0];
	//mem[1]=src->data[src->offset+1];
	//mem[2]=src->data[src->offset+2];
	//mem[3]=src->data[src->offset+3];
	//mem[4]=0;
	//liqapp_log("png.read %i bytes wanted cursor (%i/%i) :: '%s' %02x,%02x,%02x,%02x",size,src->offset,src->size,mem,mem[0],mem[1],mem[2],mem[3]);
	//SDL_RWread(src, area, size, 1);
	if(src->offset+size >= src->size)
	{
		png_error(png_ptr, "png_read_data error, input past end");
		return;
	}
	memcpy((char *)n_data,src->data+src->offset,size);
	src->offset+=size;	
}


int liqimage_pageloadpng_memstream(liqimage *self,char * filename,char *srcdata, int srcsize,int maxw,int maxh,int allowalpha)
{
	// 20090715_224851 lcuk : srcdata and srcsize are memory array versions
	// 20090715_224901 lcuk : pass NULL and 0 as well as a valid filename to use standard procedure
	// 20090715_224901 lcuk : passing valid parameters in src*, and NULL filename results in using the memory for the file instead :)
	struct liqimage_png_membuffer src;
	src.offset=0;
	src.size=srcsize;
	src.data=srcdata;
	
	
	FILE *fp=NULL;
	
	if(*filename)
	{
		liqapp_log("png.opening '%s'",filename);
		fp = fopen(filename, "rb");
		if (!fp)
		{
			liqapp_log("png.open failed %s", filename);
			return -1;
		}
	}
	else
	{
		liqapp_log("png.memory stream XYZ");
	}

	unsigned char  header[8];
	
		int ttt;
	if(*filename)
	{
		ttt=fread(header, 1, 8, fp);
		if(!ttt)
		{
			if(fp)fclose(fp);
			return -2;			
		}
	}
	else
	{
		if(srcsize<8)
		{
			return -2;			
		}
		// just copy the memory :)
		memcpy(header,srcdata,8);
		
		//src.offset+=8;	// hmmm
	}
	
		int is_png;
		is_png = !png_sig_cmp(header, 0, 8);
		if (!is_png)
		{
			liqapp_log("png.not an image");
			if(fp)fclose(fp);
			return -2;
		}
		
		
		
		


	//liqapp_log("png.reading struct");
		
	
	png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr)
		{
			liqapp_log("png.png_ptr fail");
			if(fp)fclose(fp);
			return -3;
		}
		
	//liqapp_log("png.setting jmpbuf");
		
    if (setjmp (png_jmpbuf (png_ptr)))
	{
		liqapp_log("png setjmp called, must have an error");
		
		{
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			if(fp)fclose(fp);
			return -3;
		}
	}
	
	
	

	//liqapp_log("png.creating info_struct");
	
		
	png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			liqapp_log("png.info_ptr fail");
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			if(fp)fclose(fp);
			return -4;
		}
		
	//liqapp_log("png.creating end info");
		
		
	png_infop end_info = png_create_info_struct(png_ptr);
		if (!end_info)
		{
			liqapp_log("png.end_info fail");
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			if(fp)fclose(fp);
			return -5;
		}
		
		
	if(*filename)
	{
	//	liqapp_log("png.init file io");
		png_init_io(png_ptr, fp);
	}
	else
	{
	//	liqapp_log("png.assign read function");
		png_set_read_fn(png_ptr, (char *)&src, png_read_data);
		
	}
	
	
	if(*filename)
	{
	
	//	liqapp_log("png.setting sigbytes (tell lib we skipped 8 bytes already)");
	
	
		png_set_sig_bytes(png_ptr, 8);
		
		// leave this for memory access
	}	
		
		
		
	//liqapp_log("png.read info ");
		
	png_uint_32 	wd=0;
	png_uint_32 	ht=0;
	int 			bit_depth=0;
	int 			color_type=0;

		png_read_info(png_ptr, info_ptr);
		
		
	//liqapp_log("png.get header");
		
		
		png_get_IHDR(png_ptr, info_ptr, &wd, &ht, &bit_depth, &color_type, NULL, NULL, NULL);


	typedef struct image_info_t
	{
		int wd;
		int ht;
	}
		image_info_t;

	image_info_t 	image;	
		image.wd = wd;
		image.ht = ht;
	
	
	//liqapp_log("png.checking color");
	
	int hasalpha=0;
	int isgray=0;
		if (color_type & PNG_COLOR_MASK_ALPHA)
			hasalpha=1;
			
		// manual forced disable..
		if(!allowalpha)
			hasalpha=0;
			
		// alpha channel not supported. At least not yet...   well - we might do
		//if(hasalpha)
			//png_set_strip_alpha(png_ptr);
	
		if(color_type & PNG_COLOR_MASK_COLOR)
		{
			liqapp_log("color............");
			
		}
		else
		{
			liqapp_log("gray............");
			isgray=1;
		}
		
		
	liqapp_log("png.checking rgb expansion");
		
	
		/* transfer image to RGB if not already */
		if (color_type != PNG_COLOR_TYPE_RGB)
		{
			png_set_expand(png_ptr); 
		}

		/* we are pleased with 8 bit per pixel */
		if (bit_depth == 16) 
			png_set_strip_16(png_ptr);
			
			
		//// we want RGBA
		//png_set_filler (png_ptr, 0, PNG_FILLER_AFTER);
	
	
	//liqapp_log("png.updating info");
	
		png_read_update_info(png_ptr, info_ptr);
		
		
	//liqapp_log("png.getting rowbytes");
		
	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	////todo work out if this is required, i guess ill find out soon enough..
	// done, its necessary, at least for alpha images
	
	//if(hasalpha)
	// infact, now it doesnt matter, i can fix it up with an autosize outside here
	{
		// ensure we are an even number of units wide (required for alpha reduced viewing)
		if(image.wd & 1 )image.wd--;
		if(image.ht & 1 )image.ht--;
	}
	
	
	liqapp_log("png.liqimage pagedefine");

	
	liqimage_pagedefine(self, image.wd, image.ht,liqcanvas_getdpix(),liqcanvas_getdpiy(), hasalpha );
	
	image.wd = wd;
	image.ht = ht;

	
	
	liqapp_log("png: rowbytes=%i",rowbytes);
	
	
	unsigned char * image_data = malloc(rowbytes * image.ht);
		if(!image_data)
		{
			liqapp_log("png.image_data malloc fail");
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
			fclose(fp);
			return -6;
		}
	
	png_bytepp row_pointers = malloc(image.ht * sizeof(png_bytep));
		if(!row_pointers)
		{
			liqapp_log("png.row_pointers malloc fail");
			free(image_data);
			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
			return -7;
		}
	liqapp_log("png reading data");
	int i=0;
	//int j=0;	
		for (i = 0;  i < image.ht; i++)
			row_pointers[i] = image_data + i * rowbytes;
	
		// read the whole image ?
		// is this right ?
		// ahh well it must be
			
		png_read_image(png_ptr, row_pointers);
		
		
		// now, the image_data and rot_buffers memory contains valid RGB png data.
		
		
		// quickly try and load greyscale from it :)
				
	// recurse and grab data

	//int cy00=0;			
	int x=0;
	int y=0;
	int bytesperpixel = rowbytes / image.wd;// hasalpha ? 4 : 3;
		for(y=0;y<self->height;y++)
		{
			for(x=0;x<self->width;x++)
			{
				int dstoff;
				int srcpix = (y * rowbytes) + (x * bytesperpixel);

				//dstoff=self->offsets[0] + (y * (self->pitches[0]) + x);
				//self->data[ dstoff ] = image_data[srcpix];
				//if(hasalpha)
				//{
				//	dstoff=self->offsets[3] + (y * (self->pitches[3]) + x);
				//	self->data[ dstoff ] = image_data[srcpix + 3];
				//}
				
				unsigned char ir;
				unsigned char ig;
				unsigned char ib;
				unsigned char ia;
				
				int cy;
				int cv;
				int cu;

	
				if(isgray==0)
				{
					
//#ifdef USEMAEMO
					
					ir = image_data[ srcpix    ];
					ig = image_data[ srcpix +1 ];
					ib = image_data[ srcpix +2 ];
//#else

//					ir = image_data[ srcpix +2 ];
//					ig = image_data[ srcpix +1 ];
//					ib = image_data[ srcpix    ];

//#endif
					
					//http://msdn.microsoft.com/en-us/library/ms893078.aspx
					// yes, microsoft are useful :)
					
					cy = ( (  66 * ir + 129 * ig +  25 * ib + 128) >> 8) +  16;
					cv = ( ( -38 * ir -  74 * ig + 112 * ib + 128) >> 8) + 128;
					cu = ( ( 112 * ir -  94 * ig -  18 * ib + 128) >> 8) + 128;
	
	
	
					//cy=255;
					//cu=x;
					//cv=y;
	
	
	
					dstoff=self->offsets[0] + (y * (self->pitches[0]) + x);
					self->data[ dstoff ] = cy;//image_data[srcpix];
	
					dstoff=self->offsets[1] + ((y>>1) * (self->pitches[1]) + (x>>1));
					self->data[ dstoff ] = cu;//image_data[srcpix+1];
	
					dstoff=self->offsets[2] + ((y>>1) * (self->pitches[2]) + (x>>1));
					self->data[ dstoff ] = cv;//image_data[srcpix+2];
	
					if(hasalpha)
					{
						ia = image_data[ srcpix +3 ];
						dstoff=self->offsets[3] + (y * (self->pitches[3]) + x);
						
						
						//ia=x*y;
						
						
						self->data[ dstoff ] =      ia;//(cy==cy00) ? 0 : 255; // image_data[(y * rowbytes) + ((x * bytesperpixel)+3)];
					}
	
				}
				else
				{
	
					cy = image_data[ srcpix    ];
	
	
					dstoff=self->offsets[0] + (y * (self->pitches[0]) + x);
					self->data[ dstoff ] = cy;
	
					dstoff=self->offsets[1] + ((y>>1) * (self->pitches[1]) + (x>>1));
					self->data[ dstoff ] = 128;
	
					dstoff=self->offsets[2] + ((y>>1) * (self->pitches[2]) + (x>>1));
					self->data[ dstoff ] = 128;
	
					if(hasalpha)
					{
						ia = image_data[ srcpix +1 ];
						dstoff=self->offsets[3] + (y * (self->pitches[3]) + x);
						self->data[ dstoff ] =ia;//(cy==cy00) ? 0 : 255; // image_data[(y * rowbytes) + ((x * bytesperpixel)+3)];
					}
				}
			}
		}
		

		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		free(row_pointers);
		free(image_data);
		if(fp)fclose(fp);

		liqapp_log("png finished");

		return 0;
}


//#endif // 0
		
/*	
	image.pixel_buffer = malloc(image.wd * image.ht *  sizeof(uint16_t));
	if (!image.pixel_buffer) {
		fprintf(stderr, "Not enought memory\n");
		free(image_data);
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	      
	int k=0;
	for (j = 0; j < image.ht; j++) {
		uint8_t *picture = row_pointers[j];
		uint16_t red, green, blue;
		for (i = 0; i < image.wd; i++) {
			red = (*picture++ >> 3);
			green = (*picture++ >> 2);
			blue = (*picture++ >> 3);
			
			*(image.pixel_buffer + k) = red << 11 | green << 5 | blue;
			k++;
		}
	}
 */






























































int liqimage_pagesavepng(liqimage *self,char * filename)
{
	//
	liqapp_log("png save called %i,%i: to '%s'",self->width,self->height,filename);
	FILE *outf = fopen(filename,"wb");
	if(!outf)
	{
		return liqapp_warnandcontinue(-1,"png Couldn't open file for writing");
	}
	
	
	//####################### configure the target features
	
	liqapp_log("png configuring dest type");
	// i have 1 to 4 planes
	// 1==greyscale
	// 2=grey+alpha
	// 3=YUV
	// 4=YUV+alpha
	
	int png_color_type=PNG_COLOR_TYPE_RGB;
    int png_depth=8;
	int png_bytesperpixel=4;

	switch(self->num_planes)
	{
		case 1:
			png_depth = 8;
			png_color_type = PNG_COLOR_TYPE_GRAY;
			png_bytesperpixel=1;
			break;
		
		
		case 2:
			png_depth = 8;
			png_color_type = PNG_COLOR_TYPE_GRAY;
			png_bytesperpixel=4;
			break;
		
		case 3:
			png_depth = 8;
			png_color_type = PNG_COLOR_TYPE_RGB;
			png_bytesperpixel=3;
			break;
		
		case 4:
			png_depth = 8;
			png_color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			png_bytesperpixel=4;
			break;
	}
	
	
	

    //####################### allocate and intialise RGB buffer
	
	
	
	
	// ####################################### allocate buffer
	png_struct *png_ptr;
    png_info *info_ptr;
	
    png_byte **rowoffsets;
	int x,y;
	liqapp_log("png allocating rgb buffer");
	char *rgbabuffer = malloc(png_bytesperpixel * self->width * self->height);		// alloc rgb buffer RGBA
    if (!rgbabuffer)
	{
		fclose(outf);
		return liqapp_warnandcontinue(-1,"png Couldn't allocate rgbabuffer");
    }
	// ####################################### convert YUV image into RGB
	liqapp_log("png converting yuv to rgb");
	unsigned char iy,iu,iv;
	liqcliprect *icr=liqcliprect_newfromimage(self);
	for(y=0;y<self->height;y++)
	{
		for(x=0;x<self->width;x++)
		{
			// this is v slow method but is quick and dirty enough to work, will improve later
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
			int           outoff = png_bytesperpixel * (y * self->width + x);
			rgbabuffer[outoff  ] = outr;
			rgbabuffer[outoff+1] = outg;
			rgbabuffer[outoff+2] = outb;
			
			if(png_color_type == PNG_COLOR_TYPE_RGB_ALPHA)
			{
				// for now just store this, its wrong, but will have to do
				rgbabuffer[outoff+3] = 255;
			}
			
		}
	}
	liqcliprect_release(icr);
	// ####################################### now get on with the hard work
	liqapp_log("png allocating row buffer");

    rowoffsets = malloc(sizeof(png_byte*) * self->height);
    if (!rowoffsets)
	{
		fclose(outf);
		return liqapp_warnandcontinue(-1,"png Couldn't allocate rows buffer");
    }
	
	liqapp_log("png filling row buffer");

    for (y=0; y<self->height; y++)
	{
		rowoffsets[y] = (png_byte *) &rgbabuffer[y * self->width * png_bytesperpixel];
	}

	
	liqapp_log("png allocating write struct");
	

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
	{
		free(rgbabuffer);
		free(rowoffsets);
		fclose(outf);
		return liqapp_warnandcontinue(-2,"png Couldn't allocate png write struct");
    }
	
	liqapp_log("png allocating info struct");

    info_ptr = png_create_info_struct(png_ptr);
    if (!png_ptr)
	{
		png_destroy_write_struct (&png_ptr, &info_ptr);
		free(rgbabuffer);
		free(rowoffsets);
		fclose(outf);
		return liqapp_warnandcontinue(-2,"png Couldn't allocate png info struct");
    }
	
	liqapp_log("png initializing jmpbuf");

    if (setjmp (png_jmpbuf (png_ptr)))
	{
		liqapp_log("png setjmp called, must have an error");
		
		png_destroy_write_struct (&png_ptr, &info_ptr);
		free(rgbabuffer);
		free(rowoffsets);
		fclose(outf);
		return liqapp_warnandcontinue(-2,"png setjmp was raised");
    }
	
	
	
	
	
	//####################### prepare the row_writer function
	
//	void png_rowwriter(png_structp png, png_bytep data, png_size_t size)
//	{
//		// do the writing
//		FILE *fp;
//		fp = png_get_io_ptr (png);
//		if (fwrite (data, 1, size, fp) != size)
//			png_error(png, "Write Error");
//	}
//   png_set_write_fn(png_ptr, closure, png_rowwriter, NULL);
	
	liqapp_log("png setting phyx %i,%i",
			
				  (int)(   ((float)(self->width * self->dpix))  * (100.0 / 2.54)   ),
				  (int) (  ((float)(self->height * self->dpiy)) * (100.0 / 2.54)   )
			
			
			);
	
	
	png_set_pHYs(png_ptr, info_ptr,
				  (int)(   (self->width * self->dpix)  * (100.0 / 2.54)   ),
				  (int) (  (self->height * self->dpiy) * (100.0 / 2.54)   ),
					PNG_RESOLUTION_METER);
	
	

	
	//####################### link to io stream
	
	liqapp_log("png linking to io stream");
	png_init_io( png_ptr, outf );

	//####################### declare the header
	
	liqapp_log("png setting header");
    png_set_IHDR (png_ptr, info_ptr,
		  self->width,
		  self->height,
		  png_depth,
		  png_color_type,
		  PNG_INTERLACE_NONE,
		  PNG_COMPRESSION_TYPE_DEFAULT,
		  PNG_FILTER_TYPE_DEFAULT);
	
	
	
	
	
	

 	//####################### set the time
	
	
	liqapp_log("png setting timestamp");
	png_time pt;
		png_convert_from_time_t (&pt, time (NULL));
	    png_set_tIME (png_ptr, info_ptr, &pt);

	
	//####################### create background if required
	
	
    if (png_color_type == PNG_COLOR_TYPE_RGB || png_color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		liqapp_log("png organising background");
		png_color_16 backcolor;
			//backcolor.red = 0xff;		// white
			//backcolor.blue = 0xff;
			//backcolor.green = 0xff;
			
			backcolor.red = 0x00;		// black :D
			backcolor.blue = 0x00;
			backcolor.green = 0x00;

			png_set_bKGD (png_ptr, info_ptr, &backcolor);	
			png_set_bgr(png_ptr);
	}



	
    if (png_color_type == PNG_COLOR_TYPE_RGB)
	{
		liqapp_log("png setting filler");
		png_set_filler (png_ptr, 0, PNG_FILLER_AFTER);
	}
	
	
	
	
	liqapp_log("png setting rows");
	
	png_set_rows( png_ptr, info_ptr, rowoffsets );

	liqapp_log("png writing png");

	png_write_png( png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL );

	liqapp_log("png cleaning up");

    //png_write_info (png_ptr, info_ptr);
    //png_write_image (png_ptr, rowoffsets);
    //png_write_end (png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    free(rowoffsets);
	free(rgbabuffer);
	fclose(outf);
    return 0;
}



// found this ..
//http://www.sfr-fresh.com/unix/privat/transparency-0.1.151-src.tar.gz:a/transparency-0.1.151-src/transparency.cpp
  //437         if (m_res_x > 1e-8 && m_res_y > 1e-8) {
  //438             png_set_pHYs (
  //439                 png_ptr, info_ptr,
  //440                 (int)floor(m_res_x * (100.0 / 2.54)), (int)floor (m_res_y * (100.0 / 2.54)),
  //441                 PNG_RESOLUTION_METER);
  //442         }

	
	
	
	


