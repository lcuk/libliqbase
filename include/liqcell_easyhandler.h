

#ifndef liqcell_EASYHANDLER_H
#define liqcell_EASYHANDLER_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"

// this module defines simple default event handlers for cells


	int 	liqcell_easyhandler_kinetic_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *context);
	
	// EXAMPLE: liqcell_handleradd_withcontext(b,    	"mouse",     liqcell_easyhandler_kinetic_mouse,self);
	





	int 	liqcell_easyhandler_content_zoom_click(liqcell *self, liqcellclickeventargs *args,liqcell *context);
	
	// EXAMPLE: liqcell_handleradd_withcontext(b,    	"click",     liqcell_easyhandler_content_zoom_click,self);
	
#ifdef __cplusplus
}
#endif	

#endif
