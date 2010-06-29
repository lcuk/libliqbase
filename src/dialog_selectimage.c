// this file is part of liqbase by Gary Birkett
		
#include "liqbase.h"
#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqcell_easyhandler.h"

#ifdef __cplusplus
extern "C" {
#endif

    // todo, move this into the lib at earliest opportunity
    // Tue Oct 20 15:44:27 2009 lcuk : done
    
    
    static int liqcell_filter_run(liqcell *c,char *searchterm)
    {
        liqcellfiltereventargs filterargs;
        filterargs.filterinuse= (searchterm ? 1 : 0);
        filterargs.searchterm = searchterm;
        filterargs.searchtags = NULL;
        filterargs.resultoutof= 1;
        filterargs.resultshown= 0;
        
        if(!filterargs.resultshown)
        {
            if(searchterm && *searchterm)
            {
            }
            else
            {
                filterargs.resultshown = 1;
            }
        }
        
        if(!filterargs.resultshown)
        {
            if(c->name)
                filterargs.resultshown = ( stristr(c->name,searchterm) != NULL );
        }
    
        if(!filterargs.resultshown)
        {
            if(c->classname)
                filterargs.resultshown = ( stristr(c->classname,searchterm) != NULL );
        }
        
        liqcell *ccontent = liqcell_getcontent( c );
        if(ccontent)
        {
            liqapp_log("searching in cont '%s', %i",c->name,filterargs.resultshown);
            liqcell_handlerrun( ccontent , "filter", &filterargs );
        }
        else
        {
            liqapp_log("searching in flat '%s', %i",c->name,filterargs.resultshown);
            liqcell_handlerrun( c , "filter", &filterargs );
        }
    
        if(filterargs.resultshown)
        {
            liqcell_setvisible(c,1);
            return 1;
        }
        else
        {
            liqcell_setvisible(c,0);
            return 0;
        }
    }
    
    
		
		
//#####################################################################
//#####################################################################
//##################################################################### dialog_selectimage :: by gary birkett
//#####################################################################
//#####################################################################

		
/**	
 * dialog_selectimage widget refresh, all params set, present yourself to the user.
 */	
static int dialog_selectimage_refresh(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	// there might be an OS level variable called filter
	// it should be set and adjusted correctly prior to calling this routine
	// you should do your best to account for this filter in any way you see fit
	return 0;
}
/**	
 * dialog_selectimage dialog_open - the user zoomed into the dialog
 */	
static int dialog_selectimage_dialog_open(liqcell *self,liqcelleventargs *args, liqcell *dialog_selectimage)
{
    liqcell_propremoves(  dialog_selectimage, "imagefilenameselected"  );
	return 0;
}
/**	
 * dialog_selectimage dialog_close - the dialog was closed
 */	
static int dialog_selectimage_dialog_close(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectimage widget shown - occurs once per lifetime
 */	
static int dialog_selectimage_shown(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectimage mouse - occurs all the time as you stroke the screen
 */	
static int dialog_selectimage_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectimage click - occurs when a short mouse stroke occured
 */	
static int dialog_selectimage_click(liqcell *self, liqcelleventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * dialog_selectimage keypress - the user pressed a key
 */	
//static int dialog_selectimage_keypress(liqcell *self, liqcellkeyeventargs *args,liqcell *context)
//{
//	return 0;
//}
/**	
 * dialog_selectimage keyrelease - the user released a key
 */	
//static int dialog_selectimage_keyrelease(liqcell *self, liqcellkeyeventargs *args,liqcell *context)
//{
//	return 0;
//}
/**	
 * dialog_selectimage paint - being rendered.  use the vgraph held in args to do custom drawing at scale
 */	
//static int dialog_selectimage_paint(liqcell *self, liqcellpainteventargs *args,liqcell *context)
//{
//	// big heavy event, use sparingly
//	return 0;
//}
/**	
 * dialog_selectimage dynamic resizing
 */	
static int dialog_selectimage_resize(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	float sx=((float)self->w)/((float)self->innerw);
	float sy=((float)self->h)/((float)self->innerh);
	
	liqcell *cmdselect = liqcell_child_lookup(self, "cmdselect");
	liqcell *dialog_selectimage_grid1 = liqcell_child_lookup(self, "dialog_selectimage_grid1");
	liqcell *title = liqcell_child_lookup(self, "title");
	liqcell_setrect_autoscale( cmdselect, 580,420, 220,60, sx,sy);
	liqcell_setrect_autoscale( dialog_selectimage_grid1, 0,58, 800,420, sx,sy);
	liqcell_setrect_autoscale( title, 0,0, 800,56, sx,sy);
	return 0;
}

/**	
 * dialog_selectimage.cmdselect clicked
 */	
static int cmdselect_click(liqcell *self,liqcelleventargs *args, liqcell *dialog_selectimage)
{
    // we are done here.

    liqcell *dialog_selectimage_grid1 = liqcell_child_lookup(dialog_selectimage, "dialog_selectimage_grid1");
    
           //########## major sideways hack.  classes need declarations and header files.
            int dialog_selectimage_grid_getselectedphoto_filename(liqcell *self,char *buffer,int bufferlen);
            char selfn[FILENAME_MAX]={0};
            dialog_selectimage_grid_getselectedphoto_filename(dialog_selectimage_grid1,selfn,sizeof(selfn) );

            //liqapp_log("selimg a %s",selfn);
            
            //char *selfn=liqcell_propgets(  oneedit, "imagefilenameselected",NULL );
            
            if( *selfn )
            {
                liqapp_log("selimg got sel %s",selfn);
                liqcell_propsets(  dialog_selectimage, "imagefilenameselected", selfn  );               
            }           

    
    liqcell_setvisible(dialog_selectimage,0);
	return 0;
}
    
//#####################################################################
//#####################################################################
//#####################################################################
//#####################################################################
//#####################################################################








	static int search_click(liqcell *self, liqcelleventargs *args, liqcell *dialog_selectimage)
	{
		// try to add this tag :)
		//liqcell *body = liqcell_child_lookup(dialog_selectimage,"body");
		char *searchterm = liqcell_getcaption(self);
		if(!searchterm || !*searchterm) return 1;
		liqcell_setcaption(self,"");
		return 1;
	}
	static int search_change(liqcell *self, liqcelleventargs *args, liqcell *dialog_selectimage)
	{
		// examine each tag and if matches the search show it, otherwise dont..
		char *searchterm = liqcell_getcaption(self);
		/*liqcell *body = liqcell_child_lookup(dialog_selectimage,"body");
		liqcell *c = liqcell_getlinkchild_visual(body);
		while(c)
		{
			liqcell_filter_run(c,searchterm);
			c=liqcell_getlinknext_visual(c);
		}
        */
        
        liqcell *dialog_selectimage_grid1=liqcell_child_lookup(dialog_selectimage,"dialog_selectimage_grid1");
        liqcell_filter_run(dialog_selectimage_grid1,searchterm);
        
        /*
        liqcell *searchinprogress = liqcell_child_lookup(body,"searchinprogress");
        liqcell_setvisible(searchinprogress,0);
        if(liqcell_child_countvisible(body)==0)
        {
            liqcell_setcaption_printf(searchinprogress,"No results found");
            liqcell_propsets(  searchinprogress, "backcolor",   "xrgb(40,0,0)" );
        }
        else
        {
            liqcell_setcaption_printf(searchinprogress,"Search results:",liqcell_child_countvisible(body) );
            liqcell_propsets(  searchinprogress, "backcolor",   "xrgb(0,40,0)" );
        }
        if(!searchterm || !*searchterm)
            liqcell_setvisible(searchinprogress,0);
        else
            liqcell_setvisible(searchinprogress,1);
            
            
		//liqcell_setrect(body,   0,40,dialog_selectimage->w,dialog_selectimage->h-40);
		liqcell_setrect(body,   0,0,dialog_selectimage->w,dialog_selectimage->h);
		liqcell_child_arrange_easytile( body );
		//liqcell_child_arrange_makegrid_fly(body,3,3);
		liqcell_propseti(self,"arrangecomplete",0);
		//liqcell_setpos(body,0,40);
		if(!searchterm || !*searchterm)
		{
			// bit of magic here..
			liqcell_setvisible(self,0);
		}
        */
		return 1;
		
	}
	    
    
    
	static int dialog_selectimage_keypress(liqcell *self, liqcellkeyeventargs *args,liqcell *dialog_selectimage)
	{
		liqcell *search = liqcell_child_lookup(dialog_selectimage,"search");
		if(liqcell_getvisible(search)==0)
		{
			// start it just off screen
			liqcell_setpos( search, liqcell_getx(search), liqcell_geth(dialog_selectimage) );
			liqcell_setvisible(search,1);
		}
		return liqcell_handlerrun(search,"keypress",args);
	}
	static int dialog_selectimage_keyrelease(liqcell *self, liqcellkeyeventargs *args,liqcell *dialog_selectimage)
	{
		liqcell *search = liqcell_child_lookup(dialog_selectimage,"search");
		return liqcell_handlerrun(search,"keyrelease",args);
	}
    
	static int dialog_selectimage_paint(liqcell *self, liqcellpainteventargs *args,liqcell *dialog_selectimage)
	{
		liqcell *search = liqcell_child_lookup(dialog_selectimage,"search");
		if(liqcell_getvisible(search))
		{
			if( liqcell_gety(search) > ( liqcell_geth(dialog_selectimage) - liqcell_geth(search) )  )
			{
				// move it a bit more onscreen
				int dif = liqcell_gety(search) - ( liqcell_geth(dialog_selectimage) - liqcell_geth(search) );
				//if(dif>5)dif=5;
				liqcell_setpos( search, liqcell_getx(search), liqcell_gety(search) - dif );
				liqcell_setdirty(dialog_selectimage,1);
			}
		}
        return 0;
    }

/**	
 * create a new dialog_selectimage widget
 */	
liqcell *dialog_selectimage_create()
{
	liqcell *self = liqcell_quickcreatewidget("dialog_selectimage", "form", 800, 480);
	if(!self) {liqapp_log("liqcell error not create 'dialog_selectimage'"); return NULL;  } 
	
	// Optimization:  The aim is to REDUCE the number of drawn layers and operations called.
	// Optimization:  use only what you NEED to get an effect
	// Optimization:  Minimal layers and complexity
	// Optimization:  defaults: background, prefer nothing, will be shown through if there is a wallpaper
	// Optimization:  defaults: text, white, very fast rendering

	//############################# title:label
	liqcell *title = liqcell_quickcreatevis("title", "label", 0, 0, 800, 56);
	liqcell_setfont(	title, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
	liqcell_setcaption(title, "select image" );
	liqcell_propsets(  title, "textcolor", "rgb(255,255,255)" );
	liqcell_propsets(  title, "backcolor", "xrgb(0,0,0)" );
	liqcell_propseti(  title, "textalign", 0 );
	liqcell_child_append(  self, title);
	//############################# dialog_selectimage_grid1:liqrecentphotos
	liqcell *dialog_selectimage_grid1 = liqcell_quickcreatevis("dialog_selectimage_grid1", "dialog_selectimage_grid", 0, 58, 800, 420);
	liqcell_child_append(  self, dialog_selectimage_grid1);
    //############################# cmdselect:commandbutton
	liqcell *cmdselect = liqcell_quickcreatevis("cmdselect", "commandbutton", 580, 420, 210, 60);
	liqcell_setfont(	cmdselect, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
	liqcell_setcaption(cmdselect, "Select" );
	liqcell_propsets(  cmdselect, "textcolor", "rgb(255,255,255)" );
	liqcell_propsets(  cmdselect, "backcolor", "xrgb(0,64,0)" );
	liqcell_propsets(  cmdselect, "bordercolor", "rgb(255,255,255)" );
	liqcell_propseti(  cmdselect, "textalign",  2 );
	liqcell_propseti(  cmdselect, "textaligny", 2 );
	liqcell_handleradd_withcontext(cmdselect, "click", (void*)cmdselect_click, self );
    //liqcell_setenabled(cmdselect,0);
	liqcell_child_append(  self, cmdselect);
    
    
		liqcell *search = liqcell_quickcreatevis("search","textbox",200 ,self->h-40,   350, 60);
		liqcell_setfont(   search,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (32), 0) );
		liqcell_setcaption(search, "" );
		liqcell_propsets(  search, "textcolor",   "rgb(255,255,255)" );
		liqcell_propsets(  search, "backcolor",   "xrgb(100,140,100)" );
		liqcell_handleradd_withcontext( search,    "click",           (void*)search_click,  self );
		liqcell_handleradd_withcontext( search,    "captionchange",   (void*)search_change, self );
		liqcell_setvisible(search,0);		// watch this!
		liqcell_child_append( self, search );
		
		liqcell_handleradd_withcontext(self,    "keypress",   (void*)dialog_selectimage_keypress,  self);
		liqcell_handleradd_withcontext(self,    "keyrelease", (void*)dialog_selectimage_keyrelease,self);
 		liqcell_handleradd_withcontext(self,    "paint",      (void*)dialog_selectimage_paint,  	self);
        
    
    //liqcell_propsets(  self, "backcolor", "rgb(0,0,0)" );
	liqcell_handleradd_withcontext(self, "refresh", (void*)dialog_selectimage_refresh ,self);
	liqcell_handleradd_withcontext(self, "shown", (void*)dialog_selectimage_shown ,self);
	liqcell_handleradd_withcontext(self, "resize", (void*)dialog_selectimage_resize ,self);
	liqcell_handleradd_withcontext(self, "mouse", (void*)dialog_selectimage_mouse,self );
	liqcell_handleradd_withcontext(self, "click", (void*)dialog_selectimage_click ,self);
	liqcell_handleradd_withcontext(self, "dialog_open", (void*)dialog_selectimage_dialog_open ,self);
	liqcell_handleradd_withcontext(self, "dialog_close", (void*)dialog_selectimage_dialog_close ,self);
	
	
	// allow the image thumbnails to be lazyloaded :)  (test for burchr)
	liqcell_propseti(self,"idle_lazyrun_wanted",1);
	
	return self;
}

#ifdef __cplusplus
}
#endif

