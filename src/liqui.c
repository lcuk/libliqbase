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



#define XK_MISCELLANY

#include "X11/keysymdef.h"


char *text_temp_paste = NULL;


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


			liqcell *clock = liqcell_quickcreatevis("clock",   "time",   600,0,   200,100 );
			//char buf[80];
			//liqapp_format_strftime(buf,80,"%H:%M:%S");
			//liqcell_setcaption(   clock, buf);
			liqcell_propsets(     clock,"timeformat","%H:%M:%S");
			liqcell_propseti(     clock,"textalign",2);
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
	
	liqcell *self = liqcell_quickcreatewidget("uiinfobar","section", 800,100);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
	
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,10,   780,80);
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
	
	liqcell *self = liqcell_quickcreatewidget("uitextbox","uitextbox", 800,50);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,0,   200,40);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(255,255,255)" );
			liqcell_setcaption(body,  caption );
		liqcell_child_insert( self, body );
		
		
			liqcell *data = liqcell_quickcreatevis("data","textbox", 210,0,   580,40);
			liqcell_setfont(   data,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (36), 0) );
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
	
	liqcell *self = liqcell_quickcreatewidget("uinumberbox","section", 800,50);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,0,   200,40);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(255,255,255)" );
			liqcell_setcaption(body,  caption );
		liqcell_child_insert( self, body );
		
		
			liqcell *data = liqcell_quickcreatevis("data","frame", 210,0,   580,40);
			liqcell_setfont(   data,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (36), 0) );
			liqcell_propsets(  data,  "backcolor", "rgb(220,220,255)" );
			liqcell_propsets(  data,  "textcolor", "rgb(0,0,0)" );
			liqcell_setcaption(data,  datadefault );
		
		
		liqcell_child_insert( self, data );
	}
	return self;
}



/**
 * Create a picturebox ui component
 * @param caption The caption
 * @param datafefault Data caption
 * @return liqcell* The new picturebox
 */
liqcell *uipicturebox_create(char *caption,char *datadefault)
{
	
	liqcell *self = liqcell_quickcreatewidget("uipicturebox","section", 800,150);
	
	if(self)
	{

		liqcell_propsets(  self,  "backcolor", "rgb(0,0,0)" );
		
			liqcell *body = liqcell_quickcreatevis("body","frame", 10,0,   200,40);
			liqcell_setfont(   body,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
			liqcell_propsets(  body,  "backcolor", "rgb(40,40,40)" );
			liqcell_propsets(  body,  "textcolor", "rgb(255,255,255)" );
			liqcell_setcaption(body,  caption );
		liqcell_child_insert( self, body );	
		
			liqcell *data = liqcell_quickcreatevis("data","frame", 210,0,   140,140);
			liqcell_setimage(  data,  liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/lcuk_avatar.jpg",0,0,0) );


		liqcell_child_insert( self, data );
	}
	return self;
}





	static int widget_mouse(liqcell *self, liqcellmouseeventargs *args)
	{
		liqcell_adjustpos(self,0,args->mdy);
		return 0;
	}





liqcell *liqui_create()
{
	
	liqcell *self = liqcell_quickcreatewidget("liqui","form", 800,480);
	
	if(self)
	{
			liqcell *body = liqcell_quickcreatewidget("body","frame", 800,480);
			
			liqcell_child_append( body,  uititlebar_create(   "ui", "User Interface test1", "The very first UI example I've tried" ) );	
			liqcell_child_append( body,  uiinfobar_create(    "this is a user interface test for the new liqbase") );
			liqcell_child_append( body,  uitextbox_create(    "Nickname", "lcuk" ) );
			liqcell_child_append( body,  uitextbox_create(    "Full Name", "Gary Birkett" ) );
			liqcell_child_append( body,  uitextbox_create(    "Email", "liquid@gmail.com" ) );
			liqcell_child_append( body,  uinumberbox_create(  "Karma Bonus", "35" ) );
			liqcell_child_append( body,  uitextbox_create(    "Karma Rating", "Excellent" ) );
			liqcell_child_append( body,  uitextbox_create(    "homepage", "http://liqbase.net" ) );
			liqcell_child_append( body,  uitextbox_create(    "gender", "male" ) );
			liqcell_child_append( body,  uipicturebox_create( "avatar", "smile" ) );
			//liqcell_child_append( body,  uibuttonstrip_create("options", "help,cancel,save" ) );
			
			liqcell_child_arrange_autoflow(body);
			
			liqcell_handleradd(body,    "mouse",   widget_mouse);
		
		liqcell_child_append( self, body );

		
	}


	return self;
}
