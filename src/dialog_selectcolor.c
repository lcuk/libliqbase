// this file is part of liqbase by Gary Birkett
		
#include "liqbase.h"
#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqcell_easyhandler.h"


#ifdef __cplusplus
extern "C" {
#endif
		
//#####################################################################
//#####################################################################
//##################################################################### dialog_selectcolor :: by gary birkett
//#####################################################################
//#####################################################################
		
		
/**	
 * dialog_selectcolor widget refresh, all params set, present yourself to the user.
 */	
static int dialog_selectcolor_refresh(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	// there might be an OS level variable called filter
	// it should be set and adjusted correctly prior to calling this routine
	// you should do your best to account for this filter in any way you see fit
	return 0;
}
/**	
 * dialog_selectcolor dialog_open - the user zoomed into the dialog
 */	
static int dialog_selectcolor_dialog_open(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	liqcell *picture1 = liqcell_child_lookup(self, "picture1");

	char *t = liqcell_propgets(self,"colorselected",NULL);
	if(t && *t)
	{
		liqcell_propsets(picture1,"backcolor",t);
	}
	
	return 0;
}
/**	
 * dialog_selectcolor dialog_close - the dialog was closed
 */	
static int dialog_selectcolor_dialog_close(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor widget shown - occurs once per lifetime
 */	
static int dialog_selectcolor_shown(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor mouse - occurs all the time as you stroke the screen
 */	
static int dialog_selectcolor_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectcolor click - occurs when a short mouse stroke occured
 */	
static int dialog_selectcolor_click(liqcell *self, liqcellclickeventargs *args,liqcell *context)
{
	return 0;
}

/**	
 * dialog_selectcolor paint - being rendered.  use the vgraph held in args to do custom drawing at scale
 */	
//static int dialog_selectcolor_paint(liqcell *self, liqcellpainteventargs *args,liqcell *context)
//{
//	// big heavy event, use sparingly
//	return 0;
//}
/**	
 * dialog_selectcolor dynamic resizing
 */	
static int dialog_selectcolor_resize(liqcell *self,liqcelleventargs *args, liqcell *context)
{
/*
	float sx=((float)self->w)/((float)self->innerw);
	float sy=((float)self->h)/((float)self->innerh);
	
	liqcell *greycube1 = liqcell_child_lookup(self, "greycube1");
	liqcell *colorcube1 = liqcell_child_lookup(self, "colorcube1");
	liqcell *picture1 = liqcell_child_lookup(self, "picture1");
	liqcell *currentselhead = liqcell_child_lookup(self, "currentselhead");
	liqcell *brightnesshead = liqcell_child_lookup(self, "brightnesshead");
	liqcell *title = liqcell_child_lookup(self, "title");
	liqcell *cmdaccept = liqcell_child_lookup(self, "cmdaccept");
	liqcell *colorhead = liqcell_child_lookup(self, "colorhead");
	liqcell_setrect_autoscale( colorcube1, 10,104, 366,322, sx,sy);
	liqcell_setrect_autoscale( picture1, 504,104, 290,322, sx,sy);
	liqcell_setrect_autoscale( currentselhead, 504,66, 292,36, sx,sy);
	liqcell_setrect_autoscale( brightnesshead, 382,66, 116,36, sx,sy);
	liqcell_setrect_autoscale( title, 0,0, 800,56, sx,sy);
	liqcell_setrect_autoscale( cmdaccept, 556,432, 206,48, sx,sy);
	liqcell_setrect_autoscale( colorhead, 8,66, 368,36, sx,sy);
	liqcell_setrect_autoscale( greycube1, 382,104, 116,322, sx,sy);
 */	return 0;
}


/**	
 * colorcube mouse - occurs all the time as you stroke the screen
 */	
static int colorcube_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *dialog_selectcolor)
{
	liqcell *colorcube1 = liqcell_child_lookup(dialog_selectcolor, "colorcube1");
	liqcell *picture1 = liqcell_child_lookup(dialog_selectcolor, "picture1");
	// the grey cube has been touched
	// we must ensure the colorcube shine is set to the grey value we hit
	
//int		vgraph_pget(       				vgraph *self, int x, int y, unsigned char *grey,unsigned char *u,unsigned char *v);

	int px = args->mex;// - args->ox;
	int py = args->mey;// - args->oy;

	unsigned char cy=0;
	unsigned char cu=0;
	unsigned char cv=0;
	
	vgraph_pget( args->graph, px,py, &cy,&cu,&cv );
	
	//liqcell_propseti(colorcube1,"colorcube_brightness",cy);
	liqcell_setdirty(colorcube1,1);
	
	char buf[64];
	snprintf(buf,sizeof(buf),"yuv(%i,%i,%i)",cy,cu,cv);
	
	liqcell_propsets(  picture1, "backcolor",buf);

	return 0;
}

/**	
 * greycube mouse - occurs all the time as you stroke the screen
 */	
static int greycube_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *dialog_selectcolor)
{
	liqcell *colorcube1 = liqcell_child_lookup(dialog_selectcolor, "colorcube1");
	liqcell *picture1 = liqcell_child_lookup(dialog_selectcolor, "picture1");
	// the grey cube has been touched
	// we must ensure the colorcube shine is set to the grey value we hit
	
//int		vgraph_pget(       				vgraph *self, int x, int y, unsigned char *grey,unsigned char *u,unsigned char *v);

	int px = args->mex;// - args->ox;
	int py = args->mey;// - args->oy;

	unsigned char cy=0;
	unsigned char cu=0;
	unsigned char cv=0;
	
	vgraph_pget( args->graph, px,py, &cy,&cu,&cv );
	
	liqcell_propseti(colorcube1,"colorcube_brightness",cy);
	liqcell_setdirty(colorcube1,1);
	
	char buf[64];
	snprintf(buf,sizeof(buf),"yuv(%i,128,128)",cy);
	liqcell_propsets(  picture1, "backcolor",buf);
	return 0;
}

/**	
 * dialog_selectcolor.cmdaccept clicked
 */	
static int cmdaccept_click(liqcell *self,liqcellclickeventargs *args, liqcell *dialog_selectcolor)
{
	liqcell *picture1 = liqcell_child_lookup(dialog_selectcolor, "picture1");

	char *t = liqcell_propgets(picture1,"backcolor",NULL);
	if(t && *t)
	{
		liqcell_propsets(dialog_selectcolor,"colorselected",t);
	}
	

    liqcell_propseti(dialog_selectcolor,"dialog_running",0);

	return 0;
}

/**	
 * create a new dialog_selectcolor widget
 */	
liqcell *dialog_selectcolor_create()
{
	liqcell *self = liqcell_quickcreatewidget("dialog_selectcolor", "form", 800, 480);
	if(!self) {liqapp_log("liqcell error not create 'dialog_selectcolor'"); return NULL;  } 
	
	// Optimization:  The aim is to REDUCE the number of drawn layers and operations called.
	// Optimization:  use only what you NEED to get an effect
	// Optimization:  Minimal layers and complexity
	// Optimization:  defaults: background, prefer nothing, will be shown through if there is a wallpaper
	// Optimization:  defaults: text, white, very fast rendering
	
	//############################# title:label
	liqcell *title = liqcell_quickcreatevis("title", "label", 0, 0, 800, 56);
	liqcell_setfont(	title, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
	liqcell_setcaption(title, "select color" );
	liqcell_propsets(  title, "textcolor", "rgb(255,255,255)" );
	liqcell_propsets(  title, "backcolor", "xrgb(128,128,128)" );
	liqcell_propseti(  title, "textalign", 0 );
	liqcell_propseti(  title, "textaligny", 0 );
	liqcell_child_append(  self, title);
	

	//############################# colorhead:label
	liqcell *colorhead = liqcell_quickcreatevis("colorhead", "label", 0, 56, 325, 34);
	liqcell_setfont(	colorhead, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (22), 0) );
	liqcell_setcaption(colorhead, "Color" );
	liqcell_propsets(  colorhead, "textcolor", "rgb(255,255,255)" );
	liqcell_propsets(  colorhead, "backcolor", "rgb(0,64,64)" );
	liqcell_propseti(  colorhead, "textalign", 0 );
	liqcell_propseti(  colorhead, "textaligny", 0 );
	liqcell_child_append(  self, colorhead);
	
	//############################# colorcube1:colorcube
	liqcell *colorcube1 = liqcell_quickcreatevis("colorcube1", "dialog_selectcolor_colorcube", 0, 90, 325, 320);
	liqcell_handleradd_withcontext(colorcube1, "mouse", (void*)colorcube_mouse,self );
	liqcell_child_append(  self, colorcube1);
	
	

	//############################# currentselhead:label
	liqcell *currentselhead = liqcell_quickcreatevis("currentselhead", "label", 475, 56, 325, 34);
	liqcell_setfont(	currentselhead, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (22), 0) );
	liqcell_setcaption(currentselhead, "Selected" );
	liqcell_propsets(  currentselhead, "textcolor", "rgb(255,255,255)" );
	liqcell_propsets(  currentselhead, "backcolor", "rgb(0,64,64)" );
	liqcell_propseti(  currentselhead, "textalign", 0 );
	liqcell_propseti(  currentselhead, "textaligny", 0 );
	liqcell_child_append(  self, currentselhead);
	
	//############################# currentsel:picturebox
	liqcell *picture1 = liqcell_quickcreatevis("picture1", "picturebox", 475, 90, 325, 320);
	//liqcell_setfont(	picture1, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (12), 0) );
	//liqcell_propsets(  picture1, "textcolor", "rgb(0,0,0)" );
	//liqcell_propsets(  picture1, "backcolor", "rgb(0,0,0)" );
	//liqcell_propsets(  picture1, "bordercolor", "rgb(255,255,255)" );
	liqcell_child_append(  self, picture1);
	
	
	//############################# brightnesshead:label
	liqcell *brightnesshead = liqcell_quickcreatevis("brightnesshead", "label", 325, 56, 150, 34);
	liqcell_setfont(	brightnesshead, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (22), 0) );
	liqcell_setcaption(brightnesshead, "Brightness" );
	liqcell_propsets(  brightnesshead, "textcolor", "rgb(255,255,255)" );
	liqcell_propsets(  brightnesshead, "backcolor", "rgb(0,64,64)" );
	liqcell_propseti(  brightnesshead, "textalign", 0 );
	liqcell_propseti(  brightnesshead, "textaligny", 0 );
	liqcell_child_append(  self, brightnesshead);
	
	//############################# greycube1:greycube
	liqcell *greycube1 = liqcell_quickcreatevis("greycube1", "dialog_selectcolor_greycube", 325, 90, 150, 320);
	liqcell_handleradd_withcontext(greycube1, "mouse", (void*)greycube_mouse,self );
	liqcell_child_append(  self, greycube1);
	
	

	//############################# cmdaccept:label
	liqcell *cmdaccept = liqcell_quickcreatevis("cmdaccept", "label", 580, 420, 210, 60);
	liqcell_setfont(	cmdaccept, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
	liqcell_setcaption(cmdaccept, "Select" );
	liqcell_propsets(  cmdaccept, "textcolor", "rgb(255,255,255)" );
	liqcell_propsets(  cmdaccept, "backcolor", "xrgb(0,64,0)" );
	liqcell_propsets(  cmdaccept, "bordercolor", "rgb(255,255,255)" );
	liqcell_propseti(  cmdaccept, "textalign", 2 );
	liqcell_propseti(  cmdaccept, "textaligny", 2 );
	liqcell_handleradd_withcontext(cmdaccept, "click", (void*)cmdaccept_click, self );
	liqcell_child_append(  self, cmdaccept);
	

	
	liqcell_propsets(  self, "backcolor", "rgb(0,0,0)" );
	liqcell_handleradd_withcontext(self, "refresh", (void*)dialog_selectcolor_refresh ,self);
	liqcell_handleradd_withcontext(self, "shown", (void*)dialog_selectcolor_shown ,self);
	liqcell_handleradd_withcontext(self, "resize", (void*)dialog_selectcolor_resize ,self);
	liqcell_handleradd_withcontext(self, "mouse", (void*)dialog_selectcolor_mouse,self );
	liqcell_handleradd_withcontext(self, "click", (void*)dialog_selectcolor_click ,self);
	liqcell_handleradd_withcontext(self, "dialog_open", (void*)dialog_selectcolor_dialog_open ,self);
	liqcell_handleradd_withcontext(self, "dialog_close", (void*)dialog_selectcolor_dialog_close ,self);
	return self;
}

#ifdef __cplusplus
}
#endif

