/**
 * @file	textbox.c
 * @author  Gary Birkett, Zach Habersang
 * @brief 	Textbox UI Component
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

// colors
#define  BLACK    "rgb(0,0,0)"
#define  WHITE    "rgb(255,255,255)"
#define  RED      "rgb(210,0,0)"
#define  GREEN    "rgb(0,173,0)"
#define  BLUE     "rgb(0,0,255)"
#define  YELLOW   "rgb(225,225,0)"
#define  CYAN     "rgb(0,175,175)"
#define  MAGENTA  "rgb(255,0,255)"



int textbox_selectall(liqcell *textbox)
{
	// select all!
	int selstart = liqcell_propgeti(  textbox,"selstart",-1);
	int sellength = liqcell_propgeti(  textbox,"sellength",0);
	int cursorpos = liqcell_propgeti(  textbox,"cursorpos",-1);
	
	char *caption = liqcell_getcaption(textbox);
	int captionlen = strlen(caption);


	liqcell_propseti(  textbox,  "selstart",  0 );
	liqcell_propseti(  textbox,  "sellength", captionlen );
	liqcell_propseti(  textbox,  "cursorpos", captionlen );
       
	
}

int textbox_fakebackspace(liqcell *textbox)
{
	// fake a backspace!
	liqcellkeyeventargs keyargs={0};
	keyargs.keycode = (int)8;
	char delbuf[2];
	delbuf[0]=8;
	delbuf[1]=0;
	snprintf(keyargs.keystring,sizeof(keyargs.keystring),delbuf);
	keyargs.ispress = 1;
	liqcell_handlerrun(textbox,"keypress",&keyargs);
}




int textbox_clear(liqcell *textbox)
{
	// clear

	liqcell_setcaption(textbox,"");


	liqcell_propseti(  textbox,  "selstart",  0 );
	liqcell_propseti(  textbox,  "sellength", 0 );
	liqcell_propseti(  textbox,  "cursorpos", 0 );
       
	return 0;
}




	// damn, this should be taken care of by the OS itself
	// this is because the hitpoint will not always match with where I think it will be
	// this is bad and good in practice
	static int textbox_mouse(liqcell *self, liqcellmouseeventargs *args,void *context)
	{
		// if i have a font on my cell, surely it will have been rendered correctly first..
		// infact, thats right i think
		//liqcell *base = liqcell_getbasewidget(self);
		//liqcell *body = self;

		liqfont *font = liqcell_getfont(	self);
		if(!font)return 0;

		char *cap = liqcell_getcaption(self);
		if(!cap)return 0;
		
				// 20090814_184437 lcuk : if password, replace cap with string("*",len(cap)) for selection purposes
				char passbuff[1024];				
				if(liqcell_propgeti(self,"textispassword",0))
				{
					int clen = strlen(cap);
					if(clen>=sizeof(passbuff)-1)clen=sizeof(passbuff)-1;
					int x;
					for(x=0;x<clen;x++)passbuff[x]='*';
					passbuff[x]=0;
					cap=passbuff;
				}
		
		//int caplen = strlen(cap);
		
		
		int mx = args->mex - liqcell_getx(self);
		
		int chpos = liqfont_textfitinside(font,cap,mx);
										  
		// neat :) i know where the mouse clicked (left aligned text only...)
		// todo: handle other alignments
		
		if(args->mcnt==1)
		{
			// start of selection..
			liqcell_propseti(  self,  "selfirst",  chpos );
			liqcell_propseti(  self,  "selstart",  chpos );
			liqcell_propseti(  self,  "sellength", 0 );
			liqcell_propseti(  self,  "cursorpos", chpos );
		}
		else
		{
			// extending selection
			
			int selfirst = liqcell_propgeti(  self,"selfirst",chpos);
			int selstart;
			int sellength;
			
			if(chpos<selfirst)
			{
				// got to invert
				selstart=chpos;
				sellength=selfirst-chpos;
			}
			else
			{
				selstart=selfirst;
				sellength=chpos-selfirst;				
			}

			
			liqcell_propseti(  self,  "selstart",  selstart );
			liqcell_propseti(  self,  "sellength", sellength );
			liqcell_propseti(  self,  "cursorpos", chpos );
		}
		
		return 1;

	}

	static int textbox_keypress(liqcell *self, liqcellkeyeventargs *args,void *context)
	{
		//liqcell *base = liqcell_getbasewidget(self);
		// i can then use my base to access members as defined by the widget itself
		//liqcell_setcaption(self,args->keystring);
	
		int selstart = liqcell_propgeti(  self,"selstart",-1);
		int sellength = liqcell_propgeti(  self,"sellength",0);
		int cursorpos = liqcell_propgeti(  self,"cursorpos",-1);
		
		char *caption = liqcell_getcaption(self);
		int captionlen = strlen(caption);
		
		char *key = args->keystring;
		if(!key)key="";
		if(*key && *key<32)
		{
			if(*key==10)
			{
				liqcell_handlerrun(self,"click",NULL);
				key="";
			}
			if(*key==8 || *key==9)
			{
				// bs and tab
			}
			else
			{
				// ack! ignore these in single line textbox!
				key="";
			}
		}

		
		int keylen = strlen(key);
		
		if(selstart>captionlen){ selstart=captionlen;sellength=0; }
		
		if(selstart+sellength>captionlen)
		{
			sellength = captionlen-selstart;
		}

		
		
		if(selstart>=0)// && (keylen>0))
		{
			liqapp_log("keypress: %3i '%c' %i %i",(int)(*key),*key,args->keycode,args->keymodifierstate);
			
			if(cursorpos<0)cursorpos=0;
			if(keylen==0)
			{
				// special keys


				if(args->keycode==XK_Left)
				{
					selstart--;
					if(selstart<0)selstart=0;
					if(args->keymodifierstate==0)
						sellength=0;
					else
						sellength++;
						
					cursorpos=selstart;
					liqcell_propseti(  self,  "selstart",  selstart);
					liqcell_propseti(  self,  "sellength", sellength);
					liqcell_propseti(  self,  "cursorpos", cursorpos);
				}
				else
				if(args->keycode==XK_Right)
				{
					if(args->keymodifierstate==0)
					{
						selstart+=sellength+1;
						if(selstart>captionlen)selstart=captionlen;
						sellength=0;
						//cursorpos=selstart;
					}
					else
					{
						sellength++;
						//cursorpos=selstart+sellength;
						//if(cursorpos>captionlen)cursorpos=captionlen;
					}
					
					if(selstart+sellength>captionlen)
					{
						sellength = captionlen-selstart;
					}
					

					cursorpos=selstart+sellength;
					liqcell_propseti(  self,  "selstart",  selstart);
					liqcell_propseti(  self,  "sellength", sellength);
					liqcell_propseti(  self,  "cursorpos", cursorpos);
				}	

			}
			else
			{
				// regular keypress 
				if(*key==8)
				{
					// delete ;)
					key="";
					keylen=0;
					if(selstart>0 && sellength==0)
					{
						selstart--;
						sellength++;
					}
				}
			
				//
				//liqcell_setcaption(self,args->keystring);
				char *aftersel=&caption[selstart+sellength];
				int aftersellen = strlen(aftersel);
				
				// then the result is start..selstart
				// newbit
				// selstart+sellen..end
				
				
				// !-- BUG FIX BY ZACH HABERSANG -- !
				// ----------------------------------
				// Program would segfault when 25 or so characters were entered
				// Fix: + 1 fix to reqd! :D
				// note: used gdb with backtrace to find this bug
				
				// 20090615_210659 lcuk : and me to explain why it was wrong ;) damn those +1 adjustments..
				
				int reqd = selstart  +  keylen  +  aftersellen + 1;
				char *buff=malloc(reqd);
				char *block=buff;
				if(buff)
				{
					if(selstart>0)
					{
						strncpy(block,caption,selstart);
						block+=selstart;
					}
					if(keylen>0)
					{
						strncpy(block,key,keylen);
						block+=keylen;
					}
					
					if(aftersellen>0)
					{
						strncpy(block,aftersel,aftersellen);
						block+=aftersellen;
					}
					*block=0;
					liqcell_setcaption(self,buff);
	
	
	
					
					free(buff);
				}
	
				liqcell_propseti(  self,  "selstart",  selstart + keylen);
				liqcell_propseti(  self,  "sellength", 0 );
				liqcell_propseti(  self,  "cursorpos", selstart + keylen);			
			}
			
		}
		
		
		return 0;

	}
	
	
	
	
	
static int textbox_keyrelease(liqcell *self, liqcellkeyeventargs *args)
{
	return 0;
}



static int widget_click(liqcell *self, liqcellclickeventargs *args)
{
	liqcell *dialog=liqcell_getcontent(self);
	liqcell_easyrun(dialog);
	liqcell_setvisible(dialog,1);
	//args->newdialogtoopen = liqcell_getcontent(self);
	return 1;
}

static int keyboard_show_button_click(liqcell *self,liqcellclickeventargs *args, liqcell *textbox)
{
	liqcell *vkbd = liqcell_quickcreatevis("vkbd", "liqkeyboard", 0, 0, -1, -1);
	liqcell *vkbd_textbox = liqcell_child_lookup(vkbd, "liqkeyboard_textbox");
	
	if(vkbd)
	{
		char *caption = liqcell_getcaption(textbox);
		liqcell_setcaption(vkbd_textbox, caption);
		liqcell_setdata(vkbd, textbox);
		liqcell_easyrun(vkbd);
		liqcell_release(vkbd);
	}
}

static int textbox_resize(liqcell *self, liqcelleventargs *args, void *textbox)
{
	#define IW 32 // icon width
	#define IH 32 // icon height
	
	liqcell *kb_show = liqcell_child_lookup(self, "vkbd_command");
	int ww = liqcell_getw(self);
	int hh = liqcell_geth(self);
	liqcell_setrect(kb_show,  ((ww-IW)-8),  ((hh - IH)/2),        IW,IH);   // right hand side, leaving 10 pixels X, and filling 80% of the height
	return 0;
}

/*
void liqcell_setrect(liqcell *self,int x,int y,int w,int h)
{
	if(self->x==x && self->y==y && self->w==w && self->h==h)return;
	self->x=x;
	self->y=y;
	self->w=w;
	self->h=h;
	liqcell_handlerrun(self,"move",NULL);
	liqcell_handlerrun(self,"resize",NULL);
}
*/

/**
 * Create a textbox ui component
 * @return liqcell* The new textbox
 */
liqcell *textbox_create()
{
	
	liqcell *self = liqcell_quickcreatewidget("textbox","form", 600,50);
	
	if(self)
	{
			
		liqcell_setfont(   self,  liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_propsets(  self,  "backcolor", "rgb(100,255,150)" );
		liqcell_propsets(  self,  "textcolor", "rgb(20,30,40)" );
		liqcell_propsets(  self,  "bordercolor", "rgb(255,255,255)" );

		liqcell_propseti(  self,  "selstart",  0 );
		liqcell_propseti(  self,  "sellength", 0 );
		liqcell_propseti(  self,  "cursorpos", 0 );
			
		liqcell_handleradd(self,    "mouse",      textbox_mouse);
		liqcell_handleradd(self,    "keypress",   textbox_keypress);
		liqcell_handleradd(self,    "keyrelease", textbox_keyrelease);
		
		liqcell_handleradd(self,    "resize",   textbox_resize);
	
			
		// add vkbd
		liqcell *vkbd_command = liqcell_quickcreatevis("vkbd_command" , "commandbutton", 0, 0, 0, 0);
		liqcell_handleradd_withcontext(vkbd_command, "click", keyboard_show_button_click, self);
		liqcell_setfont(vkbd_command, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (12), 0));
		liqcell_setcaption(vkbd_command, "ABC");
		liqcell_propsets(vkbd_command, "backcolor", CYAN);
		liqcell_propsets(vkbd_command, "textcolor", BLACK);
		liqcell_propseti(vkbd_command, "textalign", 2);
		liqcell_propseti(vkbd_command, "textaligny", 2);
		liqcell_propseti(vkbd_command, "lockaspect", 1);
		
		liqcell_child_insert(self, vkbd_command);
	}
	return self;
}
