
// liqcamerafaceface : face camera interface
// slightly cleaned up duplicate of liqcameraface
// these should really be instances of the same class :$

#ifdef __cplusplus
extern "C" {
#endif


// maemo5
#define VIDEO_SRC  "v4l2camsrc"
#define VIDEO_SINK "xvimagesink"


#include <stdlib.h>
#include <string.h>
#include <gst/gst.h>
#include <pthread.h>
#include <sched.h>

#include "liqapp.h"
#include "liqcanvas.h"
#include "liqcameraface.h"
#include "liq_xsurface.h"			// include available workhorse functions

static pthread_mutex_t image_push_lock = PTHREAD_MUTEX_INITIALIZER;


GstElement *CAMFACEpipeline=NULL;
int 		CAMFACEW=0;
int 		CAMFACEH=0;
int 		CAMFACEFPS=0;
liqimage *	CAMFACEdestimage=NULL;
void *      CAMFACEtag;
void 		(*CAMFACEUpdateCallback)(void *);
int         CAMFACE_isface=0;

#define mute(pix) (pix)

static int image_push(char *data)
{
	if(!CAMFACEdestimage) return -1;
	// todo: ensure n800 camera flip is handled, need to read the sensor bit and organise accordingly
	// todo: ensure mirroring option is accounted for
	pthread_mutex_lock(&image_push_lock);


		
	// ok a 32bit long contains  UYVY
	unsigned long * UYVY = (unsigned long *)data;
	unsigned char *dy= &CAMFACEdestimage->data[ CAMFACEdestimage->offsets[0] ];
	unsigned char *du= &CAMFACEdestimage->data[ CAMFACEdestimage->offsets[1] ];
	unsigned char *dv= &CAMFACEdestimage->data[ CAMFACEdestimage->offsets[2] ];
	int ux=0;
	int uy=0;
	int zl = (CAMFACEW*CAMFACEH)/2;
	
	unsigned char *ddy= dy+(CAMFACEW  )-1;
	unsigned char *ddu= du+(CAMFACEW/2)-1;
	unsigned char *ddv= dv+(CAMFACEW/2)-1;
		//	liqapp_sleep(50);
	
	int CAMFACEWd2=CAMFACEW/2;
	int CAMFACEHd2=CAMFACEH/2;
	
	// landscape portrait difference here
	//ROTATEPATCHPOINT
	if(canvas.rotation_angle==0)
	{
		// landscape first
		//(disable this during portrait test)
		do
		{
			// read data for 2 source pixels
			unsigned long p= *UYVY++;
			if(CAMFACE_isface==0)
			{
				// Primary Grey channel
				*dy++ = (p & (255<<8 )) >> 8;
				*dy++ = (p & (255<<24)) >> 24;
				if(!(uy & 1))
				{
					// even lines only, 1/2 resolution
					*du++ = mute((p & (255<<16)) >> 16);
					*dv++ = mute((p & (255    )));
				}
				ux+=2;
			}
			else
			{
				*ddy-- = (p & (255<<8 )) >> 8;
				*ddy-- = (p & (255<<24)) >> 24;
				
				if(!(uy & 1))
				{
					// even lines only, 1/2 resolution
					*ddu-- = mute((p & (255<<16)) >> 16);
					*ddv-- = mute((p & (255    )));
				}
				ux+=2;
			}
			if(ux>=CAMFACEW){ ux=0;uy++;   ddy=dy+(CAMFACEW*(uy+1))-1; ddu=du+(CAMFACEWd2*((uy>>1)+1))-1; ddv=dv+(CAMFACEWd2*((uy>>1)+1))-1;        }
			if(uy>=CAMFACEH) break;
		}
		while(--zl);
	}
	else
	{
		// portrait
		do
		{
			
			// read data for 2 source pixels
			unsigned long p= *UYVY++;
			
		//	liqapp_log("cam %3d,%3d, %08lx",ux,uy,p);
			
			// Primary Grey channels to 2 adjacent greys
			dy[ ( (ux  ) ) * CAMFACEdestimage->pitches[0] + ( (CAMFACEH-1)-(uy  ) ) ] = (p & (255<<8 )) >>  8;
			dy[ ( (ux+1) ) * CAMFACEdestimage->pitches[0] + ( (CAMFACEH-1)-(uy  ) ) ] = (p & (255<<24)) >> 24;
			if(!(uy & 1))
			{
				// even lines only, 1/2 resolution
		//		du[ ((CAMFACEHd2-1)-((ux>>1)+1) ) * CAMFACEdestimage->pitches[1] + (uy>>1) ] = mute((p & (255<<16)) >> 16);
		//		dv[ ((CAMFACEHd2-1)-((ux>>1)+1) ) * CAMFACEdestimage->pitches[2] + (uy>>1) ] = mute((p & (255    ))      );
	
	
				du[ ((ux>>1)) * CAMFACEdestimage->pitches[1] + ((CAMFACEHd2-1)-(uy>>1)) ] = mute((p & (255<<16)) >> 16);
				dv[ ((ux>>1)) * CAMFACEdestimage->pitches[2] + ((CAMFACEHd2-1)-(uy>>1)) ] = mute((p & (255    ))      );
			}
			ux+=2;
			// portrait mode, the camera itself was opened with (rows of CAMFACEH) * CAMFACEW instead of the other way round
			if(ux>=CAMFACEH){ ux=0;uy++;     UYVY+=(CAMFACEW-CAMFACEH)>>1;   UYVY=(unsigned long *)&data[ (CAMFACEW * 2) * uy ]; }
			if(uy>=CAMFACEH) break;
		}
		while(--zl);	
	}

	// tell our host that we updated (up to him what he does with the info)
	if(CAMFACEUpdateCallback)
		(*CAMFACEUpdateCallback)(CAMFACEtag);
	pthread_mutex_unlock(&image_push_lock);
	return 0;
}


static gboolean buffer_probe_callback( GstElement *image_sink, GstBuffer *buffer, GstPad *pad, int *stuff)
{
	//liqapp_log("liqcameraface: callback!");
	char *data_photo = (char *) GST_BUFFER_DATA(buffer);
	image_push(data_photo);	
	return TRUE;
}



int liqcameraface_start(int argCAMFACEW,int argCAMFACEH,int argCAMFACEFPS,liqimage * argCAMFACEdestimage,void (*argCAMFACEUpdateCallback)(void*),void *argCAMFACEtag )
{
	if(CAMFACEpipeline)
	{
		// camera pipeline already in use..
		return -1;
	}
	liqapp_log("liqcameraface: starting %i,%i %ifps",argCAMFACEW,argCAMFACEH,argCAMFACEFPS);
	CAMFACEtag=argCAMFACEtag;
	CAMFACEpipeline=NULL;
	CAMFACEW=argCAMFACEW;
	CAMFACEH=argCAMFACEH;
	CAMFACEFPS=argCAMFACEFPS;
	CAMFACEdestimage= liqimage_hold(  argCAMFACEdestimage );
	xsurface_drawclear_yuv(CAMFACEdestimage,0,128,128);
	CAMFACEUpdateCallback=argCAMFACEUpdateCallback;
	liqapp_log("liqcameraface: gst_init");
	GstElement *camera_src;
	GstElement *csp_filter;
	GstElement *image_sink;
	GstCaps *	caps;
	gst_init(NULL,NULL);
	liqapp_log("liqcameraface: creating pipeline elements");
	CAMFACEpipeline = gst_pipeline_new("liqbase-camera");
	camera_src   = gst_element_factory_make(VIDEO_SRC,          "camera_src");
	csp_filter   = gst_element_factory_make("ffmpegcolorspace", "csp_filter");
	image_sink   = gst_element_factory_make("fakesink",         "image_sink");
	if(!(CAMFACEpipeline && camera_src && csp_filter && image_sink))
	{
		liqapp_warnandcontinue(-1,"liqcameraface : Couldn't create pipeline elements");
		return -1;
	}
	// use face camera :)
	// do this once bullseye is reliably detecting again
	g_object_set(G_OBJECT(camera_src), "device", "/dev/video1", NULL);
    CAMFACE_isface = 1;
	
	// ############################################################################ add everything to the CAMFACEpipeline
	liqapp_log("liqcameraface: pipeline joining");
	gst_bin_add_many(GST_BIN(CAMFACEpipeline), camera_src, csp_filter,  image_sink, NULL);
	
	// ############################################################################ prepare the camera filter
	liqapp_log("liqcameraface: obtaining video caps");
    char capstr[FILENAME_MAX];
    snprintf(capstr,sizeof(capstr),"video/x-raw-yuv,format=(fourcc)UYVY,width=%i,height=%i",CAMFACEW,CAMFACEH);
    caps = gst_caps_from_string(capstr);
	if(!gst_element_link_filtered(camera_src, csp_filter, caps))
	{
		liqapp_warnandcontinue(-1,"liqcameraface : Could not link camera_src to csp_filter");
		return -1;
	}
	gst_caps_unref(caps);
	
	
	
	// ############################################################################ prepare the image filter
	liqapp_log("liqcameraface: preparing filter");
	
	caps = gst_caps_new_simple("video/x-raw-yuv",
			"width",  G_TYPE_INT, CAMFACEW,
			"height", G_TYPE_INT, CAMFACEH,
			NULL);
		if(!gst_element_link_filtered(csp_filter, image_sink, caps))
		{
			liqapp_warnandcontinue(-1,"liqcameraface : Could not link csp_filter to image_sink");
			return -1;
		}
	gst_caps_unref(   caps);
	// ############################################################################ finally make sure we hear about the events
	liqapp_log("liqcameraface: adding signals");
	g_object_set(     G_OBJECT(image_sink), "signal-handoffs", TRUE,                              NULL);
	g_signal_connect( G_OBJECT(image_sink), "handoff",         G_CALLBACK(buffer_probe_callback), NULL);
	liqapp_log("liqcameraface: starting stream");
	gst_element_set_state(CAMFACEpipeline, GST_STATE_PLAYING);
	liqapp_log("liqcameraface: done");
	return 0;
}


liqimage * liqcameraface_getimage()
{
	// return the current image of this camera, if NULL camera is switched off
	return CAMFACEdestimage;
}

void liqcameraface_stop()
{
	liqapp_log("liqcameraface_stop first");
	if(!CAMFACEpipeline)
	{
		liqapp_log("liqcameraface_stop no pipe");
		// camera pipeline not in use..
		return;
	}
	liqapp_log("liqcameraface_stop stopping");
	gst_element_set_state(CAMFACEpipeline,GST_STATE_NULL);
	liqapp_log("liqcameraface_stop unrefing");
	gst_object_unref(GST_OBJECT(CAMFACEpipeline));
	liqapp_log("liqcameraface_stop clearing vars");
	CAMFACEW=0;
	CAMFACEH=0;
	CAMFACEFPS=0;
	CAMFACEdestimage=NULL;
	CAMFACEUpdateCallback=NULL;
	CAMFACEpipeline=NULL;
	liqapp_log("liqcameraface_stop releasing destimage");
	liqimage_release(CAMFACEdestimage);
	// todo: find out if i need an anti-"get_init(..)" call here?...
	liqapp_log("liqcameraface_stop done.");
}

#ifdef __cplusplus
}
#endif

