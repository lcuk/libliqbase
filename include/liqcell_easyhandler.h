

#ifndef liqcell_EASYHANDLER_H
#define liqcell_EASYHANDLER_H 1

#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"

// this module defines simple default event handlers for cells


	int 	liqcell_easyhandler_kinetic_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *context);

	int 	liqcell_easyhandler_content_zoom_click(liqcell *self, liqcellclickeventargs *args,liqcell *context);
	

#endif
