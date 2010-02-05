/**
 * @file	liqui.c
 * @author  Gary Birkett
 * @brief 	Libliqbase User Interface Functions
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

#include "liqcell_easyhandler.h"

#define XK_MISCELLANY

#include "X11/keysymdef.h"

// this should not be here
//char *text_temp_paste = NULL;


//#####################################################################
//#####################################################################
//##################################################################### liqui :: by gary birkett 
//#####################################################################
//#####################################################################

/**
 * Create a titlebar ui component
 * @param key Name of the widget
 * @param title Title to display
 * @param description Describe what this titlebar is for
 * @return liqcell* The new titlebar
 */
liqcell *uititlebar_create(char *key,char *title,char *description)
{
	
	liqcell *self = liqcell_quickcreatewidget(key,"section", 800,100);
	
	if(self)
	{
		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
		//texturestrip_blu.jpg
		liqcell_setimage(  self,  liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/texturestrip_dark.jpg",0,0,0) );
		
		liqcell_child_append( self, liqcell_quickcreatevis("app_icon",   "icon",    5   ,10  ,    90, 80 )    );
		liqcell_child_append( self, liqcell_quickcreatevis("app_title",  "label",   100 ,0  ,   700, 55 )    );
		liqcell_child_append( self, liqcell_quickcreatevis("app_desc",   "label",   100 ,55 ,   700, 40 )    );

		liqcell_setimage(  liqcell_child_lookup( self,"app_icon"),  liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/sun.png",0,0,1) );
		liqcell_setfont(   liqcell_child_lookup( self,"app_title"), liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (40), 0) );
		liqcell_setfont(   liqcell_child_lookup( self,"app_desc"),  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (18), 0) );


		liqcell_propsets(  liqcell_child_lookup( self,"app_title"), "textcolor", "rgb(255,255,255)" );
		liqcell_propsets(  liqcell_child_lookup( self,"app_desc"),  "textcolor", "rgb(0,100,0)" );

		liqcell_setcaption(liqcell_child_lookup( self,"app_title"), title );
		liqcell_setcaption(liqcell_child_lookup( self,"app_desc"), description );


			liqcell *clock = liqcell_quickcreatevis("clock",   "time",   500,0,   200,100 );
			//char buf[80];
			//liqapp_format_strftime(buf,80,"%H:%M:%S");
			//liqcell_setcaption(   clock, buf);
			liqcell_propsets(     clock,"timeformat","%H:%M:%S");
			liqcell_propseti(     clock,"textalign",2);
			liqcell_propseti(     clock,"textaligny",2);
			liqcell_propsets(     clock,"fontname", "/usr/share/fonts/nokia/nosnb.ttf" );
			liqcell_propseti(     clock,"fontsize", 32 );
			liqcell_propsets(     clock, "textcolor", "rgb(255,255,255)" );
		//	liqcell_handleradd(self,    "mouse",   widget_mouse);
			liqcell_child_append( self, clock    );

	}
	return self;
}











	

/**
 * Create a infobar ui component
 * @param infotext Caption of the bar
 * @return liqcell* The new infobar
 */
liqcell *uiinfobar_create(char *infotext)
{
	
	liqcell *self = liqcell_quickcreatewidget("uiinfobar","section", 800,80);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
	
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,10,   780,60);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (36), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(220,220,255)" );
			liqcell_setcaption(body,  infotext );
			
			
			//liqcell_propseti(  body,  "selstart",  5 );
			//liqcell_propseti(  body,  "sellength", 12 );
			//liqcell_propseti(  body,  "cursorpos", 17 );
			
			//liqcell_handleradd(body,    "mouse",      textbox_mouse);
			//liqcell_handleradd(body,    "keypress",   textbox_keypress);
			//liqcell_handleradd(body,    "keyrelease", textbox_keyrelease);
			
			
		
		
		liqcell_child_insert( self, body );
	}
	return self;
}

/**
 * Create a textbox ui component, this is the development function
 * @param caption Body Caption
 * @param datadefault Caption of the data
 * @return liqcell* The new textbox
 */
liqcell *uitextbox_create(char *caption,char *datadefault)
{
	
	liqcell *self = liqcell_quickcreatewidget("uitextbox","uitextbox", 800,80);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,10,   200,60);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(255,255,255)" );
			liqcell_setcaption(body,  caption );
		liqcell_child_insert( self, body );
		
		
			liqcell *data = liqcell_quickcreatevis("data","textbox", 220,5,   560,70);
			liqcell_setfont(   data,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (42), 0) );
			liqcell_propsets(  data,  "backcolor", "rgb(220,255,220)" );
			liqcell_propsets(  data,  "textcolor", "rgb(0,0,0)" );
			liqcell_setcaption(data,  datadefault );
		liqcell_child_insert( self, data );



			//liqcell_propseti(  data,  "selstart",  5 );
			//liqcell_propseti(  data,  "sellength", 12 );
			//liqcell_propseti(  data,  "cursorpos", 17 );
			
		//	liqcell_handleradd(data,    "mouse",      textbox_mouse);
		//	liqcell_handleradd(data,    "keypress",   textbox_keypress);
		//	liqcell_handleradd(data,    "keyrelease", textbox_keyrelease);



//liqcell *liqkeyboard_create();

	//		liqcell *kb = liqcell_quickcreatevis("data","liqkeyboard", 500,0,   60,20);
			//liqcell_setfont(   kb,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (36), 0) );
			//liqcell_propsets(  kb,  "backcolor", "rgb(220,255,220)" );
			//liqcell_propsets(  kb,  "textcolor", "rgb(0,0,0)" );
			//liqcell_setcaption(kb,  datadefault );

	//	liqcell_child_insert( data, kb );



		
		
	}
	return self;
}


/**
 * Create a numberbox ui component
 * @param caption The caption
 * @param datafefault Data caption
 * @return liqcell* The new numberbox
 */
liqcell *uinumberbox_create(char *caption,char *datadefault)
{
	
	liqcell *self = liqcell_quickcreatewidget("uinumberbox","section", 800,80);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,10,   200,60);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(255,255,255)" );
			liqcell_setcaption(body,  caption );
		liqcell_child_insert( self, body );
		
		
			liqcell *data = liqcell_quickcreatevis("data","frame", 220,5,   560,70);
			liqcell_setfont(   data,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (42), 0) );
			liqcell_propsets(  data,  "backcolor", "rgb(220,220,255)" );
			liqcell_propsets(  data,  "textcolor", "rgb(0,0,0)" );
			liqcell_setcaption(data,  datadefault );
		
		
		liqcell_child_insert( self, data );
	}
	return self;
}

/**	
 * uienumbox_node_setformat
 * set the display formatting for this item - whether selected or not
 */	
static int uienumbox_node_setformat(liqcell *node,int isselected)
{

		if(isselected)
		{
			liqcell_setfont(   node,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (32), 0) );
			liqcell_propsets(  node,  "backcolor", "rgb(220,220,255)" );
			liqcell_propsets(  node,  "textcolor", "rgb(0,0,0)" );
			liqcell_propsets(  node,  "bordercolor", "rgb(255,255,255)" );
			liqcell_setselected(node, 1);
		}
		else
		{
			liqcell_setfont(   node,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  node,  "backcolor", "rgb(200,200,205)" );
			liqcell_propsets(  node,  "textcolor", "rgb(30,30,30)" );
			liqcell_propsets(  node,  "bordercolor", "rgb(128,128,128)" );
			liqcell_setselected(node, 0);					
		}

	return 0;
}

/**	
 * uienumbox node clicked
 */	
static int uienumbox_node_click(liqcell *self,liqcellclickeventargs *args, liqcell *uienumbox)
{
	//liqcell *body = liqcell_child_lookup(uienumbox, "body");
	liqcell *data = liqcell_child_lookup(uienumbox, "data");
		
	liqcell *node = liqcell_getlinkchild_visual(data);
	while(node)
	{
		if(node==self)
		{
			uienumbox_node_setformat(node,1);
		}
		else
		{
			uienumbox_node_setformat(node,0);				
		}
		node=liqcell_getlinknext_visual(node);
	}
	return 0;
}


/**
 * Create an enumbox ui component
 * @param caption The caption
 * @param datafefault the default to select
 * @param choices semicolon delim list of items (only a short qty the paradigm is only for simple choices)
 * @return liqcell* The new enumbox
 */
liqcell *uienumbox_create(char *caption,char *datadefault,char *choices)
{
	
	liqcell *self = liqcell_quickcreatewidget("uienumbox","section", 800,80);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,10,   200,60);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(255,255,255)" );
			liqcell_setcaption(body,  caption );
		liqcell_child_insert( self, body );
		
		
			liqcell *data = liqcell_quickcreatevis("data","frame", 220,5,   560,70);
			//liqcell_setfont(   data,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (42), 0) );
			liqcell_propsets(  data,  "backcolor", "rgb(255,255,255)" );
			//liqcell_propsets(  data,  "textcolor", "rgb(0,0,0)" );
			liqcell_setcaption(data,  datadefault );


			char *allchoices = strdup(choices);
			char *onechoice;
			onechoice = strtok(allchoices, ";");
			while( onechoice )
			{
				//
				liqcell *node = liqcell_quickcreatevis(onechoice,"frame", 220,5,   560,70);
				if(datadefault && (strcmp(onechoice,datadefault)==0) )
				{
					uienumbox_node_setformat(node,1);
				}
				else
				{
					uienumbox_node_setformat(node,0);				
				}
				liqcell_propseti(     node,"textalign",2);
				liqcell_propseti(     node,"textaligny",2);
				liqcell_setcaption(node,  onechoice );
				liqcell_handleradd_withcontext(node, "click", uienumbox_node_click, self );
				liqcell_child_append( data, node );
				
				onechoice = strtok(NULL, ";");
				
			}
			free(allchoices);
			
			liqcell_child_arrange_makegrid(data, liqcell_child_countvisible(data)  , 1);



		
		
		liqcell_child_insert( self, data );
	}
	return self;
}

/**	
 * uipicturebox clicked
 */	
static int uipicturebox_click(liqcell *self,liqcellclickeventargs *args, liqcell *uipicturebox)
{
	liqcell *dialog = liqcell_quickcreatevis("dialog1", "dialog_selectimage", 0,0, -1,-1);
	liqcell_easyrun(dialog);
	char *sel=liqcell_propgets(  dialog, "imagefilenameselected",NULL );
	if( sel && *sel )
	{	
		// do whatever is needed
		liqcell_setimage(  self,  NULL );
		liqcell_propsets(  self,  "imagefilename", sel );
		liqcell_handlerrun(uipicturebox,"refresh",NULL);
	}
	liqcell_release(dialog);
	return 0;
}

/**
 * Create a picturebox ui component
 * @param caption The caption
 * @param datafefault Data caption
 * @return liqcell* The new picturebox
 */
liqcell *uipicturebox_create(char *caption,char *datadefaultimagefilename)
{
	
	liqcell *self = liqcell_quickcreatewidget("uipicturebox","section", 800,150);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,10,   200,130);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(255,255,255)" );
			liqcell_setcaption(body,  caption );
		liqcell_child_insert( self, body );	
		
			liqcell *data = liqcell_quickcreatevis("data","frame", 220,5,   560,140);
			liqcell_setimage(  data,  liqimage_cache_getfile( datadefaultimagefilename ,0,0,0) );
			
			
		liqcell_handleradd_withcontext(data, "click", uipicturebox_click, self );


		liqcell_child_insert( self, data );
	}
	return self;
}


/**	
 * uicolorbox clicked
 */	
static int uicolorbox_click(liqcell *self,liqcellclickeventargs *args, liqcell *uicolorbox)
{
	liqcell *dialog = liqcell_quickcreatevis("dialog1", "dialog_selectcolor", 0,0, -1,-1);
	
	liqcell_propsets(  dialog,  "colorselected", liqcell_propgets(  self, "backcolor", NULL )  );
	
	liqcell_easyrun(dialog);
	char *sel=liqcell_propgets(  dialog, "colorselected",NULL );
	if( sel && *sel )
	{	
		// do whatever is needed
		liqcell_propsets(  self,  "backcolor", sel );
	}
	liqcell_release(dialog);
	return 0;
}

/**
 * Create a colorbox ui component
 * @param caption The caption
 * @param datafefault Data caption
 * @return liqcell* The new colorbox
 */
liqcell *uicolorbox_create(char *caption,char *datadefault)
{
	
	liqcell *self = liqcell_quickcreatewidget("uicolorbox","section", 800,150);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,10,   200,130);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(255,255,255)" );
			liqcell_setcaption(body,  caption );
		liqcell_child_insert( self, body );	
		
			liqcell *data = liqcell_quickcreatevis("data","frame", 270,5,   460,140);
			liqcell_propsets(  data,  "backcolor", datadefault );
			
			
		liqcell_handleradd_withcontext(data, "click", uicolorbox_click, self );


		liqcell_child_insert( self, data );
	}
	return self;
}








/**	
 * liqui_cmdaccept clicked
 */	
static int liqui_cmdaccept_click(liqcell *self,liqcellclickeventargs *args, liqcell *liqui)
{

    liqcell_propseti(liqui,"dialog_running",0);

	return 0;
}


liqcell *liqui_create()
{
	
	liqcell *self = liqcell_quickcreatewidget("liqui","form", 800,480);
	
	if(self)
	{


			liqcell *body = liqcell_quickcreatewidget("body","frame", 800,480);
			
			liqcell_child_append( body,  uititlebar_create(   "ui", "User Interface test1", "The very first UI example I've tried" ) );	
			liqcell_child_append( body,  uiinfobar_create(    "this is a user interface test") );
			liqcell_child_append( body,  uitextbox_create(    "nickname", "lcuk" ) );
			liqcell_child_append( body,  uicolorbox_create(   "favoritecolor", "rgb(50,150,50)" ) );
			liqcell_child_append( body,  uienumbox_create(    "level", "High", "Low;Medium;High;Godlike" ) );
			liqcell_child_append( body,  uitextbox_create(    "fullname", "Gary Birkett" ) );
			liqcell_child_append( body,  uitextbox_create(    "email", "liquid@gmail.com" ) );
			liqcell_child_append( body,  uipicturebox_create( "avatar", "/usr/share/liqbase/libliqbase/media/lcuk_avatar.jpg" ) );
			liqcell_child_append( body,  uinumberbox_create(  "karmabonus", "35" ) );
			liqcell_child_append( body,  uitextbox_create(    "karmarating", "Excellent" ) );
			liqcell_child_append( body,  uitextbox_create(    "homepage", "http://liqbase.net" ) );
			liqcell_child_append( body,  uitextbox_create(    "gender", "male" ) );
			//liqcell_child_append( body,  uibuttonstrip_create("options", "help,cancel,save" ) );
		
			liqcell_child_append( body,  uitextbox_create(    "title", "Fluid motion" ) );
			liqcell_child_append( body,  uitextbox_create(    "author", "Gary Birkett" ) );
			liqcell_child_append( body,  uitextbox_create(    "email", "liquid@gmail.com" ) );
			
			liqcell_child_append( body,  uicolorbox_create(   "pencolor", "rgb(255,255,0)" ) );
			liqcell_child_append( body,  uienumbox_create(    "pentrail", "Short", "Off;Short;Medium;Long" ) );
			
			liqcell_child_append( body,  uienumbox_create(    "backstyle", "Colored", "Blank;Colored;Textured" ) );
			liqcell_child_append( body,  uicolorbox_create(   "backcolor", "rgb(0,30,0)" ) );
			liqcell_child_append( body,  uipicturebox_create( "backimage", "/usr/share/liqbase/libliqbase/media/lcuk_avatar.jpg" ) );


			liqcell_child_append( body,  uienumbox_create(    "starcount", "200", "50;100;150;200;300;400;500" ) );
			liqcell_child_append( body,  uienumbox_create(    "starsize", "Medium", "Small;Medium;Large" ) );
			liqcell_child_append( body,  uipicturebox_create( "starimage", "/usr/share/liqbase/libliqbase/media/lcuk_avatar.jpg" ) );
			
			liqcell_child_append( body,  uienumbox_create(    "gravity", "Medium", "None;Light;Medium;Strong" ) );
				
			liqcell_child_arrange_autoflow(body);
			
			body->h += 60 + 10;	// make sure it can extend enough to fit the accept button
			
			liqcell_handleradd(body,    "mouse",   liqcell_easyhandler_kinetic_mouse);
			
		
		liqcell_child_append( self, body );


		//############################# cmdaccept:label
		liqcell *cmdaccept = liqcell_quickcreatevis("cmdaccept", "label", 580, 420, 210, 60);
		liqcell_setfont(	cmdaccept, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
		liqcell_setcaption(cmdaccept, "Save" );
		liqcell_propsets(  cmdaccept, "textcolor", "rgb(255,255,255)" );
		liqcell_propsets(  cmdaccept, "backcolor", "xrgb(0,64,0)" );
		liqcell_propsets(  cmdaccept, "bordercolor", "rgb(255,255,255)" );
		liqcell_propseti(  cmdaccept, "textalign", 2 );
		liqcell_propseti(  cmdaccept, "textaligny", 2 );
		liqcell_handleradd_withcontext(cmdaccept, "click", liqui_cmdaccept_click, self );
		liqcell_child_append(  self, cmdaccept);
		
		
	}


	return self;
}
