
// hildon attributes and links into the main system

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include <X11/extensions/XShm.h>

#ifdef USE_OSSO
#include <libosso.h>
osso_context_t *osso_context;
#endif



#include "liqapp.h"

int liqapp_hildon_init()
{
	// call after initializing app.* structure
#ifdef USE_OSSO
	char buf[255];
	snprintf(buf,sizeof(buf),"org.maemo.%s",app.title);
	osso_context = osso_initialize(buf, "1.0", TRUE, NULL);
	/* Check that initialization was ok */
	if (osso_context == NULL)
	{
		liqapp_log("liqapp hildon osso init error");
	    return -1;
	}

    osso_display_state_on(osso_context);
    osso_display_blanking_pause(osso_context);
#endif
	return 0;
}


int liqapp_hildon_close()
{
	// all done now, call at the end :)
#ifdef USE_OSSO
	osso_deinitialize(osso_context);
#endif
	return 0;
}




