
#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks

#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>


#include <unistd.h>
#include <features.h>		/* Uses _GNU_SOURCE to define getsubopt in stdlib.h */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <dirent.h>
#include <math.h>
#include <sys/klog.h>

#ifdef __cplusplus
extern "C" {
#endif



/*
// New V4L2 APIs for digital camera
//Object recognition
 // hmmmm
 // http://webcache.googleusercontent.com/search?q=cache:ZPeaTbFfAgoJ:www.celinuxforum.org/CelfPubWiki/ELC2009Presentations%3Faction%3DAttachFile%26do%3Dget%26target%3DFramework_for_digital_camera_in_linux-in_detail.ppt+%28V4L2_CID_CAMERA_CLASS_BASE%2B20%29&cd=1&hl=en&ct=clnk&gl=uk&client=firefox-a
 
 // also look here
 http://talk.maemo.org/showthread.php?p=670491#post670491
 
 // also see
 http://forums.internettablettalk.com/showthread.php?p=455828#post455828
 
Nokia-N900:~# /root/v4l2-ctl --list-ctrls
                     brightness (int)  : min=0 max=255 step=1 default=0 value=0
                       contrast (int)  : min=0 max=255 step=1 default=16 value=16
               exposure_time_us (int)  : min=50 max=33400 step=50 default=33400 value=33317 flags=slider
                    gain_0_1_ev (int)  : min=0 max=40 step=1 default=0 value=0 flags=slider
                  color_effects (menu) : min=0 max=2 default=0 value=0
                 focus_absolute (int)  : min=0 max=1023 step=1 default=0 value=143
error 22 getting ext_ctrl Flash strobe
               flash_timeout_us (int)  : min=1000 max=500000 step=54600 default=500000 value=500000 flags=slider
                flash_intensity (int)  : min=12 max=19 step=1 default=12 value=12 flags=slider
                torch_intensity (int)  : min=0 max=1 step=1 default=0 value=0 flags=slider
            indicator_intensity (int)  : min=0 max=7 step=1 default=0 value=0 flags=slider
              test_pattern_mode (menu) : min=0 max=8 default=0 value=0
          focus_ramping_time_us (int)  : min=0 max=3200 step=50 default=0 value=0
             focus_ramping_mode (menu) : min=0 max=1 default=0 value=0
            short_circuit_fault (bool) : default=0 value=0 flags=read-only
          overtemperature_fault (bool) : default=0 value=0 flags=read-only
                  timeout_fault (bool) : default=0 value=0 flags=read-only
              overvoltage_fault (bool) : default=0 value=0 flags=read-only
                    frame_width (int)  : min=0 max=0 step=0 default=0 value=3984 flags=read-only
                   frame_height (int)  : min=0 max=0 step=0 default=0 value=672 flags=read-only
                  visible_width (int)  : min=0 max=0 step=0 default=0 value=864 flags=read-only
                 visible_height (int)  : min=0 max=0 step=0 default=0 value=656 flags=read-only
                 pixel_clock_hz (int)  : min=0 max=0 step=0 default=0 value=80000000 flags=read-only
                    sensitivity (int)  : min=0 max=0 step=0 default=0 value=65536 flags=read-only
*/




//#####################################################################
//#####################################################################
//#####################################################################
//#####################################################################
//#####################################################################

// http://www.gitorious.org/omap3camera/mainline/blobs/ee89561891a2e97f6d9a1996a0fdc12f5f36b96f/include/linux/videodev2.h#line1153
// or
// http://www.gitorious.org/omap3camera/mainline/commit/edb5f99

// and this desribes them in use
// http://gitorious.org/omap3camera/mainline/commit/a41027c

#define V4L2_CID_EXPOSURE_ABSOLUTE              (V4L2_CID_CAMERA_CLASS_BASE+2)
//#define V4L2_CID_FOCUS_ABSOLUTE                 (V4L2_CID_CAMERA_CLASS_BASE+10)
#define V4L2_CID_ZOOM_ABSOLUTE                  (V4L2_CID_CAMERA_CLASS_BASE+13)
//#define V4L2_CID_FLASH_INTENSITY                (V4L2_CID_CAMERA_CLASS_BASE+19)
#define V4L2_CID_TORCH_INTENSITY                (V4L2_CID_CAMERA_CLASS_BASE+20)
#define V4L2_CID_INDICATOR_INTENSITY            (V4L2_CID_CAMERA_CLASS_BASE+21)

static int liqcamera_currentfocus=0;


int doioctl(int fd, unsigned long int request, void *parm, const char *name)
{
	int retVal = ioctl(fd, request, parm);

	if (retVal < 0)
		liqapp_log("%s: failed: %s", name, strerror(errno));

	return retVal;
}


int liqcamera_setfocus(int newvalue)
{
	int fd = -1;
	liqcamera_currentfocus = newvalue;
	const char *device = "/dev/video0";
	
	if ((fd = open(device, O_RDWR)) < 0) {
		liqapp_log("liqcamera_setfocus: Failed to open %s: %s\n", device,
			strerror(errno));
		return -1;
	}
	//if(newfocus==1000) newfocus=1023;
	struct v4l2_control ctrl = { 0 };
	ctrl.id = V4L2_CID_FOCUS_ABSOLUTE;
	ctrl.value = newvalue;
	if (doioctl(fd, VIDIOC_S_CTRL, &ctrl, "VIDIOC_S_CTRL"))
	{
		liqapp_log("liqcamera_setfocus: error %s\n",
				strerror(errno));
	}
	close(fd);
	return 0;
}

int liqcamera_setbrightness(int newvalue)
{
	
	char buf[255];
	snprintf(buf,sizeof(buf),"/root/v4l2-ctl -c brightness=%i",newvalue);
	system(buf);
	
	
	return 0;
}

int liqcamera_setcontrast(int newvalue)
{
	
	char buf[255];
	snprintf(buf,sizeof(buf),"/root/v4l2-ctl -c contrast=%i",newvalue);
	system(buf);
	
	
	return 0;
}


int liqcamera_setexposuretime(int newvalue)
{
	newvalue=(newvalue/50)*50;
	
	char buf[255];
	snprintf(buf,sizeof(buf),"/root/v4l2-ctl -c exposure_time_us=%i",newvalue);
	system(buf);
	
	
	return 0;
}


int liqcamera_settorch(int newvalue)
{
	
	char buf[255];
	snprintf(buf,sizeof(buf),"/root/v4l2-ctl -c torch_intensity=%i",newvalue);
	system(buf);
	
	
	return 0;

	// this does not work!
	// i had to resort to hackery v4l2-ctl instead of calling the methods ...
	// what did i do wrong?
	
	
	int fd = -1;
	const char *device = "/dev/video0";



	liqapp_log("V4L2_CID_CAMERA_CLASS_BASE=%d",V4L2_CID_CAMERA_CLASS_BASE);
	liqapp_log("V4L2_CID_FOCUS_ABSOLUTE-V4L2_CID_CAMERA_CLASS_BASE =%d",V4L2_CID_FOCUS_ABSOLUTE -V4L2_CID_CAMERA_CLASS_BASE);
	liqapp_log("V4L2_CID_TORCH_INTENSITY-V4L2_CID_CAMERA_CLASS_BASE=%d",V4L2_CID_TORCH_INTENSITY-V4L2_CID_CAMERA_CLASS_BASE);
	
	if ((fd = open(device, O_RDWR)) < 0) {
		liqapp_log("liqcamera_settorch: Failed to open %s: %s\n", device,
			strerror(errno));
		return -1;
	}
	//if(newfocus==1000) newfocus=1023;
	struct v4l2_control ctrl = { 0 };
	
	

	ctrl.id = V4L2_CID_TORCH_INTENSITY;
	if (ioctl (fd, VIDIOC_G_CTRL, &ctrl) == -1) {
		liqapp_log("liqcamera_settorch: cannot get intensity (%s)\n", strerror (errno));
		close(fd);
		return -2;
	}
	liqapp_log("V4L2_CID_TORCH_INTENSITY Actual == %d", ctrl.value );
	
	
	ctrl.id = V4L2_CID_TORCH_INTENSITY;
	ctrl.value = newvalue;
	if (doioctl(fd, VIDIOC_S_CTRL, &ctrl, "VIDIOC_S_CTRL"))
	{
		liqapp_log("liqcamera_settorch: error %s\n",strerror(errno));
	}
	close(fd);
	return 0;
}

#ifdef __cplusplus
}
#endif

