
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include "md5.h"
#include "liqapp.h"
#include "liqimage.h"
#include "liqcanvas.h"
#include "liqcliprect.h"

#include  <stdio.h>
#include  <stdlib.h>
#include  <dlfcn.h>
#include <elf.h>

#define TRUE -1


					//char imagethumb[ FILENAME_MAX ];
					//if( liqimage_find_thumbnail_for(imagethumb,sizeof(imagethumb),fn) == 0 )
					//{
						// w00t!   (hello btw)
					//}

//#####################################################################
//##################################################################### try to lookup a thumbnail, return 0 if filled in, -1 otherwise
//#####################################################################

int liqimage_find_thumbnail_for(char *resultbuffer,int resultsize,char *bigimagefilename)
{
	// turbo mode!
	// no thumbnailing :o
	//snprintf(resultbuffer,resultsize,"%s", bigimagefilename);
	//return 0;


	char imageuri[FILENAME_MAX+10];
	
	snprintf(imageuri,sizeof(imageuri),"file://%s",bigimagefilename);
	
	char  imageuri_md5[32+1]={0};

		struct cvs_MD5Context context;
		unsigned char checksum[16];
		cvs_MD5Init (&context);
		cvs_MD5Update (&context, (unsigned char *)imageuri, strlen (imageuri));
		cvs_MD5Final (checksum, &context);
		int i;
		for (i = 0; i < 16; i++)
		{
			snprintf (&imageuri_md5[i*2],3, "%02x", (unsigned int) checksum[i]);
			//printf ("%02x  %i", (unsigned int) checksum[i],i);
		}
		imageuri_md5[32]=0;
		
	//liqapp_log("liqimage_find_thumbnail_for: checking for '%s' %s",imageuri_md5,imageuri);
	
	char thumbpath[ FILENAME_MAX ];
	
	snprintf(thumbpath,sizeof(thumbpath),"%s/.thumbnails/cropped/%s.jpeg",app.homepath,imageuri_md5);
	
	
	if(liqapp_fileexists(thumbpath))
	{
		// thumb exists
		snprintf(resultbuffer,resultsize,"%s", thumbpath);
		return 0;
		
	}
	// thumb does not exist
	// lets not waste time (argggg)
	//snprintf(resultbuffer,resultsize,"%s", bigimagefilename);
	return -1;

}

