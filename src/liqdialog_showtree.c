

#include <string.h>



#include "liqbase.h"
#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqcell_easypaint.h"
#include "liqcell_mk_star.h"
#include "liqcell_easyhandler.h"

#ifdef __cplusplus
extern "C" {
#endif




static const int treefontsize=16;
static const int treeindent=25;



//#####################################################################
//#####################################################################
//##################################################################### deepview tree nodes
//#####################################################################
//#####################################################################





static int deepviewitem_nobble_click(liqcell *self, liqcellclickeventargs *args,void *context)
{
	//liqapp_log("nobble");
	
	liqcell *deepview = liqcell_getbasewidget(self);
	//liqapp_log("nobble1");
	if(!deepview)return -1;
	
	//liqapp_log("nobble2");
	liqcell *head = liqcell_local_lookup(deepview,"head");
	if(!head)return -2;
	
	//liqcell *preview = liqcell_local_lookup(head,"preview");
	
	
	
	//liqapp_log("nobble3");
	liqcell *childmat = liqcell_local_lookup(deepview,"childmat");
	//liqapp_log("nobble4");
	
	if(!childmat)
	{
		// we MUST be clicking open
		// lets get hold of our context and create the whole branch
		//liqapp_log("nobble5");
	}
	else
	{
		//liqapp_log("nobble6");
		if(liqcell_getvisible(childmat))
		{
			// already visible, we MUST be hiding it
			//liqapp_log("nobble7");
			liqcell_setvisible(childmat,0);
						liqcell_setimage(  self, liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/add.png" ,0,0,1) );

		}
		else
		{
			// not visible, we must be showing it
			//liqapp_log("nobble8");
			liqcell_setvisible(childmat,1);
						liqcell_setimage(  self, liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/gtk-remove.png" ,0,0,1) );
		}
	}
	
	while(deepview && childmat)
	{
		//liqapp_log("nobble9 %s",deepview->name);
		liqcell_child_arrange_easycol(childmat);
		liqcell_child_arrange_easycol(deepview);
		childmat->x=treeindent;
		
		//liqapp_log("nobble10");
		
		// walk backwards for as long as we have
		liqcell *par = liqcell_getlinkparent(deepview);
		
		//liqapp_log("nobble11 %s:%s",par->name,par->classname);
		childmat=NULL;
		deepview=NULL;
		if(par && liqcell_isclass(par,"childmat"))
		{
			//liqapp_log("nobble12 %s:%s",par->name,par->classname);
			childmat = par;
			deepview = liqcell_getbasewidget(par);
			par=NULL;
		}
		
		if(par && liqcell_isclass(par,"deepview"))
		{
			//liqapp_log("nobble13 %s:%s",par->name,par->classname);
			//childmat = par;
			liqcell_child_arrange_easycol(par);
			//deepview = liqcell_getbasewidget(par);			
		}		
	}
	
	//liqapp_log("nobble14");
	return 0;
}



static int deepviewitem_click(liqcell *self, liqcellclickeventargs *args,void *context)
{
	//liqcell_setselected(self);
	//self->selected=1;
	
	// work back to the deepview..
	// recurse and clear all selection..
	
	if(self->selected)
	{
		self->selected=0;
		//liqcell_setimage(  self, NULL );
		
		liqcell_propsets(  self,"textcolor","rgb(255,255,255)" );
		liqcell_propsets(  self,"backcolor","rgb(0,0,0)" );
	}
	else
	{
		self->selected=1;
		//liqcell_setimage(  self, liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/blob.png" ,0,0,1) );
		
		liqcell_propsets(  self,"textcolor","rgb(255,255,0)" );
		liqcell_propsets(  self,"backcolor","rgb(0,0,255)" );

	}
	return 1;
}


//static liqcell *deepview(liqcell *node,int recdep);

static liqcell *deepviewitem(liqcell *node,int recdep)
{
	int xl=0;
	int ww=800;
	
	if(recdep<8)
	{
		xl=recdep*treeindent;
	}
	else
	{
		xl=8*treeindent;
	}
	ww=800-xl;
	
	
	liqfont *font = liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (treefontsize), 0);
	liqfont_setview(font,1,1);
	
	
	int fh = liqfont_textheight(font);
	
	// deepview
	// deepview.head
	// deepview.head.plusminus
	// deepview.head.preview
	// deepview.head.title
	// deepview.childmat
	
	liqcell *self = liqcell_quickcreatewidget(node->name, "deepviewitem", ww, fh);
	
	
		//liqcell_propsets(     self,	"backcolor", "rgb(255,255,255)"  );
		liqcell *head = liqcell_quickcreatevis("head", "panel", 0, 0, self->w, self->h);


			//########################################## create the nobble
			liqcell *nobble = liqcell_quickcreatevis("plusminus", "button", 0, 0, treeindent, fh);
			liqcell_handleradd(   nobble,   "click",   (void *)deepviewitem_nobble_click);
			if(recdep<20)
			{
				// already expanded..
						liqcell_setimage(  nobble, liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/gtk-remove.png" ,0,0,1) );
						liqcell_propseti(  nobble,"lockaspect",1);
			}
			else
			{
				// not yet open
				if(liqcell_getlinkchild(node) || liqcell_getcontent(node))
				{
					// there are children to recurse later
						liqcell_setimage(  nobble, liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/add.png" ,0,0,1) );
						liqcell_propseti(  nobble,"lockaspect",1);
				}
				else
				{
					// no children of any sort under neath us..
					// leave it blank, no need to have anything shown here
					liqcell_setvisible(nobble,0);
				}
				
			}
			liqcell_child_insert(head, nobble);
			

		
		
		
			//########################################## create the preview
			liqcell *preview = liqcell_quickcreatevis("preview", "icon", treeindent, 0, treeindent, fh);
			// if its a visible cell we add the preview
			// if not, we can add an icon based on its classname
			
			// 20090414_012255 lcuk : set the content NO MATTER WHAT KIND OF ITEM IT IS!
			// 20090414_012306 lcuk : this is important becase we will be using it later
			liqcell_setcontent(preview, node);
			
			if(liqcell_getvisible(node))
			{
				// live preview
				liqcell_propseti(  preview,"lockaspect",1);
			}
			else
			{
				if(liqcell_getlinkchild(node))
				{
					// has contents underneath
					liqcell_setimage(  preview, liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/folder.png" ,0,0,1) );
					liqcell_propseti(  preview,"lockaspect",1);
				}
				else
				{
					// is terminal node
					liqcell_setimage(  preview, liqimage_cache_getfile( "/usr/share/liqbase/libliqbase/media/txt.png" ,0,0,1) );
					liqcell_propseti(  preview,"lockaspect",1);
				}
			}
			liqcell_child_insert(head, preview);
			
			
			//########################################## create the caption itself
			liqcell *tl = liqcell_quickcreatevis(liqcell_getname(node), "label", 100, 0, (300-xl), fh);
			liqcell_setfont(tl, liqfont_hold(font));
			liqcell_handleradd(   tl,   "click",   (void *)deepviewitem_click);
			//liqcell_propsets(tl,	"textcolor", "rgb(0,0,0)"  );
			
			//liqcell_propsets(  self,"textcolor","rgb(255,255,255)" );
			//liqcell_propsets(  self,"backcolor","rgb(0,0,0)" );
			liqcell_child_insert(head, tl);


			liqcell *tr = liqcell_quickcreatevis(liqcell_getcaption(node), "label", 400-xl, 0, 400, fh);
			liqcell_setfont(tr, liqfont_hold(font));
			liqcell_handleradd(   tr,   "click",   (void *)deepviewitem_click);			
			liqcell_propsets(  tr,"textcolor","rgb(255,255,0)" );
			liqcell_child_insert(head, tr);

			
		
		liqcell_child_insert(self, head);
		
		
		
		
		
		
		//liqcell_propsets(self, "bordercolor", "rgb(255,0,0)");
	
	
		int yy=0;
		
		
		liqcell *c;
		if(recdep<20)
		{
			
			// 20090414_012420 lcuk : create a mat for the children to sit on
			liqcell *childmat = liqcell_quickcreatevis("childmat", "childmat", treeindent, self->h, self->w-treeindent, 0);
			
			int cntvis=0;
			
				c = liqcell_getcontent(node);
				if(c)
				{
					if(liqcell_getflagvisual(c))cntvis++;


					liqcell *r = deepviewitem(c,recdep+1);
					r->x=00;
					r->y=yy;
					
					yy+=r->h;
					childmat->h+=r->h;
					self->h+=r->h;
					
					liqcell_child_append(childmat, r);
				}
				
				c = liqcell_getlinkchild(node);
				while(c)
				{
					if(liqcell_getflagvisual(c))cntvis++;
					
					
					liqcell *r = deepviewitem(c,recdep+1);
					r->x=00;
					r->y=yy;
					
					yy+=r->h;
					childmat->h+=r->h;
					self->h+=r->h;
					
					liqcell_child_append(childmat, r);
			
					c=liqcell_getlinknext(c);
				}
				
				

				
			liqcell_child_append(self, childmat);
			
				if(cntvis==0)
				{
					// FOLD UP!
					deepviewitem_nobble_click(childmat,NULL,NULL);
				}			
		}
	return self;
}


static liqcell *deepview(liqcell *node,int recdep)
{
	liqcell *root=deepviewitem(node,recdep);
	if(!root)
	{
		liqapp_log("deepview, couldnt create root");
		return NULL;
	}
	
	liqcell *self = liqcell_quickcreatewidget(node->name, "deepview", root->w, root->h);
	if(self)
	{
		liqcell_handleradd(self, "mouse", (void *)liqcell_easyhandler_kinetic_mouse);
		liqcell_child_append(self, root);
	}
	return self;
}























//#####################################################################
//#####################################################################
//##################################################################### liqdialog_showtree
//#####################################################################
//#####################################################################






int liqdialog_showtree(char *key,char *title,char *description,liqcell *data)
{
	//
	
	
		//######################################### create the main instance
	liqcell *self = liqcell_quickcreatewidget("liqtreebrowse","form", 800,480);

	if(self)
	{
		//! create header
	//	liqcell *thead = uititlebar_create("thead",title,description);
	//	liqcell_child_append( self, thead    );	
		
		//! create body
		liqcell *tbody = liqcell_quickcreatevis("tbody",   "grid",    0,0,   800, 480);// - thead->h );//- tbar->h );
		//liqcell_handleradd(   tbody,   "shown",   liqtreebrowse_tbody_shown);

		//
		
		
		
		liqcell_child_append(tbody,  deepview(data,0)  );
		




		//liqcell_child_append( self, thead    );
		liqcell_child_append( self, tbody    );


		// now stack them..
		liqcell_child_arrange_easycol(self);
		
		
		// present it to the user
		liqcell_easyrun(self);
		
		// and release everything
		liqcell_release(self);
	}
	
	

	return 0;
}

#ifdef __cplusplus
}
#endif

