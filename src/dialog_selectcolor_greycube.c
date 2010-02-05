// this file is part of liqbase by Gary Birkett
		
#include "liqbase.h"
#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqcell_easyhandler.h"
		
//#####################################################################
//#####################################################################
//##################################################################### dialog_selectcolor_greycube :: by gary birkett
//#####################################################################
//#####################################################################
		
		
/**	
 * dialog_selectcolor_greycube widget refresh, all params set, present yourself to the user.
 */	
static int dialog_selectcolor_greycube_refresh(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	// there might be an OS level variable called filter
	// it should be set and adjusted correctly prior to calling this routine
	// you should do your best to account for this filter in any way you see fit
	return 0;
}
/**	
 * dialog_selectcolor_greycube dialog_open - the user zoomed into the dialog
 */	
static int dialog_selectcolor_greycube_dialog_open(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor_greycube dialog_close - the dialog was closed
 */	
static int dialog_selectcolor_greycube_dialog_close(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor_greycube widget shown - occurs once per lifetime
 */	
static int dialog_selectcolor_greycube_shown(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor_greycube mouse - occurs all the time as you stroke the screen
 */	
static int dialog_selectcolor_greycube_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor_greycube click - occurs when a short mouse stroke occured
 */	
static int dialog_selectcolor_greycube_click(liqcell *self, liqcellclickeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor_greycube keypress - the user pressed a key
 */	
static int dialog_selectcolor_greycube_keypress(liqcell *self, liqcellkeyeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor_greycube keyrelease - the user released a key
 */	
static int dialog_selectcolor_greycube_keyrelease(liqcell *self, liqcellkeyeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor_greycube paint - being rendered.  use the vgraph held in args to do custom drawing at scale
 */	
static int dialog_selectcolor_greycube_paint(liqcell *self, liqcellpainteventargs *args,liqcell *context)
{
	// big heavy event, use sparingly
    vgraph_setpencolor(  args->graph,vcolor_YUV(128,128,128) );
    vgraph_drawgreycol(args->graph,0,0,self->w,self->h);
    
	return 0;
}
/**	
 * dialog_selectcolor_greycube dynamic resizing
 */	
static int dialog_selectcolor_greycube_resize(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	float sx=((float)self->w)/((float)self->innerw);
	float sy=((float)self->h)/((float)self->innerh);
	
	//liqcell *image2 = liqcell_child_lookup(self, "image2");
	//liqcell_setrect_autoscale( image2, 0,0, 116,322, sx,sy);
	return 0;
}

/**	
 * dialog_selectcolor_greycube_child_test_seek this function shows how to access members
 */	
	  
static void dialog_selectcolor_greycube_child_test_seek(liqcell *dialog_selectcolor_greycube)
{	  
	//liqcell *image2 = liqcell_child_lookup(dialog_selectcolor_greycube, "image2");
}	  
/**	
 * create a new dialog_selectcolor_greycube widget
 */	
liqcell *dialog_selectcolor_greycube_create()
{
	liqcell *self = liqcell_quickcreatewidget("dialog_selectcolor_greycube", "form", 116, 324);
	if(!self) {liqapp_log("liqcell error not create 'dialog_selectcolor_greycube'"); return NULL;  } 
	
	// Optimization:  The aim is to REDUCE the number of drawn layers and operations called.
	// Optimization:  use only what you NEED to get an effect
	// Optimization:  Minimal layers and complexity
	// Optimization:  defaults: background, prefer nothing, will be shown through if there is a wallpaper
	// Optimization:  defaults: text, white, very fast rendering
	//############################# image2:image
	//liqcell *image2 = liqcell_quickcreatevis("image2", "image", 0, 0, 116, 322);
	//liqcell_propsets(  image2, "bordercolor", "rgb(255,255,255)" );
	//liqcell_child_append(  self, image2);
	//liqcell_propsets(  self, "backcolor", "rgb(235,233,237)" );
	liqcell_handleradd_withcontext(self, "refresh", dialog_selectcolor_greycube_refresh ,self);
	liqcell_handleradd_withcontext(self, "shown", dialog_selectcolor_greycube_shown ,self);
	liqcell_handleradd_withcontext(self, "resize", dialog_selectcolor_greycube_resize ,self);
	//liqcell_handleradd_withcontext(self, "keypress", dialog_selectcolor_greycube_keypress,self );
	//liqcell_handleradd_withcontext(self, "keyrelease", dialog_selectcolor_greycube_keyrelease ,self);
	//liqcell_handleradd_withcontext(self, "mouse", dialog_selectcolor_greycube_mouse,self );
	//liqcell_handleradd_withcontext(self, "click", dialog_selectcolor_greycube_click ,self);
	liqcell_handleradd_withcontext(self, "paint", dialog_selectcolor_greycube_paint ,self); // use only if required, heavyweight
	//liqcell_handleradd_withcontext(self, "dialog_open", dialog_selectcolor_greycube_dialog_open ,self);
	//liqcell_handleradd_withcontext(self, "dialog_close", dialog_selectcolor_greycube_dialog_close ,self);
	return self;
}

