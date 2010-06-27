/**
 * @file	liqkeyboard.c
 * @author  Gary Birkett, Zach Habersang
 * @brief 	Liqbase Virtual Keyboard
 *
 * Copyright (C) 2008 Zach Habersang
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

#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks
#include <stdlib.h>

#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"

#ifdef __cplusplus
extern "C" {
#endif

liqcell *build_vkbd();




//######################################################################
//######################################################################
//######################################################################
static liqcell *mkframe(liqcell *self,char *title,int w,int h)
{
	liqcell *ch= liqcell_quickcreatevis(title,NULL,0,0,w,h);
	//liqcell_pageautoloadbytitle_apg(ch);
	liqcell_child_append(self,ch);
	return ch;
}


static liqcell *mkkey(liqcell *self,char *title,int w,int h)
{
	liqcell *ch= liqcell_quickcreatevis(title,"keycap",0,0,w,h);
	//liqcell_pageautoloadbytitle_apg(ch);
	liqcell_child_append(self,ch);
	return ch;
}


//######################################################################
//######################################################################
//######################################################################

/**	
 * liqkeyboard keypress - the user faked pressing a key
 */	
static int liqkeyboard_keypress(liqcell *self, liqcellkeyeventargs *args,liqcell *keyboard)
{
	liqcell *textbox = liqcell_child_lookup(keyboard, "liqkeyboard_textbox");
	//liqcell *buttondel = liqcell_child_lookup(dialogusername, "buttondel");
	//liqcell *buttonaccept = liqcell_child_lookup(dialogusername, "buttonaccept");	
	liqcell_handlerrun(textbox, "keypress", args);
	
	return 0;
}
/**	
 * liqkeyboard keyrelease - the user faked released a key
 */	
static int liqkeyboard_keyrelease(liqcell *self, liqcellkeyeventargs *args,liqcell *keyboard)
{
	liqcell *textbox = liqcell_child_lookup(keyboard, "liqkeyboard_textbox");
	//liqcell *buttondel = liqcell_child_lookup(dialogusername, "buttondel");
	//liqcell *buttonaccept = liqcell_child_lookup(dialogusername, "buttonaccept");	
	liqcell_handlerrun(textbox, "keyrelease", args);
	
	return 0;
}	


static int key_click(liqcell *self, liqcelleventargs *args,liqcell *keyboard)
{
	// 20090624_005355 lcuk : key was clicked, raise a keyboard event and send the click letter itself through as eventargs
	liqapp_log("liqkeyboard: key click");
	liqcellkeyeventargs keyargs={0};
		
	keyargs.keycode = (int)liqcell_gettag(self);
	snprintf(keyargs.keystring,sizeof(keyargs.keystring),liqcell_getcaption(self));
	keyargs.ispress = 1;
	liqcell_handlerrun(keyboard,"keypress",&keyargs);
		
	keyargs.ispress = 0;
	liqcell_handlerrun(keyboard,"keyrelease",&keyargs);
	return 0;
}

/*
static int key_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *keyboard)
{
	// whilst the button is pressed, the button should change color		
	if(args->mez!=0)
	{
		liqcell_propsets(  self, "textcolor", "rgb(0,0,0)" );
		liqcell_propsets(  self, "backcolor", "rgb(150,100,150)" );
			
	}
	else
	{
		liqcell_propsets(  self, "textcolor", "rgb(255,255,255)" );
		liqcell_propsets(  self, "backcolor", "rgb(100,150,100)" );
			
	}
	return 0;
}
 */
static int enter_click(liqcell *self,liqcelleventargs *args, liqcell *keyboard)
{
	// get local textbox caption
	liqcell *local_textbox = liqcell_child_lookup(keyboard, "liqkeyboard_textbox");
	char *caption = liqcell_getcaption(local_textbox);
	
	// set remote textbox caption to the caption of the local textbox
	liqcell *textbox = (liqcell *)liqcell_getdata(keyboard);
	liqcell_setcaption(textbox, caption);
	
	// get out of here!
	liqcell_setvisible(keyboard, 0);

	return 0;
}

/**	
 * dialogusername.buttondel clicked
 */	
static int buttondel_click(liqcell *self,liqcelleventargs *args, liqcell *keyboard)
{
	liqcell *textbox = liqcell_child_lookup(keyboard, "liqkeyboard_textbox");
	
	// fake a backspace!
	liqcellkeyeventargs keyargs={0};
	keyargs.keycode = (int)8;
	char delbuf[2];
	delbuf[0]=8;
	delbuf[1]=0;
	snprintf(keyargs.keystring,sizeof(keyargs.keystring),delbuf);
	keyargs.ispress = 1;
	liqcell_handlerrun(textbox,"keypress",&keyargs);
	
	return 0;
}

static int vkbd_textbox_shown(liqcell *self, liqcelleventargs *args, liqcell *keyboard)
{
	char *caption = liqcell_propgets(keyboard, "remote_textbox_caption", 0);
	liqcell_setcaption(self, "test");
	return 0;
}


//######################################################################
//######################################################################
//######################################################################


// liqcell_setcontent(hiddenentry, actualtextboxcell);

liqcell *liqkeyboard_create()
{
	liqcell *self = liqcell_quickcreatewidget("liqkeyboard", "form", 800, 480);
	if (!self) { liqapp_log("liqcell error not create 'liqkeyboard'"); return NULL; } 
	
	
	// textbox
	liqcell *textbox = liqcell_quickcreatevis("liqkeyboard_textbox", "textbox", 106, 49, 600, 82);
	liqcell_setfont(textbox, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (60), 0));
	liqcell_setcaption(textbox, "");
	liqcell_propsets(textbox, "textcolor", "rgb(255,255,255)" );
	liqcell_propsets(textbox, "backcolor", "rgb(0,0,0)" );
	liqcell_propsets(textbox, "bordercolor", "rgb(200,100,100)" );
	liqcell_propseti(textbox, "textalign", 0 );
	


	liqcell_propseti(textbox,  "selstart",  0 );
	//liqcell_propseti(textbox,  "sellength", strlen(liqcell_getcaption(item1)) );
	//liqcell_propseti(textbox,  "cursorpos", strlen(liqcell_getcaption(item1)) );
		
	liqcell_child_append(self, textbox);
	

	// keyboard
	liqcell *liqkeyboard = build_vkbd();
	liqcell_setfont(liqkeyboard, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (12), 0));
	//liqcell_propsets(liqkeyboard, "textcolor", "rgb(0,0,0)");
	//liqcell_propsets(liqkeyboard, "backcolor", "rgb(0,128,128)");
	liqcell_child_append(self, liqkeyboard);

	liqcell_handleradd_withcontext(liqkeyboard, "keypress", (void*)liqkeyboard_keypress, self);
	liqcell_handleradd_withcontext(liqkeyboard, "keyrelease", (void*)liqkeyboard_keyrelease, self);
	
	// backspaces
	liqcell *buttondel = liqcell_quickcreatevis("buttondel", "commandbutton", 700, 240, 100, 120);
	liqcell_setfont(	buttondel, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (12), 0) );
	liqcell_setcaption(buttondel, "< BACK" );
	liqcell_handleradd_withcontext(buttondel, "click", (void*)buttondel_click, self );
	liqcell_propseti(     buttondel,"textalign", 2 );
	liqcell_propseti(     buttondel,"textaligny",2 );
	liqcell_child_append(  self, buttondel);
	
	// enter
	liqcell *enter = liqcell_quickcreatevis("keyboard_enter", "commandbutton", 700, 360, 100, 120);
	liqcell_setcaption(enter, "ENTER" );
	liqcell_setfont(enter, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (12), 0) );
	liqcell_propsets(enter, "backcolor", "rgb(rgb(0,100,0))");
	liqcell_handleradd_withcontext(enter, "click", (void*)enter_click, self);
	liqcell_propseti(enter,"textalign", 2 );
	liqcell_propseti(enter,"textaligny",2 );
	liqcell_child_append(self, enter);

	return self;
}

//######################################################################
//######################################################################
//######################################################################
static liqcell *keyrowstart(liqcell *keyboard, char *title)
{
	return mkframe(keyboard,title,700,240/5);
}

static liqcell *keystd(liqcell *keyboard, liqcell *keyrow, int keysize,char *keycode,char *normal,char *caps)
{
	float sizes[9] = { 1.0,      1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 6.0, 20.0};
	// width/15
	if(keysize<0 || keysize>7) keysize=0;
	int w=(int)   ((float)(700/14) * sizes[keysize]);
	liqcell *key;
	
	int keycodenumeric=1;
	if(*keycode)
	{
		keycodenumeric=atoi(keycode);
	}

	if(keycodenumeric==0)
	{
		key = mkkey(keyrow,keycode,w,240/5);
		liqcell_setcaption(key,normal);
		
		liqcell_propsets(  key, "backcolor", "rgb(100,150,100)" );

		//liqcell_propsets(     key,"fontname", "/usr/share/fonts/nokia/nosnb.ttf" );
		//liqcell_propseti(     key,"fontsize", 24 );
		//liqcell_propsets(     key,"backcolor", "rgb(100,100,150)" );
		//liqcell_propseti(     key,"textalign", 2 );
		//liqcell_propseti(     key,"textaligny",2 );

		//key->style=NULL;
	}
	else
	{
		key = mkkey(keyrow,normal,w,240/5);
		//key->style=stylekeycap;
		//key->handlermouse=key_mouse;

		liqcell_propseti(     key,"textalign",1);
		liqcell_propsets(     key,"fontname", "/usr/share/fonts/nokia/nosnb.ttf" );
		liqcell_propseti(     key,"fontsize", 32 );
		liqcell_propsets(     key,"textcolor", "rgb(255,255,255)" );
		liqcell_propsets(     key,"backcolor", "rgb(100,150,100)" );
		liqcell_propseti(     key,"textalign", 2 );
		liqcell_propseti(     key,"textaligny",2 );
		
		liqcell_propsets(  key, "backcolor", "rgb(100,150,100)" );
		
		//liqcell_handleradd_withcontext(   key,"mouse", key_mouse, keyboard );
		liqcell_handleradd_withcontext(   key,"click", (void *)key_click, keyboard );
		liqcell_settag(       key,(void*)keycodenumeric);

	}
	return key;

}

static void keyrowend(liqcell *keyrow)
{
	liqcell_child_arrange_autoflow(keyrow);
}

liqcell *build_vkbd()
{
	liqcell *keyboard = liqcell_quickcreatevis("liqkeyboard", "form", 0, 240, 700, 240);

	//static liqcell *keyboard=NULL;
	liqcell *keyrow=NULL;
	
	keyrow = keyrowstart(keyboard, "num");
//		keystd(1,"ESC","","");
		keystd(keyboard, keyrow, 1,"",   "`" ,"¬");
		keystd(keyboard, keyrow, 1,"",   "1" ,"!");
		keystd(keyboard, keyrow, 1,"",   "2" ,"\"");
		keystd(keyboard, keyrow, 1,"",   "3" ,"£");
		keystd(keyboard, keyrow, 1,"",   "4" ,"$");
		keystd(keyboard, keyrow, 1,"",   "5" ,"%");
		keystd(keyboard, keyrow, 1,"",   "6" ,"^");
		keystd(keyboard, keyrow, 1,"",   "7" ,"&");
		keystd(keyboard, keyrow, 1,"",   "8" ,"*");
		keystd(keyboard, keyrow, 1,"",   "9" ,"(");
		keystd(keyboard, keyrow, 1,"",   "0" ,")");
		keystd(keyboard, keyrow, 1,"",   "-" ,"_");
		keystd(keyboard, keyrow, 1,"",   "=" ,"+");
	keyrowend(keyrow);

	keyrow = keyrowstart(keyboard, "qwerty");
		keystd(keyboard, keyrow, 3,"","\t","");
		keystd(keyboard, keyrow, 1,"",   "q" ,"Q");
		keystd(keyboard, keyrow, 1,"",   "w" ,"W");
		keystd(keyboard, keyrow, 1,"",   "e" ,"E");
		keystd(keyboard, keyrow, 1,"",   "r" ,"R");
		keystd(keyboard, keyrow, 1,"",   "t" ,"T");
		keystd(keyboard, keyrow, 1,"",   "y" ,"Y");
		keystd(keyboard, keyrow, 1,"",   "u" ,"U");
		keystd(keyboard, keyrow, 1,"",   "i" ,"I");
		keystd(keyboard, keyrow, 1,"",   "o" ,"O");
		keystd(keyboard, keyrow, 1,"",   "p" ,"P");
		keystd(keyboard, keyrow, 1,"",   "[" ,"{");
		keystd(keyboard, keyrow, 1,"",   "]" ,"}");
	keyrowend(keyrow);

	keyrow = keyrowstart(keyboard, "asdf");
		keystd(keyboard, keyrow, 4,"CLK","","");
		keystd(keyboard, keyrow, 1,"",   "a" ,"A");
		keystd(keyboard, keyrow, 1,"",   "s" ,"S");
		keystd(keyboard, keyrow, 1,"",   "d" ,"D");
		keystd(keyboard, keyrow, 1,"",   "f" ,"F");
		keystd(keyboard, keyrow, 1,"",   "g" ,"G");
		keystd(keyboard, keyrow, 1,"",   "h" ,"H");
		keystd(keyboard, keyrow, 1,"",   "j" ,"J");
		keystd(keyboard, keyrow, 1,"",   "k" ,"K");
		keystd(keyboard, keyrow, 1,"",   "l" ,"L");
		keystd(keyboard, keyrow, 1,"",   ";" ,":");
		keystd(keyboard, keyrow, 1,"",   "'" ,"@");
		keystd(keyboard, keyrow, 1,"",   "#" ,"~");
		//keystd(2,"CR","","");
	keyrowend(keyrow);

	keyrow = keyrowstart(keyboard, "zxcv");
		keystd(keyboard, keyrow, 5,"SHL","","");
		keystd(keyboard, keyrow, 1,"",   "z" ,"Z");
		keystd(keyboard, keyrow, 1,"",   "x" ,"X");
		keystd(keyboard, keyrow, 1,"",   "c" ,"C");
		keystd(keyboard, keyrow, 1,"",   "v" ,"V");
		keystd(keyboard, keyrow, 1,"",   "b" ,"B");
		keystd(keyboard, keyrow, 1,"",   "n" ,"N");
		keystd(keyboard, keyrow, 1,"",   "m" ,"M");
		keystd(keyboard, keyrow, 1,"",   "," ,"<");
		keystd(keyboard, keyrow, 1,"",   "." ,">");
		keystd(keyboard, keyrow, 1,"",   "/" ,"?");
		keystd(keyboard, keyrow, 1,"",   "@" ,"@");
	keyrowend(keyrow);

	keyrow = keyrowstart(keyboard, "bottom");
		keystd(keyboard, keyrow, 3,"CTL","","");
		keystd(keyboard, keyrow, 3,"WN1","","");
		keystd(keyboard, keyrow, 3,"ALT","","");
		keystd(keyboard, keyrow, 7,""," ","");
	keyrowend(keyrow);


	//liqcell_child_arrange_easycol(keyboard);
	liqcell_child_arrange_autoflow(keyboard);
	return keyboard;
}

#ifdef __cplusplus
}
#endif

