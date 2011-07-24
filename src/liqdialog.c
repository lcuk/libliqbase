/* liqbase
 * Copyright (C) 2008 Gary Birkett
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
 */

/*
 *
 * quick dialog and menu screens
 *
 */




#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>



#include "liqbase.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"


#include "liqcell.h"
#include "liqcell_easypaint.h"
#include "liqapp_prefs.h"
#include "liqcell_historystore.h"
#include "liqdialog.h"



// header is fontbig.height + canvas.pixelheight*0.1


int liqdialog_showoption(const char *question,int defaultanswer,int answercount, const char ** answers)
{
	return liqdialog_askv(NULL,question,NULL,NULL,defaultanswer,answercount,answers);
}



int liqdialog_showyesno(const char *question)
{	
	int todo=0;
	
		//
		{
			const char *options[]={"Yes","No",0};
			todo=liqdialog_showoption(question,-1,2,&options[0]);
		}
		switch(todo)
		{
			case -1:
				return -1;
			case 0:
				return 0;
				break;
			case 1:
				return -1;
				break;
		}
		
	return -1;
}


//###########################################################################
//###########################################################################
//###########################################################################


//###########################################################################
//###########################################################################
//###########################################################################
int liqdialog_ask(const char *key,const char *question,const char *description,const char *footingnotes,int defaultoption,const char *option1, ...)
{
	//todo: make this ask() function call showoption passing all the parameters
	//todo: once it works, expand name put it in dialog class
	//description comes from key, should be base configurable here though
	//icon comes from key
	//char *options[10];
	//int result=liqdialog_showoption(question,0,&options[0]);
	//..
	const char *answerarray[20];
	int answercount=0;
	if(!option1)
	{
		// nothing to ask...
		return -1;
	}
	answerarray[0] = option1;
	answercount=1;
	va_list arg;
	va_start(arg, option1);
	
	char *a;
	for(answercount=1;answercount<19;answercount++)
	{
		a = va_arg (arg, char *);
		if(a==NULL)
		{
			// reached the end
			break;
		}
		answerarray[answercount] = a;
	}
	answerarray[answercount] = NULL;
	
	va_end(arg);
	
	return liqdialog_askv(key,question,description,footingnotes,defaultoption,0,answerarray);
}




//##################################################### dialogbutton :: click handler
static int dialogbutton_click(liqcell *self, liqcellclickeventargs *args, liqcell *widget)
{
	liqapp_log("dialogbutton click");
	return 0;
}


//##################################################### dialogbutton :: mouse handler
static int dialogbutton_mouse(liqcell *self, liqcellmouseeventargs *args, liqcell *widget)
{
	liqapp_log("dialogbutton mouse");
	return 0;
}




int liqdialog_askv(const char *key,const char *question,const char *description,const char *footingnotes,int defaultoption,int answercount,const char ** answers)
{
int result;
	
	if(answercount==0)
	{
		const char **a=answers;
		while(*a++) 
		{
			if(answercount>20)break;		// for now, lets not get stuck...
			answercount++;
		}
	}
	
	if(!footingnotes)footingnotes="contact liquid@gmail.com, lcuk on #maemo";
	
	liqapp_log("Running Dialog with %i options",answercount);






	//##################################################### 
	liqcell *self = liqcell_quickcreatewidget("testcode","form", 800,480);
	if(!self)
	{
		liqapp_log("could not allocate 'testcode'");
		return 0;
	}
	liqcell *b = NULL;
	liqcell *c = NULL;


	

	//##################################################### label1 :: label
	//# This is the main widget constructor for this node
	b = liqcell_quickcreatevis("question","label",  	0,0,  800,112 );
	    liqcell_setcaption(b,       question);
	    liqcell_setfont(   b, 	liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (26), 0) );
	    liqcell_propsets(  b,    	"bordercolor",	"rgb(100,100,100)" );
	    liqcell_propsets(  b,    	"textcolor",		"rgb(255,255,255)" );
	    liqcell_propseti(  b,    	"textalign",		0 );
	    liqcell_propseti(  b,    	"textaligny",		2 );
	    //liqcell_handleradd_withcontext(b,"click",         	(void*)label1_click, self);
	    //liqcell_handleradd_withcontext(b,"mouse",         	(void*)label1_mouse, self);
	liqcell_child_append( self, b );

	//##################################################### panel1 :: panel
	//# This is the main widget constructor for this panel
	b = liqcell_quickcreatevis("panel1","panel",  	0,112,  800,480-112-56 );
	    liqcell_propsets(  b,    	"bordercolor",	"rgb(100,100,100)" );



		//##################################################### 
		int a;
		for(a=0;a<answercount;a++)
		{

			//############################# toolname:button
			liqcell *bt = liqcell_quickcreatevis(answers[a], "button", 0,0, 100, 56);
			liqcell_setfont(	bt, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (32), 0) );
			liqcell_setcaption(bt, answers[a] );
			liqcell_propsets(  bt, "textcolor",   "rgb(255,255,255)" );
			if(a==defaultoption)
				liqcell_propsets(  bt, "backcolor",   "xrgb(50,160,50)");
			else
				liqcell_propsets(  bt, "backcolor",   "xrgb(50,100,50)");
			liqcell_propsets(  bt, "bordercolor", "rgb(255,255,255)" );
			liqcell_propseti(  bt, "textalign",   2 );
			liqcell_propseti(  bt, "textaligny",  2 );
			
			liqcell_handleradd_withcontext(bt,    "click",   (void*)dialogbutton_click, self);
			liqcell_handleradd_withcontext(bt,    "mouse",   (void*)dialogbutton_mouse, self);
			

			liqcell_child_append( b, bt );
		}

		liqcell_child_arrange_easytile( b );
		
	liqcell_child_append( self, b );

	//##################################################### label2 :: label
	//# This is the main widget constructor for this node
	b = liqcell_quickcreatevis("footer","label",  	0,424,  800,56 );
	    liqcell_setcaption(b,       footingnotes);
	    liqcell_setfont(   b, 	liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (26), 0) );
	    liqcell_propsets(  b,    	"bordercolor",	"rgb(100,100,100)" );
	    liqcell_propsets(  b,    	"textcolor",		"rgb(255,255,255)" );
	    liqcell_propseti(  b,    	"textalign",		2 );
	    liqcell_propseti(  b,    	"textaligny",		2 );
	    //liqcell_handleradd_withcontext(b,"click",         	(void*)label2_click, self);
	    //liqcell_handleradd_withcontext(b,"mouse",         	(void*)label2_mouse, self);
	liqcell_child_append( self, b );






	
	liqcell_propseti(self,"dialog_zoomed",1  );
	liqcell_easyrun(self);
	liqcell_release(self);	
	return result;
}



