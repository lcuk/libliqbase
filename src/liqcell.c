/**
 * @file	liqcell.c
 * @author  Gary Birkett
 * @brief 	liqcell App level helper functions
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

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <stdarg.h>
#include <dirent.h>

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>

#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks


#include "liqbase.h"


#include "liqcell.h"
#include "liqcell_prop.h"


char*  liqcell_local_lookup_getname(liqcell *self,char *name)
{
	liqcell *c = liqcell_local_lookup(self,name);
	if(c) return liqcell_getname(c);
	return NULL;
}
char*  liqcell_local_lookup_getcaption(liqcell *self,char *name)
{
	liqcell *c = liqcell_local_lookup(self,name);
	if(c) return liqcell_getcaption(c);
	return NULL;	
}



//#########################################################################
//#########################################################################
//######################################################################### cell construction and reference counting
//#########################################################################
//#########################################################################

/**
 * Low level liqcell creation which allocates memory and sets the liqcell into use.
 * @return liqcell* The newly created liqcell
 */
liqcell *liqcell_new()
{
	// use this to allocate and hold onto a reference
	// should really overload this and allow variations..
	liqcell *self = (liqcell *)calloc( sizeof( liqcell ),1 );
	memset((char *)self,0, sizeof( liqcell ));
	self->usagecount = 1;
	self->enabled=1;
	return self;
}

/** 
 * Add to the usage count, hold onto an object which someone else created
 * @param self The liqcell to modify
 * @return liqcell* The modified liqcell
 */
liqcell * liqcell_hold(liqcell *self)
{
	// use this to hold onto an object which someone else created



	if(self)self->usagecount++;
    
	int imgc=0;
	if(self->image)imgc=self->image->usagecount;
	//liqapp_log("liqcell hold %s::%i::%i",self->name,self->usagecount,imgc);


	return self;
}

/**
 * Decrease the usage count, once this gets to 0, the liqcell is freed
 * @param self The liqcell to modify
 */
void liqcell_release(liqcell *self)
{
	if(!self) return;
	// use this when you are finished with an object
	int imgc=0;
	if(self->image)imgc=self->image->usagecount;
	//liqapp_log("liqcell release %s::%i::%i",self->name,self->usagecount,imgc);
	
	self->usagecount--;
	if(!self->usagecount) liqcell_free(self);
}

/**
 * Recursively free all liqcells of the parent liqcell provided and their items 
 * (font, image, etc..)
 * @param self The parent will have all children freed
 */
void liqcell_free(liqcell *self)
{
	//liqapp_log("liqcell freeing tree %s:%s",self->name,self->classname?self->classname:"");
	// never call _free directly, outside sources should _release the object

	// these char* objects should really be results from the strings tree
	// and hence need dereferencing before use, decide later not important for now
	



    
    // time to notify everyone about our impending death.
    liqcell_handlerrun(self,"destroy",NULL);
    

    if( self->classname && (strcmp(self->classname,"liqtimer")==0) )
    {
        // cheat to close down the thread!
        pthread_t 		*tid = (pthread_t *)liqcell_getdata(self);
        if(tid)
        {
            // only live once...
            liqapp_log("liqcell_free thread cancelling for '%s'",self->name);
            pthread_cancel(*tid);
            tid=NULL;
            liqapp_log("liqcell_free thread cancelled for '%s'",self->name);
       }
    }
    
	if(self->linkparent)
	{
		//
		//liqcell_release(self->linkparent);
		self->linkparent=NULL;
	}    
    	
	
	liqcell *c=self->linkchild;
	
	self->linkchild=NULL;
	
	while(c)
	{
		//liqapp_log("meh %s",c->name);
		liqcell *d=c->linknext;
		
		liqcell *cprev = c->linkprev;
		liqcell *cnext = c->linknext;
		if(cprev) cprev->linknext = cnext;
		if(cnext) cnext->linkprev = cprev;

		c->linkprev=NULL;
		c->linknext=NULL;
		c->linkparent=NULL;
		
		if(self->linkchild==NULL)
		{
			if(cprev)
			{
				self->linkchild=cprev;
			}
			else
			{
				self->linkchild=cnext;
			}			
		}
				
		liqcell_release(c);
		
		c=d;
	}

	
	//liqapp_log("liqcell freeing contents %s",self->name);
	liqcell_setname(self,NULL);


	liqcell_setcaption(self,NULL);

	liqcell_setclassname(self,NULL);

	liqcell_setcontext(self,NULL);
	liqcell_setdata(self,NULL);

	liqcell_setfont(self,NULL);

	liqcell_setcontent(self,NULL);
	liqcell_setsketch(self,NULL);
	liqcell_setimage(self,NULL);


	
	//liqapp_log("liqcell freeing self");
	
	
	free(self);
	//liqapp_log("liqcell freed");
}

//#########################################################################
//#########################################################################
//######################################################################### set data
//#########################################################################
//#########################################################################

/**
 * Set the x, y coordinate position of the liqcell
 * @param self Liqcell to set the position of
 * @param x The x-coordinate
 * @param y The y-coordinate
 */
void liqcell_setpos(liqcell *self,int x,int y)
{
	if(self->x==x && self->y==y)return;
	self->x=x;
	self->y=y;
	liqcell_handlerrun(self,"move",NULL);
}

/**
 * Set the width and height of the liqcell
 * @param self Liqcell to set dimensions of
 * @param w The width
 * @param h The height
 */
void liqcell_setsize(liqcell *self,int w,int h)
{
	if(self->w==w && self->h==h)return;
	self->w=w;
	self->h=h;
	liqcell_handlerrun(self,"resize",NULL);
}

/**
 * Set the size of a liqcell by providing just the liqcell and using the inner width
 * and inner height
 * @param self The liqcell to set the size of
 */
void liqcell_setsize_from_inner(liqcell *self)
{
	liqcell_setsize(self, self->innerw,self->innerh);
}

/**
 * Set the inner sizes parameters of the liqcell
 * @param self The liqcell to modify
 * @param w Width
 * @param h Height
 */
void liqcell_setinnersize(liqcell *self,int w,int h)
{
	self->innerw=w;
	self->innerh=h;
}

/**
 * Set the inner size parameters using the width and height parameters of the liqcell
 * @param self The liqcell to modify
 */
void liqcell_setinnersize_from_outer(liqcell *self)
{
	self->innerw=self->w;
	self->innerh=self->h;
}

/**
 * Set the x, y-coordinates and height/width of a liqcell
 * @param self The liqcell to modify
 * @param x X-Coordinate
 * @param y Y-Coordinate
 * @param w Width
 * @param h Height
 */
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

/**
 * Move and resize a liqcell
 * @param self The liqcell to modify
 * @param x New x-coordinate
 * @param y New y-coordinate
 * @param w New width
 * @param h New height
 * @param fraction Ratio to increase or decreases the liqcell position and size
 * @return int Success or Failure
 */
int 	liqcell_movetowardsrect(liqcell *self,int x,int y,int w,int h, float fraction)
{
	if(self->x==x && self->y==y && self->w==w && self->h==h)
	{
		// already at target
		return 0;
	}
	if(fraction<0 || fraction>1)
	{
		// hmm how to respond
		return 1;
	}
	self->x=self->x + ((x-self->x) * fraction);
	self->y=self->y + ((y-self->y) * fraction);
	self->w=self->w + ((w-self->w) * fraction);
	self->h=self->h + ((h-self->h) * fraction);
	liqcell_handlerrun(self,"move",NULL);
	liqcell_handlerrun(self,"resize",NULL);

	if(self->x==x && self->y==y && self->w==w && self->h==h)
	{
		// now arrived at target
		return 0;
	}
	// still work to do
	return 1;
}



/**
 * Set the rect, but use default original scaling to do it
 * @param self The liqcell to modify
 * @param x X-Coordinate
 * @param y Y-Coordinate
 * @param w Width
 * @param h Height
 * @param sx X Scaling fraction
 * @param sy Y Scaling fraction
 */
void liqcell_setrect_autoscale(liqcell *self,int x,int y,int w,int h,float sx,float sy)
{
	if(!self)return;
	//liqcell *par = liqcell_getlinkparent(self);
	//if(!par)return;
	//if(!par->w)return;
	//if(!par->h)return;
	//if(!par->innerw)return;
	//if(!par->innerh)return;
	
	//self->x=x*par->innerw/par->w;
	//self->y=y*par->innerh/par->h;
	//self->w=w*par->innerw/par->w;
	//self->h=h*par->innerh/par->h;

	self->x=x*sx;//*par->w/par->innerw;
	self->y=y*sy;//*par->h/par->innerh;
	self->w=w*sx;//*par->w/par->innerw;
	self->h=h*sy;//*par->h/par->innerh;

	liqcell_handlerrun(self,"move",NULL);
	liqcell_handlerrun(self,"resize",NULL);
}

/**
 * Increase or decrease the liqcell x, y-coordinates
 * @param self The liqcell to modify
 * @param dx Change in the x-coordinate
 * @param dy Change in the y-coordinate 
 */
void 	liqcell_adjustpos(liqcell *self,int dx,int dy)
{
	if(dx==0 && dy==0)return;
	self->x+=dx;
	self->y+=dy;
	liqcell_handlerrun(self,"move",NULL);
}

/**
 * Increase or decrease the liqcell width and height
 * @param self The liqcell to modify
 * @param dw Change in the width
 * @param dh Change in the height 
 */
void 	liqcell_adjustsize(liqcell *self,int dw,int dh)
{
	if(dw==0 && dh==0)return;
	self->w+=dw;
	self->h+=dh;
	liqcell_handlerrun(self,"resize",NULL);

}

/**
 * Increase or decrease the liqcell inner width and inner height
 * @param self The liqcell to modify
 * @param dw Change in the width
 * @param dh Change in the height 
 */
void 	liqcell_adjustinnersize(liqcell *self,int dw,int dh)
{
	self->innerw+=dw;
	self->innerh+=dh;
}

/**
 * Set the dirty hold to indicate whether a liqcell can be updated
 * @param self The liqcell to modify
 * @param dirtyhold Set this to 0 to indicate no hold, can be updated.
 */
void liqcell_setdirtyhold(liqcell *self,int dirtyhold)
{
	//liqapp_log("dirty hold : '%s'",self->name);
	//the dirty hold switch indicates we should hold the dirty until we release
	self->dirtyhold=dirtyhold;
	if(dirtyhold==0 && self->dirty)
	{
		// push back the dirt once the hold is released
		liqcell_setdirty(self,1);
	}
}

/**
 * Get the dirtyhold value of the liqcell
 * @param self The liqcell to lookup
 */
int liqcell_getdirtyhold(liqcell *self)
{
	return self->dirtyhold;
}

/**
 * Set dirty in order to indicate that the liqcell has changed, and needs to be updated
 * @param self The liqcell to modify
 * @param dirty Set this to 1 to indicate that the liqcell needs to be updated
 */
void liqcell_setdirty(liqcell *self,int dirty)
{
	// dirty flag indicates something has changed
	// i will also allow raising a generic changed event at this time :)

	//liqapp_log("dirty change : '%s'=%i",self->name,dirty);

	self->dirty=dirty;
	if((dirty) && (self->linkparent) && (self->dirtyhold==0))
	{
		// cascade backwards telling parent we are dirty
		// note, we dont cascade a clean signal backwards..
		liqcell_setdirty( self->linkparent, 1 );
	}
	if(dirty)
	{
		if(liqcell_getflagwidget(self) || (self->linkparent==NULL))
		{
			liqcell_handlerrun(self,"dirty",NULL);
		}
	}
}

/**
 * Get the dirty value of the liqcell
 * @param self The liqcell to lookup
 */
int    	liqcell_getdirty(liqcell *self)
{
	return self->dirty;
}

/**
 * Set the liqcell name
 * @param self The liqcell to modify
 * @param name The name of the liqcell
 */
void liqcell_setname(liqcell *self,char *name)
{
	if(self->name)
	{
		free(self->name);
		self->name=NULL;
	}
	if(name) {     self->name      = strdup(name);   if(!self->name) liqapp_errorandfail(-1,"cannot alloc name");    }//      liqcell_setdirty(self,1);    }
}

/**
 * Set the caption of the liqcell
 * @param self The liqcell to modify
 * @param caption The caption of the liqcell
 */
void liqcell_setcaption(liqcell *self,char *caption)
{
	// 20090626_213441 lcuk : did some extra logging for zachmon's benefit
	if(!self)
	{
		liqapp_log("liqcell_setcaption failed, self==NULL");
		return;		
	}
	//liqapp_log("caption on '%s' changing to '%s'",self->name, (caption?caption:"[NULL]") );
	if(self->caption)
	{
		//liqapp_log("caption on '%s' was '%s'",self->name,self->caption);
		free(self->caption);
		self->caption=NULL;
	}
	if(caption) {     self->caption      = strdup(caption);   }//        liqcell_setdirty(self,1);    }
	//liqapp_log("caption on '%s' changed, raising event",self->name );
	liqcell_handlerrun(self,"captionchange",NULL);
	//liqapp_log("caption on '%s' change fin",self->name );
}




//############################################################# 
//############################################################# 
//############################################################# 

/** Helper function for liqcell_setcaption */
void liqcell_setcaption_vprintf(liqcell *self,char *format, va_list arg)
{
    //time_t     now;
    //struct tm  *ts;
    char       buf[1024];
	vsnprintf(buf,1023,format, arg);
	liqcell_setcaption(self,buf);
}

/**
 * Set formatted string prop and 
 */
void liqcell_setcaption_printf(liqcell *self,char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	liqcell_setcaption_vprintf(self,format, arg);
	va_end(arg);
}







/**
 * Set the classname of the liqcell
 * @param self The liqcell to modify
 * @param classname The classname of the liqcell
 */
void liqcell_setclassname(liqcell *self,char *classname)
{
	if(self->classname)
	{
		free(self->classname);
		self->classname=NULL;
	}
	if(classname)  {    self->classname      = strdup(classname);      }//     liqcell_setdirty(self,1);    }
}

/**
 * Set the context of the liqcell
 * @param self The liqcell to modify
 * @param context The context of the liqcell
 */
void liqcell_setcontext(liqcell *self,char *context)
{
	if(self->context)
	{
		free(self->context);
		self->context=NULL;
	}
	if(context) {     self->context      = strdup(context);   }//        liqcell_setdirty(self,1);    }
}

/**
 * Set the data of the liqcell
 * @param self The liqcell to modify
 * @param data The data of the liqcell
 */
void liqcell_setdata(liqcell *self,void *data)
{
	if(self->data)
	{
		// do not free data, its YOUR responsibility
		//free(self->data);
		self->data=NULL;
	}
	if(data){      self->data      = data;       }//    liqcell_setdirty(self,1);    }
}

/**
 * Set the sketch of the liqcell
 * @param self The liqcell to modify
 * @param sketch The sketch of the liqcell
 */
void liqcell_setsketch(liqcell *self,liqsketch *sketch)
{
	if(self->sketch)
	{
		liqsketch_release(self->sketch);
		self->sketch=NULL;
	}
	if(sketch) {     self->sketch      = liqsketch_hold(sketch);   }//        liqcell_setdirty(self,1);    }
	//if(sketch) {     self->sketch      = (sketch);   }//        liqcell_setdirty(self,1);    }
}

/**
 * Load the sketch of the liqcell
 * @param self The liqcell to load the sketch of
 */
void liqcell_sketch_autoload(liqcell *self)
{
	if(!self->sketch)
	{
		char *fn = liqcell_propgets(self,"sketchfilename",NULL);
		if(fn && liqapp_fileexists(fn))
		{
			liqcell_setsketch( self, liqsketch_newfromfile(fn) );
		}
	}
}


/**
 * Set the provided liqcell's bakground image.
 * @param self The liqcell
 * @param image The liqimage to set
 */
void liqcell_setimage(liqcell *self,liqimage *image)
{
	if(self->image)
	{
		liqimage_release(self->image);
		self->image=NULL;
		//liqcell_setdirty(self,1);
	}
	if(image){      self->image      = liqimage_hold(image);          liqcell_setdirty(self,1);    }
	//if(image){      self->image      = (image);          liqcell_setdirty(self,1);    }
}

/** 
 * Set the font of the liqcell
 * @param self The liqcell to modify
 * @param font The liqcell's font
 */
void liqcell_setfont(liqcell *self,liqfont *font)
{
	if(self->font)
	{
		liqfont_release(self->font);
		self->font=NULL;
	}
	if(font){      self->font      = liqfont_hold(font);     }//      liqcell_setdirty(self,1);    }
	//if(font){      self->font      = (font);     }//      liqcell_setdirty(self,1);    }
}

/**
 * Set the content of one liqcell to the content of the other liqcell.
 * @param self The destination liqcell
 * @return liqcell* The source liqcell
 */
void liqcell_setcontent(liqcell *self,liqcell *content)
{
	if(self->content)
	{
		liqcell_release(self->content);
		self->content=NULL;
	}
	//if(content){      self->content      = liqcell_hold(content);    }//       liqcell_setdirty(self,1);    }
	if(content){      self->content      = (content);    }//       liqcell_setdirty(self,1);    }

}

/**
 * Set the selection of the liqcell
 * @param self The liqcell to
 * @param arg The argument set to selected parameter of the liqcell
 */
void liqcell_setselected(liqcell *self,int arg)
{
	self->selected      = arg;
	// set dirty..
}

/**
 * Return the selected value
 * @param self The liqcell to lookup
 * @return int The selected value
 */
int liqcell_getselected(liqcell *self)
{
	return self->selected;
}

/**
 * Set the tag of the liqcell
 * @param self The liqcell to modify
 * @param tag The liqcell's tag
 */
void liqcell_settag(liqcell *self,void *tag)
{
	self->tag=(int)tag;
}

/**
 * Get the tag value of the liqcell 
 * @param self The liqcell to lookup
 * @return void* The generic pointer to the tag
 */
void *liqcell_gettag(liqcell *self)
{
	return (void *)self->tag;
}

/**
 * Set the enabled value of the liqcell
 * @param self The liqcell to modify
 * @param arg The value to set enabled of the liqcell
 */
void liqcell_setenabled(liqcell *self,int arg)
{
	self->enabled      = arg;
	// set dirty..
}

/**
 * Return the enabled value
 * @param self The liqcell to lookup
 * @return int The enabled value
 */
int liqcell_getenabled(liqcell *self)
{
	return self->enabled;
}

/**
 * Set the liqcell's kinetic values int he x and y direction
 * @param self The liqcell to modify
 * @param kx The kinetic value in the x direction
 * @param ky The kinetic value in the y direction
 */
void liqcell_setkinetic(liqcell *self,int kx,int ky)
{
	//liqapp_log("kinetic: %i,%i",kx,ky);
	self->kineticx=kx;
	self->kineticy=ky;
}

/**
 * Set the shown value of the liqcell
 * @param self The liqcell to modify
 * @param arg The value to set shown
 */
void liqcell_setshown(liqcell *self,int arg)
{
	if(arg)
	{
		self->kind |= cellkind_shown;
	}
	else
	{
		self->kind &= !cellkind_shown;
	}
}

/**
 * Get the shown value of the liqcell
 * @param self The liqcell to lookup
 * @return int The shown value
 */
int liqcell_getshown(liqcell *self)
{
	return (self->kind & cellkind_shown);
}


//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################

/**
 * Get the linked parent of the liqcell
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the parent
 */
liqcell *liqcell_getlinkparent(liqcell *self)
{
	return self->linkparent;

}

/**
 * Get the linked previous of the liqcell, the previous link in the linked list
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the previous
 */
liqcell *liqcell_getlinkprev(liqcell *self)
{
	return self->linkprev;
}

/**
 * Get the linked next of the liqcell, the next link in the linked list
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the next
 */
liqcell *liqcell_getlinknext(liqcell *self)
{
	return self->linknext;
}

/**
 * Get the linked previous of the liqcell, the previous link in the linked list that is VISUAL - though may be hidden
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the first previous that is visual
 */
liqcell *liqcell_getlinkprev_visual(liqcell *self)
{
	liqcell *c = self->linkprev;
	while(c)
	{
		if(liqcell_getflagvisual(c))return c;
		c=c->linkprev;
	}
	return NULL;
}

/**
 * Get the linked previous of the liqcell, the previous link in the linked list that is VISUAL - though may be hidden
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the first previous that is visual
 */
liqcell *liqcell_getlinknext_visual(liqcell *self)
{
	liqcell *c = self->linknext;
	while(c)
	{
		if(liqcell_getflagvisual(c))return c;
		c=c->linknext;
	}
	return NULL;
}

/**
 * Get the linked child of the liqcell, the child link in the linked list that is VISUAL - though may be hidden
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the first child that is visual
 */
liqcell *liqcell_getlinkchild_visual(liqcell *self)
{
	liqcell *c = self->linkchild;
	while(c)
	{
		if(liqcell_getflagvisual(c))return c;
		c=c->linknext;
	}
	return NULL;
}





/**
 * Get the linked previous of the liqcell, the previous link in the linked list that is VISIBLE - that is, set to show now
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the first previous that is visible
 */
liqcell *liqcell_getlinkprev_visible(liqcell *self)
{
	liqcell *c = self->linkprev;
	while(c)
	{
		if(liqcell_getvisible(c))return c;
		c=c->linkprev;
	}
	return NULL;
}

/**
 * Get the linked previous of the liqcell, the previous link in the linked list that is VISIBLE - that is, set to show now
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the first previous that is visible
 */
liqcell *liqcell_getlinknext_visible(liqcell *self)
{
	liqcell *c = self->linknext;
	while(c)
	{
		if(liqcell_getvisible(c))return c;
		c=c->linknext;
	}
	return NULL;
}

/**
 * Get the linked child of the liqcell, the child link in the linked list that is VISIBLE - that is, set to show now
 * @param self The liqcell to lookup
 * @return liqcell* Liqcell pointer to the first child that is visible
 */
liqcell *liqcell_getlinkchild_visible(liqcell *self)
{
	liqcell *c = self->linkchild;
	while(c)
	{
		if(liqcell_getvisible(c))return c;
		c=c->linknext;
	}
	return NULL;
}


/**
 * Return the linked child of the parent provided.
 * @param self The parent liqcell
 * @return liqcell* The linked child
 */
liqcell *liqcell_getlinkchild(liqcell *self)
{
	return self->linkchild;
}

/**
 * Get the data of the liqcell
 * @param self The liqcell to lookup
 * @return lvoid* The data
 */
void *liqcell_getdata(liqcell *self)
{
	return self->data;
}

/*
liqcell *liqcell_getlinkcontent(liqcell *self)
{
	if((!self->linkcontent) && (self->classname))
	{
		// lookup and find ref to classname
		liqcell *c=NULL;
		liqcell *p=liqcell_getlinkparent(self);
		while(p)
		{
		//liqapp_log("getanc");
			c=liqcell_child_lookup(p,self->classname);
			if((c) && (c!=self))
			{
				self->linkcontent = c;
				return c;
			}
			p=liqcell_getlinkparent(p);
		}
	}
 	return self->linkcontent;
}
*/


int liqcell_getflagwidget(liqcell *self)
{
	return (self->kind & cellkind_widget);
}


int liqcell_getflagvisual(liqcell *self)
{
	return (self->kind & cellkind_visual);
}

int liqcell_getvisible(liqcell *self)
{
	return (self->kind & cellkind_visual) && self->visible;
}

void liqcell_setvisible(liqcell *self,int arg)
{
	
	if(self->kind & cellkind_visual) self->visible=arg;

}


/**
 * Return the name of the provided liqcell.
 * @param self The liqcell
 * @return string The name of the liqcell
 */
char *liqcell_getname(liqcell *self)
{
	return self->name;
}

/**
 * Get the name of a liqcell that includes all of the parent names
 * @param self The liqcell to get the qualified name of 
 * @param buff Copy the qualified name into this buffer
 * @param buffmax Maximum length of the buffer
 * @return int The numbers of bytes used
 */
int liqcell_getqualifiedname(liqcell *self, char *buff, int buffmax)
{
	int buffmaxorig=buffmax;
	int used=0;
	liqcell *p=liqcell_getlinkparent(self);
	if(p)
	{
		//
		used = liqcell_getqualifiedname(p,buff,buffmax);
		buff+=used;
		buffmax-=used;
		if(buffmax<=0)return buffmaxorig;
		used+=snprintf(buff,buffmax,".%s",self->name);
	}
	else
	{
		if(buffmax<=0)return 0;
		used+=snprintf(buff,buffmax,"%s",self->name);
	}
	return used;



}

/**
 * Return the classname
 * @param self The liqcell to lookup
 * @return char* The class name
 */
char *liqcell_getclassname(liqcell *self)
{
	return self->classname;
}

/**
 * Return a liqcell caption
 * @param self The liqcell to lookup
 * @return char* The caption
 */
char *liqcell_getcaption(liqcell *self)
{
	if(!self->caption)
		return self->name;
	return self->caption;
}

/**
 * Return the liqcell's sketch
 * @param self The liqcell to lookup
 * @return liqsketch* The sketch
 */
liqsketch *liqcell_getsketch(liqcell *self)
{
	//if(!self->sketch && self->content)
	//	return liqcell_getsketch(self->content);
	return self->sketch;
}

/**
 * Return the liqcell's image
 * @param self The liqcell to lookup
 * @return liqsketch* The image
 */
liqimage *liqcell_getimage(liqcell *self)
{
	//if(!self->image && self->content)
	//	return liqcell_getimage(self->content);
	return self->image;
}

/**
 * Return the liqcell's font
 * @param self The liqcell to lookup
 * @return liqsketch* The font
 */
liqfont *liqcell_getfont(liqcell *self)
{
	if(!self->font && self->content)
		return liqcell_getfont(self->content);
	return self->font;
}

/**
 * Return the liqcell's content
 * @param self The liqcell to lookup
 * @return liqsketch* The content
 */
liqcell *liqcell_getcontent(liqcell *self)
{
	//if(!self->content && self->linkcontent)
	//	return liqcell_getcontent(self->linkcontent);
	return self->content;
}

/**
 * Return the liqcell's centre x-coordinate
 * @param self The liqcell to lookup
 * @return int The centre x-coordinate
 */
int liqcell_getcx(liqcell *self)
{
	return self->x+self->w/2;
}

/**
 * Return the liqcell's centre y-coordinate
 * @param self The liqcell to lookup
 * @return int The centre y-coordinate
 */
int liqcell_getcy(liqcell *self)
{
	return self->y+self->h/2;
}

/**
 * Return the liqcell's x-coordinate
 * @param self The liqcell to lookup
 * @return int The x-coordinate
 */
int liqcell_getx(liqcell *self)
{
	return self->x;
}

/**
 * Return the liqcell's y-coordinate
 * @param self The liqcell to lookup
 * @return int The y-coordinate
 */
int liqcell_gety(liqcell *self)
{
	return self->y;
}

/**
 * Return the liqcell's width
 * @param self The liqcell to lookup
 * @return int The width
 */
int liqcell_getw(liqcell *self)
{
	return self->w;
}

/**
 * Return the liqcell's height
 * @param self The liqcell to lookup
 * @return int The height
 */
int liqcell_geth(liqcell *self)
{
	return self->h;
}

/**
 * Return the liqcell's inner width
 * @param self The liqcell to lookup
 * @return int The inner width
 */
int liqcell_getinnerw(liqcell *self)
{
	return self->innerw;
}

/**
 * Return the liqcell's inner height
 * @param self The liqcell to lookup
 * @return int The inner height
 */
int    	liqcell_getinnerh(liqcell *self)
{
	return self->innerh;
}





//#########################################################################
//#########################################################################
//######################################################################### cell linkage
//#########################################################################
//#########################################################################


//##################################################################

/**
 * Get the last child of the parent provided
 * @param self The parent liqcell
 * @return liqcell*
 */
liqcell *liqcell_lastchild(liqcell *self)
{
	if(self->linkchild==NULL)return NULL;
	liqcell *sa=self->linkchild;
	while(sa)
	{
		if(sa->linknext==NULL) return sa;
		sa=sa->linknext;
	}
	return NULL;
}

//##################################################################

/**
 * Take the liqcell provided and put it at the top of the chain
 * @param self The liqcell to put to the top
 */
void liqcell_zorder_totop(liqcell *self)
{
	// todo remove the strange blend of procedures and member lookups - i know its an internal function, but...
	liqcell *par = liqcell_getlinkparent(self);
	if(!par) return;
	// this puts it to the end of the list, desired for rendering, not for detecting
	//liqcell *parlc=liqcell_lastchild(par);
	//if(!parlc) return;
	//self->linkprev=parlc;
	//self->linknext=NULL;
	//parlc->linknext=self;
	//return;

	// this puts it to the start of the list...
	liqcell *parc = liqcell_getlinkchild(par);
	if(!parc) return;		// something really messed up here..
	if(parc == self) return;  // already at top :)
	liqcell *p = liqcell_getlinkprev(self);
	liqcell *n = liqcell_getlinknext(self);
	if(p){ p->linknext = n; }
	if(n){ n->linkprev = p; }
	self->linkprev=NULL;
	self->linknext=parc;
	parc->linkprev=self;
	//parc->linknext=NULL;
	par->linkchild = self;
}

/**
 * Link a child liqcell to a parent liqcell by setting the child's linked parent to the parent (self),
 * the child's previous to NULL, and the child's next link to parents's linked child. Then set
 * the parent's linked child to chiled provided. This starts a chain in a linked list
 * of liqcells.
 * 
 * @param self The parent liqcell
 * @param child The child liqcell
 * @return liqcell* Child liqcell
 * 
 */
liqcell*  liqcell_child_insert(liqcell *self,liqcell *child)
{
	// insert the child into the tree
	if(!child)return NULL;
	//liqcell_hold(child);
	child->linkparent=(self);
	child->linkprev=NULL; // we are at the start
	child->linknext=(self->linkchild);
	if(self->linkchild) self->linkchild->linkprev = child;
	self->linkchild = child;
	//self->childcount++;
	liqcell_setdirty(self,1);
	return child;
}

/**
 * Count the number of visible children
 * @param self The liqcell to count the children of
 * @return int The number of children
 */
int liqcell_child_countvisible(liqcell *self)
{
	int answercount=0;

	// count the number of visible children
	liqcell *c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{
			// work it!
			answercount++;		//
		}
		c=liqcell_getlinknext(c);
	}
	return answercount;
}

/**
 * Count the number of selected children
 * @param self The liqcell to count the children of
 * @return int The number of children
 */
int liqcell_child_countselected(liqcell *self)
{
	int answercount=0;

	// count the number of visible children
	liqcell *c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getselected(c))
		{
			// work it!
			answercount++;		//
		}
		c=liqcell_getlinknext(c);
	}
	return answercount;
}
/**
 * Create a chain in an already linked list of liqcells. If a list doesn't exist for the parent
 * (self) then start a new list. Set the child's parent to the parent of the list, and the 
 * child's previous to the previous child. 
 * 
 * @param self The parent liqcell
 * @param c The child liqcell
 * @return liqcell* Child liqcell
 * 
 */
liqcell* liqcell_child_append(liqcell *self,liqcell *c)
{
	if(!c)return NULL;
	liqcell *sa=liqcell_lastchild(self);
	if(sa==NULL)
	{
		liqcell_child_insert(self,c);
		return c;
	}
	
	//liqcell_hold(c);
	c->linkparent=self;
	c->linkprev=sa;
	c->linknext=sa->linknext;	// we are at the end
	sa->linknext=c;
	//self->childcount++;
	liqcell_setdirty(self,1);
	return c;
}

/**
 * Remove all the children of the parent
 * @param self The liqcell to remove the children of
 * @return int Success or Failure
 */
int liqcell_child_removeall(liqcell *self)
{
	liqcell *c=liqcell_getlinkchild(self);
	while(c)
	{
		liqcell *d = c->linknext;
		liqcell_child_remove(self,c);
		c=d;
	}
	return 0;
}

/**
 * Remove all visible children
 * @param self The liqcell to remove the children of
 * @return int Success or Failure
 */
int liqcell_child_removeallvisual(liqcell *self)
{
	liqcell *c=liqcell_getlinkchild(self);
	while(  c   )
	{
		liqcell *d = c->linknext;
		if( liqcell_getflagvisual(c) ) liqcell_child_remove(self,c);
		c=d;
	}
	return 0;
}

/**
 * Remove all children of a specified class
 * @param self The liqcell to remove the children of
 * @return int Success or Failure
 */
int liqcell_child_removeallclass(liqcell *self, char *classname)
{
	liqcell *c=liqcell_getlinkchild(self);
	while(  c   )
	{
		liqcell *d = c->linknext;
		if( liqcell_isclass(c,classname) ) liqcell_child_remove(self,c);
		c=d;
	}
	return 0;
}


/**
 * Remove a child from the provided parent
 * @param self The parent to remove the child from
 * @param child The child to remove
 * @return int Success or Failure
 */
int  liqcell_child_remove(liqcell *self,liqcell *child)
{
	// remove specified child, heh, leave this for now
		//liqapp_log("meh %s",c->name);
		
		// unlink child from the tree
		
		if(self != child->linkparent)
		{
			// child does not belong to me
			return -1;
		}
		
		
		liqcell *cprev = child->linkprev;
		liqcell *cnext = child->linknext;
		if(cprev) cprev->linknext = cnext;
		if(cnext) cnext->linkprev = cprev;

		child->linkprev=NULL;
		child->linknext=NULL;
		child->linkparent=NULL;


		
		if(self->linkchild==child)
		{
			if(cprev)
			{
				self->linkchild=cprev;
			}
			else
			{
				self->linkchild=cnext;
			}			
		}
		
		liqcell_release(child);
		return 0;
}

/**
 * Insert a child liqcell in a sorted fashion
 * @param self The liqcell to insert a child into
 * @param ch The child to insert
 * @return liqcell* Child liqcell
 */
liqcell* liqcell_child_insertsorted(liqcell *self,liqcell * ch)
{
	if(!ch)return NULL;
	// insert sorted
	if(!self->linkchild || !ch->name) // filename)
	{
		// first child or no name to sort on
		//liqapp_log("append '%s'",ch->name);
		liqcell_child_append(self,ch);
		return ch;// ch;
	}
	else
	{
		liqcell *sa=ch;
		liqcell *xx=self->linkchild;
		while(xx)
		{
			if(xx->name && strcasecmp(sa->name,xx->name)< 0)
			{
				// insert it here...
				//liqapp_log("insert before '%s' < '%s'",sa->name,xx->name);
				if(xx==self->linkchild)
				{
					// first child
					self->linkchild = sa;

					sa->linkparent = self;
					sa->linkprev = xx->linkprev;
					sa->linknext = xx;

					xx->linkprev = sa;
					//self->childcount++;
					//liqcell_hold(ch);
					liqcell_setdirty(self,1);
					return sa;// sa;
				}
				else
				{
					// in the middle
					liqcell *yy=xx->linkprev;

					sa->linkparent = self;
					sa->linkprev = yy;
					sa->linknext = xx;

					yy->linknext = sa;

					xx->linkprev = sa;
					//self->childcount++;
					//liqcell_hold(ch);
					liqcell_setdirty(self,1);
					return sa;// sa;
				}
			}
			xx=xx->linknext;
		}

		//liqapp_log("at end '%s'",ch->name);
		liqcell_child_append(self,ch);
		return ch;// ch;
	}
	return ch;// ch;
}

/**
 * Insert a child into a parent, sorted by name
 * @param self The parent
 * @param ch The child
 * @param sortpositive 
 * @return liqcell* Child
 */
liqcell* liqcell_child_insertsortedbyname(liqcell *self,liqcell * ch,int sortpositive)
{
	if(!ch)return NULL;
	// insert sorted
	if(!self->linkchild || !ch->name) // filename)
	{
		// first child or no key to sort on
		liqcell_child_append(self,ch);
		return ch;
	}
	else
	{
		liqcell *sa=ch;
		liqcell *xx=self->linkchild;
		while(xx)
		{
			if(xx->name)
			{
				int res=strcasecmp(sa->name,xx->name);
				if(!sortpositive)res=-res;
				if(res < 0)
				{
					// insert it here...
					if(xx==self->linkchild)
					{
						// first child
						self->linkchild = sa;
						sa->linkparent = self;
						sa->linkprev = xx->linkprev;
						sa->linknext = xx;
						xx->linkprev = sa;
						//self->childcount++;
						//liqcell_hold(ch);
						liqcell_setdirty(self,1);
						return sa;
					}
					else
					{
						// in the middle
						liqcell *yy=xx->linkprev;
						sa->linkparent = self;
						sa->linkprev = yy;
						sa->linknext = xx;
						yy->linknext = sa;
						xx->linkprev = sa;
						//self->childcount++;
						//liqcell_hold(ch);
						liqcell_setdirty(self,1);
						return sa;
					}
				}
			}
			xx=xx->linknext;
		}
		liqcell_child_append(self,ch);
		return ch;
	}
	return ch;
}


//#########################################################################
//#########################################################################
//######################################################################### cell child search and lookup
//#########################################################################
//#########################################################################

//liqcell* liqcell_child_find(liqcell *self,char *query)
//{
//	// use a search query to match (may involve patterns)
//	if(!self->linkchild) return NULL;
//	return liqcell_findfirst(self->linkchild,query);
//}

/**
 * Look for a child with a simple name such as "zachchild"
 * @param self The liqcell to lookup the child
 * @param name The name of the child to find
 * @return liqcell* The found child or NULL
 */
liqcell* liqcell_child_lookup_simple(liqcell *self,char *name)
{
	// 20090615_031345 lcuk : ignore dot name in this variation

	liqcell *c=self->linkchild;
	while(c)
	{
		//liqapp_log("test %s==%s == %i",c->name,name,strcmp(c->name,name));
		if(strcmp(c->name,name)==0)
		{
			return c;
		}
		c=c->linknext;
	}
	return NULL;
}

/**
 * Look for a child with a non-simple name such as "zachparent.zachchild"
 * @param self The liqcell to lookup the child
 * @param name The name of the child to find
 * @return liqcell* The found child or NULL
 */
liqcell* liqcell_child_lookup(liqcell *self,char *name)
{
	// find a named child
	char *dot=strchr(name,'.');
	if(dot)
	{
		//liqapp_log("dot1 %s",dot);
		char *buf=strndup(name,dot-name);
		if(buf)
		{
			//liqapp_log("dot2 %s,buf '%s'",dot,buf);
			liqcell *x = liqcell_child_lookup(self,buf);
			
			//liqapp_log("dot3 %s",dot);
			free(buf);
			if(x)
			{
				//liqapp_log("dot4 %s",dot);
				liqcell *y = liqcell_child_lookup(x,dot+1);
				//liqapp_log("dot5 %s",dot);
				return y;
			}
			//liqapp_log("liqcell_child_lookup '%s' not found '%s'",self->name,name);
			return NULL;
		}
		// invalid string
		//liqapp_log("liqcell_child_lookup failed strdup");
		return NULL;
	}
	
	liqcell *c=self->linkchild;
	while(c)
	{
		//liqapp_log("test %s==%s == %i",c->name,name,strcmp(c->name,name));
		if(strcmp(c->name,name)==0)
		{
			return c;
		}
		c=c->linknext;
	}
	//liqapp_log("liqcell_child_lookup '%s' not found '%s'",self->name,name);
	return NULL;
}

/**
 * Look for a child based on the name and classname
 * @param self The liqcell to lookup the child
 * @param name The name
 * @param classname The classname
 * @return liqcell* The found liqcell or NULL
 */
liqcell*  liqcell_child_lookup_nameclass(liqcell *self,char *name,char *classname)
{
	//liqapp_log("find a child called '%s:%s'",name,classname);
	liqcell *c=self->linkchild;
	while(c)
	{
		//liqapp_log("test %s==%s == %i",c->name,name,strcmp(c->name,name));
		if(strcmp(c->name,name)==0 && strcmp(c->classname,classname)==0)
		{
			return c;
		}
		c=c->linknext;
	}
	return NULL;
}

/**
 * Local lookup liqcell child
 * @param self The liqcell to lookup the child
 * @param name The name of the child to find
 * @return liqcell* The found child or NULL
 */
liqcell *liqcell_local_lookup(liqcell *self,char *name)
{
	liqcell *p=self;
	//while(p)
	{
		liqcell *c = liqcell_child_lookup(p,name);
		if(c) return c;
		//p=liqcell_getcontent(p);
	}
	return NULL;
}

/**
 * Local lookup nameclass
 * @param self The liqcell to lookup the child
 * @param name The name of the child to find
 * @return liqcell* The found child or NULL
 */
liqcell*  liqcell_local_lookup_nameclass(liqcell *self,char *name,char *classname)
{
	liqcell *p=self;
	//while(p)
	{
		liqcell *c = liqcell_child_lookup_nameclass(p,name,classname);
		if(c) return c;
		//p=liqcell_getcontent(p);
	}
	return NULL;
}

/**
 * Global lookup liqcell child
 * @param self The liqcell to lookup the child
 * @param name The name of the child to find
 * @return liqcell* The found child or NULL
 */
liqcell*  liqcell_global_lookup(liqcell *self,char *name)
{
	liqcell *p=self;
	while(p)
	{
		liqcell *c = liqcell_local_lookup(p,name);
		if(c) return c;
		p=liqcell_getlinkparent(p);
	}
	return NULL;
}

/**
 * Global lookup liqcell nameclass
 * @param self The liqcell to lookup the child
 * @param name The name of the child to find
 * @return liqcell* The found child or NULL
 */
liqcell*  liqcell_global_lookup_nameclass(liqcell *self,char *name,char *classname)
{
	liqcell *p=self;
	while(p)
	{
		liqcell *c = liqcell_local_lookup_nameclass(p,name,classname);
		if(c) return c;
		p=liqcell_getlinkparent(p);
	}
	return NULL;
}

//#########################################################################
//#########################################################################
//######################################################################### cell findfirst findnext for sequential pattern matching
//#########################################################################
//#########################################################################


//liqcell* liqcell_findfirst(liqcell *self,char *query)
//{
//	// use a search query to match ALONG our chain, including self
//	return NULL;
//}

//liqcell* liqcell_findnext(liqcell *self,char *query)
//{
//	// use a search query to match ALONG our chain
//	return NULL;
//}

/**
 * Create new liqcell, set visible and other attributes (widget)
 * @param name The name of the new liqcell
 * @param classname The classname of the new liqcell
 * @param innerw Inner width
 * @param innerh Inner height
 * @return liqcell* The new liqcell
 */
liqcell*  liqcell_quickcreatewidget(char *name,char *classname,int innerw,int innerh)
{
	liqcell *self = liqcell_new();
	if(classname && *classname)
	{
		if( (strcmp(classname,"form")==0) )
		{
			// Sat Aug 29 23:54:16 2009 lcuk : stupid thing, i gave everything as a form initially
			classname=name;
		}
	}
	liqcell_setname(self,name);
	liqcell_setclassname(self,classname);
	liqcell_setrect(self,0,0,innerw,innerh);
	liqcell_setinnersize_from_outer(self);		// this ensures that widgets have an initial scale
	self->visible=1;
	self->kind |= cellkind_visual;
	self->kind |= cellkind_widget;
	return self;
}

/**
 * Create a liqcell and set the name and classname. Also define the starting position (x,y)
 * and set the width and height. Set it to visible right away
 * 
 * @param name The name of the liqcell
 * @param classname The class that the liqcell belongs to.
 * @param x Starting X-Coordinate
 * @param y Starting Y-Coordinate
 * @param w Width of the liqcell
 * @param h Height of the liqcell
 * @return liqcell* The created liqcell
 * 
 */
liqcell*  liqcell_quickcreatevis(char *name,char *classname,int x,int y,int w,int h)
{
	liqcell *self=NULL;	
	if(classname && *classname)
	{
		// gulp!
		liqcell *dllcache_runconstructor(char *classname);
		self = dllcache_runconstructor(classname);
	}
	if(!self)
	{
		self = liqcell_new();
		self->kind |= cellkind_visual;
		self->visible=1;
		liqcell_setclassname(self,classname);
	}
	
	
	
	liqcell_setname(self,name);
	if((w>0) || (h>0))	liqcell_setrect(self,x,y,w,h);     // 20090422_184637 lcuk : only set the dimensions when we have valid ones

	return self;
}

/**
 * Create a liqcell and set the name and classname only.
 * @param name The name of the liqcell
 * @param classname The class that the liqcell belongs to.
 * @return liqcell* The created liqcell
 * 
 */
liqcell*  liqcell_quickcreatenameclass(char *name,char *classname)
{
	liqcell *self = liqcell_new();
	liqcell_setname(self,name);
	liqcell_setclassname(self,classname);
	self->kind |= cellkind_prop;
	return self;
}




/**
 * Create a liqcell and set the name, classname, and the data.
 * @param name The name of the liqcell
 * @param classname The class that the liqcell belongs to
 * @param data The data to be held by the liqcell, but is not realloced or released, it is under your control.
 * @return liqcell* The created liqcell
 * 
 */
liqcell*  liqcell_quickcreatedata(char *name,char *classname,void *data)
{
	liqcell *self = liqcell_new();
	liqcell_setname(self,name);
	liqcell_setclassname(self,classname);
	liqcell_setdata(self,data);
	self->kind |= cellkind_prop;
	return self;
}

/**
 * Create a liqcell and set the name, classname, and the caption.
 * @param name The name of the liqcell
 * @param classname The class that the liqcell belongs to
 * @param caption The data to be copied and stored with the cell.
 * @return liqcell* The created liqcell
 * 
 */
liqcell*  liqcell_quickcreatecaption(char *name,char *classname,char *caption)
{
	liqcell *self = liqcell_new();
	liqcell_setname(self,name);
	liqcell_setclassname(self,classname);
	liqcell_setcaption(self,caption);
	self->kind |= cellkind_prop;
	return self;
}

/**
 * Fully create a liqcell that sets the name, classname, data, and context.
 * @param name The name of the liqcell
 * @param classname The class that the liqcell belongs to
 * @param context Describes the context of the liqcell
 * @param data The data to be held by the liqcell
 * @return liqcell* The created liqcell
 * 
 */
liqcell *liqcell_quickcreatefull(char *name,char *classname,char *context,void *data)
{
	liqcell *self = liqcell_new();
	liqcell_setname(self,name);
	liqcell_setclassname(self,classname);
	liqcell_setcontext(self,context);
	liqcell_setdata(self,data);
	self->kind |= cellkind_prop;
	return self;
}

int liqcell_iskind(liqcell *self,int cellkind)
{
	// 20090408_223641 lcuk : this was testing OR instead of AND tsk tsk, not noticed till I started using it
	return (self->kind & cellkind);
}


//#########################################################################
//#########################################################################
//######################################################################### cell test
//#########################################################################
//#########################################################################

/**
 * Helper function for liqcell_print2
 * @param self The provided liqcell
 * @param title Title in printinf the tree
 * @param recdep Used in the recursive loop to generate indention
 */
void liqcell_print(liqcell *self,char *title,int recdep)
{
	char *indent = (char *)malloc(recdep+1);
	if(!indent) return;
	int i;
	for(i=0;i<recdep;i++)
	{
		indent[i]=' ';
	}
	indent[recdep]=0;
	
	
	char cap[32];
	snprintf(cap,30,"%s",(self->caption?self->caption:""));
	cap[31]=0;
	char *cc=cap;
	while(*cc)
	{
		if(*cc==10 || *cc==13 || *cc=='\t')*cc=' ';
		cc++;
	}

	//char buf[1024]="\0";
	//liqtile_fullyqualifiedkey(self,buf,1024);
	liqapp_log("%15s :: (%3i,%3i)-(%3i,%3i) %s %s:%s %s",title,self->x,self->y,self->w,self->h,  indent, (self->name?self->name:""),(self->classname?self->classname:"") ,cap );

	free(indent);

	liqcell *xx=self->linkchild;
	while(xx)
	{
		liqcell_print(xx,title,recdep+1);
		xx=xx->linknext;
	}

}

/**
 * Print a tree of liqcells starting with the provided liqcell
 * @param self The provided liqcell
 */
void liqcell_print2(liqcell *self)
{
	static int recdep=0;
	liqcell_print(self,"self",recdep*4);
	if(recdep>=2)return;
	recdep++;
	liqcell *xx=self->linkchild;
	while(xx)
	{
		liqcell_print2(xx);
		xx=xx->linknext;
	}
	recdep--;
}


int strcmpx (const char * s1, const char * s2)
{

    for(; *s1 == *s2; ++s1, ++s2)
        if(*s1 == 0)
            return 0;
    return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}

/**
 * Search for handler
 * @param self The liqcell to search for a handler for
 * @param handlername The name of the handler to find
 * @return void* The handler
 */
void*  liqcell_handlerfind(liqcell *self,char *handlername)
{
	liqcell *p=self;
	// only check self
	//while(p)
	{
		liqcell *c=p->linkchild;
		while(c)
		{
			if(c->name && c->classname)
			{
				//liqapp_log("mmm %s:%s %i",c->name,c->classname,(int)c->data);

				if((strcmp(c->classname,"handler")==0) && (strcasecmp(c->name, handlername)==0) )
				{
					// found it!
					//liqapp_log("ok %s:%s %i",c->name,c->classname,(int)c->data);
					return  c->data;
				}
			}
			c=liqcell_getlinknext(c);
		}
	//	p=liqcell_getlinkcontent(p);
	}

	return NULL;
}


/**
 * Add a handler by creating a child liqcell for the provided parent. The nameclass must
 * be called "handler" to be identified as a handler liqcell.
 * 
 * @param self The liqcell to add a handler to
 * @param handlername The name of the handler
 * @param handler Function pointer to actual handler
 * @return liqcell Newly created child liqcell
 * 
 */
liqcell*  liqcell_handleradd(liqcell *self,char *handlername, void *handler)
{
	//liqapp_log("add %s:%s %i",handlername,"handler",(int)handler);

	return liqcell_child_insert(self, liqcell_quickcreatedata(handlername,"handler",handler) );
}

/**
 * Add a handler by creating a child liqcell for the provided parent. The nameclass must
 * be called "handler" to be identified as a handler liqcell.
 * 
 * @param self The liqcell to add a handler to
 * @param handlername The name of the handler
 * @param handler Function pointer to actual handler
 * @return liqcell Newly created child liqcell
 * 
 */
liqcell*  liqcell_handleradd_withcontext(liqcell *self,char *handlername, void *handler,void *context)
{
	//liqapp_log("add %s:%s %i",handlername,"handler",(int)handler);
	liqcell *h = liqcell_quickcreatedata(handlername,"handler",handler);
	liqcell_settag(h,context);
	return liqcell_child_insert(self, h );
}


/**
 * Run/Execute the handler of the provided liqcell. This only executes the handler
 * provided by "handlername".
 * 
 * @param self The liqcell with the handler
 * @param handlername The name of the handler to run
 * @return int 1 for Successful handler run, which means each handler must return 1
 * for success
 * 
 */
int liqcell_handlerrun(liqcell *self,char *handlername,void *args)
{
	int (*event_handler)(liqcell *cell,void *args,void *context);
	liqcell *p=self;
	
	// 20090618_020708 lcuk : make sure we keep hold of reference for this part
	// 20090618_020717 lcuk : i just realised i want to collapse all the tree from within an event
	// 20090618_020733 lcuk : and without this guard, the usagecount should hit 0 and the memory released
	// 20090618_020808 lcuk : but some of the time i carry on looping
	//liqcell_hold(self);
	
	//while(p)
	{
		liqcell *c=p->linkchild;
		while(c)
		{
			if(c->name && c->classname)
			{
				//liqapp_log("mmm ?? %s == %s:%s %i",handlername, c->name,c->classname,(int)c->data);

				if((strcmp(c->classname,"handler")==0) && (strcasecmp(c->name, handlername)==0) )
				{
					// found it!
					//liqapp_log("ok %s:%s %i",c->name,c->classname,(int)c->data);
					void *context = liqcell_gettag(c);
					event_handler = (int (*)(liqcell*, void*, void*))c->data;
					if(event_handler)
					{
						
						//if(context) liqcell_hold(context);

						
						// Run/Execute the handler
						int res = event_handler(self,args,context);
						
						//if(context) liqcell_release(context);
						
						
						if(res)
						{
							// event was handled...
							//liqcell_release(self);
							return res;
						}
					//	return res;		// if the event existed, we run it, no consequence, later, inc ref count and allow recursive multiple event chains
					// 20100129 lcuk removed the return, this should be fun.
					}
					//return 0;
				}
			}
			c=liqcell_getlinknext(c);
		}
		//p=liqcell_getcontent(p);
	}
	//liqcell_release(self);
	return 0;
}

/**
 * Get the base (top parent) of the provided liqcell
 * @param self The provided liqcell
 * @return liqcell* The base liqcell
 */
liqcell *liqcell_getbasewidget(liqcell *self)
{
	// called from within an event
	// steps backwards until it finds the base widget this item was created by
	
	liqcell *c=self;
	while(c)
	{
		if(liqcell_iskind(c,cellkind_widget)) return c;
		c=liqcell_getlinkparent(c);
	}
	return NULL;
}

/**
 * Check if the string provided is a classname of the liqcell provided
 * @param self The provided liqcell
 * @param classname The string to cechk
 * @param int True or False
 */
int liqcell_isclass(liqcell *self,char *classname)
{
	// return if this cell is a member of a class
	return ( (self) && (self->classname) && (strcasecmp(self->classname, classname)==0) );
}



//##############################################################
//##############################################################
//##############################################################


static int lowest(int a,int b)
{
	if(abs(a)<abs(b))
	{
		liqapp_log("lowest %i :: %i = a %i",a,b,a);
		return a;
	}
	liqapp_log("lowest %i :: %i = b %i",a,b,b);
	return b;
}


static int dimension_ensurevisible( int rs,int re,    int ps,int pe, int ss,int se)
{
	// calculate the minimal adjustment within a dimension required to ensure S is visible through the portal that R provides
	// the adjustment will be applied to P upon returning from this function
	// to slide the rule along so s is visible :)
	//ss += ps;	// start by adjusting 
	//se += pe;
	liqapp_log("dim ol: r(%i,%i)   p(%i,%i)    s(%i,%i)",   rs,re,     ps,pe,     ss,se);
	if(re<=ss)
	{
		// S is way below, lets adjust
		return lowest(ss-rs,se-re);
	}
	if(rs<=ss)
	{
		// S is actually somewhat visible
		// but we might be chopping off the bottom of it
		if(re<=se)
		{
			return lowest(ss-rs,se-re);
		}
		// otherwise we let it be, floating somewhere within
		return 0;
	}

	// S is partially or entirely above us
	return lowest(ss-rs,se-re);
}

/**
 * Adjust a position of a liqcell to ensure it is visible
 * @param self The liqcell to adjusts
 * @return Success or Failure
 */
int liqcell_ensurevisible(liqcell *self)
{
	liqapp_log("ensure: %s",self->name);
	int xs=self->x;
	int xe=self->x+self->w;
	int ys=self->y;
	int ye=self->y+self->h;
	
	liqcell *p=liqcell_getlinkparent(self);
	//while(p)
	if(p)
	{
		liqcell *r=liqcell_getlinkparent(p);
		if(r)
		{
			liqapp_log("trying in : %s",p->name);
			xs+=p->x;
			xe+=p->x;
			ys+=p->y;
			ye+=p->y;

			//
			int ax = -dimension_ensurevisible(0,r->w,   p->x,p->x+p->w,   xs,xe);
			int ay = -dimension_ensurevisible(0,r->h,   p->y,p->y+p->h,   ys,ye);


			liqapp_log("gave me : a(%i,%i)",  ax,ay);

			liqcell_adjustpos(p,ax,ay);
			xs-=ax;
			xe-=ax;
			ys-=ay;
			ye-=ay;

		}
		//p=r;
	}
	return 0;
}

/**
 * Adjust a position of a liqcell to ensure it is visible and centered
 * @param self The liqcell to adjusts
 * @return int Success or Failure
 */
int liqcell_ensurevisible_centred(liqcell *self)
{
	liqapp_log("ensure: %s",self->name);
	int xs=self->x;
	int xe=self->x+self->w;
	int ys=self->y;
	int ye=self->y+self->h;
	
	liqcell *p=liqcell_getlinkparent(self);
	//while(p)
	if(p)
	{
		liqcell *r=liqcell_getlinkparent(p);
		if(r)
		{
			liqapp_log("trying in : %s",p->name);
			xs+=p->x;
			xe+=p->x;
			ys+=p->y;
			ye+=p->y;

			//
			int ax = -dimension_ensurevisible(r->w*0.5,r->w*0.5,   p->x,p->x+p->w,   xs,xe);
			int ay = -dimension_ensurevisible(r->h*0.5,r->h*0.5,   p->y,p->y+p->h,   ys,ye);


			liqapp_log("gave me : a(%i,%i)",  ax,ay);

			liqcell_adjustpos(p,ax,ay);
			xs-=ax;
			xe-=ax;
			ys-=ay;
			ye-=ay;

		}
		//p=r;
	}
	return 0;
}



//#########################################################################
//#########################################################################
//######################################################################### cell test
//#########################################################################
//#########################################################################

/**
 * Testing function
 */
void liqcell_test()
{

	liqcell *root = liqcell_quickcreatenameclass("root","frame");


	liqcell *f1 = liqcell_child_insert(root,liqcell_quickcreatenameclass("f1","frame"));
		liqcell_child_insert(f1,liqcell_new());
		liqcell_child_insert(f1,liqcell_new());
		liqcell_child_insert(f1,liqcell_new());


	liqcell *f2 = liqcell_child_insert(root,liqcell_quickcreatenameclass("f2","frame"));
		liqcell_child_insert(f1,liqcell_new());
		liqcell_child_insert(f1,liqcell_new());
		liqcell_child_insert(f1,liqcell_new());


	liqcell *f3 = liqcell_child_insert(root,liqcell_quickcreatenameclass("f3","button"));


	liqcell *f4;
	f4=f3;
	f4=f2;
	f4=f1;


	liqcell_release(root);

	// should i dictate a test is required for every class?
	// seems logical..
}
