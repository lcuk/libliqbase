/**
 * @file	liqlist.c
 * @author  Gary Birkett
 * @brief 	simple listbox component
 * 
 * Copyright (C) 2008 Gary Birkett
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <string.h>
#include <stdlib.h>


#include "liqui.h"



// this file is part of liqbase by Gary Birkett
		
#include "liqbase.h"
#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqcell_easyhandler.h"
		
int liqlist_clear(liqcell *liqlist)
{
	liqcell *backplane = liqcell_child_lookup(liqlist, "backplane");
    liqcell_child_removeallvisual(backplane);
    return 0;
}
        
        
int liqlist_additem(liqcell *liqlist,char *item)
{
    //liqcell *listitemtemplate = liqcell_child_lookup(liqlist, "listitemtemplate");
	liqcell *backplane = liqcell_child_lookup(liqlist, "backplane");
    
    //############################# listitemtemplate:label
    liqcell *listitemtemplate = liqcell_quickcreatevis("listitemtemplate", "label", 0, 0, 800, 50);
    liqcell_setfont(	listitemtemplate, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (32), 0) );
    liqcell_setcaption(listitemtemplate, item );
    liqcell_propsets(  listitemtemplate, "textcolor", "rgb(255,255,255)" );
    //liqcell_propsets(  listitemtemplate, "backcolor", "rgb(235,233,237)" );
    liqcell_propseti(  listitemtemplate, "textalign", 0 );
    //liqcell_setvisible(listitemtemplate,0);
    
    liqcell_child_selectnone(backplane);
    liqcell_child_append(  backplane, listitemtemplate);		
    liqcell_setselected(listitemtemplate,1);
    liqcell_child_arrange_easycol(backplane);
    liqcell_ensurevisible(listitemtemplate);
}

int liqlist_setindex(liqcell *liqlist,int index)
{
	liqcell *backplane = liqcell_child_lookup(liqlist, "backplane");
    liqcell_child_selectnone(backplane);
    int upto=0;
    liqcell *c=liqcell_getlinkchild_visual(backplane);
    while(c && upto!=index)
    {
        c=liqcell_getlinknext_visual(c);
        upto++;
    }
    if(c)
    {
        
        liqcell_setselected(c,1);
        liqcell_ensurevisible(c);
        return index;
    }
    return -1;
}

int liqlist_getindex(liqcell *liqlist)
{
	liqcell *backplane = liqcell_child_lookup(liqlist, "backplane");
    liqcell_child_selectnone(backplane);
    int upto=0;
    liqcell *c=liqcell_getlinkchild_visual(backplane);
    while(c)
    {
        if(liqcell_getselected(c)) break;
        c=liqcell_getlinknext_visual(c);
        upto++;
    }
    if(c)
    {
        return upto;
    }
    return -1;
}


int liqlist_count(liqcell *liqlist)
{
	liqcell *backplane = liqcell_child_lookup(liqlist, "backplane");
	return liqcell_child_countvisible(backplane);
}


//#####################################################################
//#####################################################################
//##################################################################### liqlist :: by gary birkett
//#####################################################################
//#####################################################################
		
		
/**	
 * liqlist widget refresh, all params set, present yourself to the user.
 */	
static int liqlist_refresh(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	// there might be an OS level variable called filter
	// it should be set and adjusted correctly prior to calling this routine
	// you should do your best to account for this filter in any way you see fit
	return 0;
}
/**	
 * liqlist dialog_open - the user zoomed into the dialog
 */	
static int liqlist_dialog_open(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * liqlist dialog_close - the dialog was closed
 */	
static int liqlist_dialog_close(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * liqlist widget shown - occurs once per lifetime
 */	
static int liqlist_shown(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	return 0;
}
/**	
 * liqlist mouse - occurs all the time as you stroke the screen
 */	
static int liqlist_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * liqlist click - occurs when a short mouse stroke occured
 */	
static int liqlist_click(liqcell *self, liqcelleventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * liqlist keypress - the user pressed a key
 */	
static int liqlist_keypress(liqcell *self, liqcellkeyeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * liqlist keyrelease - the user released a key
 */	
static int liqlist_keyrelease(liqcell *self, liqcellkeyeventargs *args,liqcell *context)
{
	return 0;
}
/**	
 * liqlist paint - being rendered.  use the vgraph held in args to do custom drawing at scale
 */	
//static int liqlist_paint(liqcell *self, liqcellpainteventargs *args,liqcell *context)
//{
//	// big heavy event, use sparingly
//	return 0;
//}
/**	
 * liqlist dynamic resizing
 */	
static int liqlist_resize(liqcell *self,liqcelleventargs *args, liqcell *context)
{
	float sx=((float)self->w)/((float)self->innerw);
	float sy=((float)self->h)/((float)self->innerh);
	
    liqcell *listitemtemplate = liqcell_child_lookup(self, "listitem");
	liqcell *backplane = liqcell_child_lookup(self, "backplane");
	liqcell_setrect_autoscale( backplane, 0,0, 800,480, sx,sy);
	liqcell_setrect_autoscale( listitemtemplate, 0,0, 800,50, sx,sy);
	return 0;
}

/**	
 * liqlist_child_test_seek this function shows how to access members
 */	
	  
static void liqlist_child_test_seek(liqcell *self)
{	  
    liqcell *listitemtemplate = liqcell_child_lookup(self, "listitemtemplate");
	liqcell *backplane = liqcell_child_lookup(self, "backplane");
}	  
/**	
 * create a new liqlist widget
 */	
liqcell *liqlist_create()
{
	liqcell *self = liqcell_quickcreatewidget("liqlist", "form", 800, 480);
	if(!self) {liqapp_log("liqcell error not create 'liqlist'"); return NULL;  } 
	
	// Optimization:  The aim is to REDUCE the number of drawn layers and operations called.
	// Optimization:  use only what you NEED to get an effect
	// Optimization:  Minimal layers and complexity
	// Optimization:  defaults: background, prefer nothing, will be shown through if there is a wallpaper
	// Optimization:  defaults: text, white, very fast rendering

    //############################# listitemtemplate:label
    liqcell *listitemtemplate = liqcell_quickcreatevis("listitemtemplate", "label", 0, 0, 800, 50);
    liqcell_setfont(	listitemtemplate, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (32), 0) );
    liqcell_setcaption(listitemtemplate, "listitem" );
    liqcell_propsets(  listitemtemplate, "textcolor", "rgb(255,255,255)" );
    //liqcell_propsets(  listitemtemplate, "backcolor", "rgb(235,233,237)" );
    liqcell_propseti(  listitemtemplate, "textalign", 0 );
    liqcell_setvisible(listitemtemplate,0);
    liqcell_child_append(  self, listitemtemplate);
    

	//############################# backplane:picturebox
	liqcell *backplane = liqcell_quickcreatevis("backplane", "picturebox", 0, 0, 800, 480);
	//liqcell_setfont(	backplane, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (12), 0) );
	//liqcell_propsets(  backplane, "textcolor", "rgb(0,0,0)" );
	//liqcell_propsets(  backplane, "backcolor", "rgb(64,64,64)" );
    liqcell_handleradd_withcontext(backplane,    "mouse",   liqcell_easyhandler_kinetic_mouse,self);
	liqcell_child_append(  self, backplane);
    
	//liqcell_propsets(  self, "backcolor", "rgb(0,0,0)" );
   // liqcell_propsets(  self, "bordercolor", "rgb(255,255,255)" );
	liqcell_handleradd_withcontext(self, "refresh", liqlist_refresh ,self);
	liqcell_handleradd_withcontext(self, "shown", liqlist_shown ,self);
	liqcell_handleradd_withcontext(self, "resize", liqlist_resize ,self);
	liqcell_handleradd_withcontext(self, "keypress", liqlist_keypress,self );
	liqcell_handleradd_withcontext(self, "keyrelease", liqlist_keyrelease ,self);
	liqcell_handleradd_withcontext(self, "mouse", liqlist_mouse,self );
	liqcell_handleradd_withcontext(self, "click", liqlist_click ,self);
	//liqcell_handleradd_withcontext(self, "paint", liqlist_paint ,self); // use only if required, heavyweight
	liqcell_handleradd_withcontext(self, "dialog_open", liqlist_dialog_open ,self);
	liqcell_handleradd_withcontext(self, "dialog_close", liqlist_dialog_close ,self);
	return self;
}

