
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

	char *tit = strdup(app.title);
	// 20090616_224615 lcuk : qwerty12 says that i am not allowed '-' characters in StartupWMClass, so no '-' chars allowed
	// 20090616_224744 lcuk : example which blows hildon desktop:   StartupWMClass=liqbase-playground
	// 20090616_224804 lcuk : force all '-' characters in title to be '_' and change .desktop and service files to match
	
	char *t=tit;
	while(t && *t)
	{
		if(*t=='-') *t='_';
		t++;
	}

	free(tit);

	char buf[255];
	snprintf(buf,sizeof(buf),"org.maemo.%s",tit);

	liqapp_log("hildon/osso initializing context: %s",buf);

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




