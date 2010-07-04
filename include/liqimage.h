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
 * Header for the liqimage bitmap class
 *
 */
#ifndef LIQIMAGE_H
#define LIQIMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct liqimage 	// you might recognise this as deriving from the XVImage structure :)
{
	unsigned int usagecount;
	int width;
	int height;
	int data_size;
	int num_planes;
	int *offsets;
	int *pitches;
	unsigned char *data;
	int dpix;
	int dpiy;
	void *XVImageSource;		// set if this image is initialized from an XVImage structure (ie uses other memory)

}
	liqimage;


//#########################################################################
//#########################################################################
//######################################################################### additional constructors
//#########################################################################
//#########################################################################
liqimage *  liqimage_cache_lookuponly(const char *filename,int maxw,int maxh,int allowalpha);
liqimage *  liqimage_cache_getfile(const char *filename,int maxw,int maxh,int allowalpha);
void        liqimage_cache_release(liqimage *self);

liqimage *  liqimage_newatsize(          int w,int h,int allowalpha);
liqimage *  liqimage_newfromfile(        const char *filename,int maxw,int maxh,int allowalpha);
liqimage *  liqimage_newfromfilejpeg(	const char *filename);
liqimage *	liqimage_newfromfilepng(		const char *filename,int allowalpha);

//#########################################################################
//#########################################################################
//######################################################################### standard object handling
//#########################################################################
//#########################################################################

liqimage * 	liqimage_new();
liqimage * 	liqimage_hold(liqimage *self);
void       	liqimage_release(liqimage *self);
void    	liqimage_free(liqimage *self);

//#########################################################################
//#########################################################################
//######################################################################### class methods
//#########################################################################
//#########################################################################

void 		liqimage_pagereset(				liqimage *self);
void 		liqimage_pagedefine(			liqimage *self,int w,int h,int dpix,int dpiy,int hasalpha);
void 		liqimage_pagedefinefromXVImage(	liqimage *self,void *XvImagePtr,int dpix,int dpiy);
int 		liqimage_pageloadjpeg(			liqimage *self, const char * filename,int maxw,int maxh);
int 		liqimage_pageloadpng(			liqimage *self, const char * filename,int maxw,int maxh,int allowalpha);
int         liqimage_pageloadpng_memstream( liqimage *self, const char * filename,char *srcdata, int srcsize,int maxw,int maxh,int allowalpha);

int 		liqimage_pagesavepng(liqimage *self, const char * filename);

liqimage *  liqimage_getthumbnail(          liqimage *original,int maxw,int maxh);

int 		liqimage_getwidth(				liqimage *self);
int 		liqimage_getheight(				liqimage *self);

//#########################################################################
//#########################################################################
//######################################################################### get hold of the filename of a thumbnail representation of specified image filename
//#########################################################################
//#########################################################################

					//char imagethumb[ FILENAME_MAX ];
					//if( liqimage_find_thumbnail_for(imagethumb,sizeof(imagethumb),fn) == 0 )
					//{
						// w00t!   (hello btw)
					//}
					// now returns the expected thumbnail whether or not it exists
					// return value 
					//  0 == exists and matches
					// -1 no datasize
					// -2 no data
					// -3 no file
int liqimage_find_thumbnail_for(char *resultbuffer,int resultsize,char *bigimagefilename);




//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################

// call a simple rotate method on the images
// the image needs to be defined properly 

int liqimage_rotate(liqimage *self,liqimage *original,int angle);

#ifdef __cplusplus
}
#endif

#endif
