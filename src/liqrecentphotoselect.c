// /home/user/MyDocs/.camera


//  "/media/mmc1/svn/liqbase/libliqbase/media/jacobpics"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include "liqbase.h"
#include "liqcell.h"
#include "liqcell_dllcache.h"
#include "liqcell_easyrun.h"
#include "liqcell_easyhandler.h"
#include "liqcell_arrange.h"
#include "liqcell_easypaint.h"

#ifdef __cplusplus
extern "C" {
#endif

//#include "liqdialogs.h"


//static int viewtree_click(liqcell *self, liqcellclickeventargs *args, void *context)
//{
//	liqdialog_showtree("view tree","showing tree","",(liqcell *)context);
//}

    /**
     * get the first photograph listed.  does not have to be visible in the filter though
     */
	int liqrecentphotoselect_getfirstphoto_filename(liqcell *self,char *buffer,int bufferlen)
    {
        if(!self)
        {
            *buffer = '\0';
            return -1;
        }
        liqcell *body= liqcell_child_lookup(self, "body");
        if(!body)
        {
            *buffer = '\0';
            return -2;
        }
        liqcell *c=liqcell_getlinkchild_visual(body);
        if(!c)
        {
            *buffer = '\0';
            return -1;
        }
        snprintf(buffer,bufferlen,"%s",liqcell_propgets(c,"imagefilename",""));
        return 0;        
    }
    
    
    
    /**
     * get the photograph selected.  does not have to be visible in the filter though
     */
	int liqrecentphotoselect_getselectedphoto_filename(liqcell *self,char *buffer,int bufferlen)
    {
        if(!self)
        {
            snprintf(buffer,bufferlen,"x1");
            return -1;
        }
        liqcell *body= liqcell_child_lookup(self, "body");
        if(!body)
        {
            liqapp_log("mmm '%s:%s'",self->name,self->classname?self->classname:"NULL");
            snprintf(buffer,bufferlen,"x2");
            return -2;
        }
        
        liqcell *c=liqcell_getlinkchild_visual(body);
        while(c)
        {
            if(liqcell_getselected(c))
            {
                // start with big image
                char *s=liqcell_propgets(c,"imagefilenamebig",NULL);
                if(!s || !*s)
                {
                    // then try smaller one
                    s=liqcell_propgets(c,"imagefilename",NULL);
                }
                if(s)
                {
                    snprintf(buffer,bufferlen,"%s",s);
                }
                return 0;
            }
            c=liqcell_getlinknext_visual(c);
        }
        snprintf(buffer,bufferlen,"x3");
        return -2;
    }



int autothumb_getthumb(liqcell *self,char *bigfilename)
{
	// using the magical autothumb function, create a thumbnail from the filename
	
	char tmp[FILENAME_MAX]={0};
	
	strncpy(tmp,bigfilename,sizeof(tmp));
	liqapp_ensurecleanusername(tmp);
	
	char thumbfn[FILENAME_MAX]={0};
	
	snprintf(thumbfn,sizeof(thumbfn),"%s/thumbs/%s",app.userdatapath,tmp);
	
	liqapp_log("autothumb '%s'",thumbfn);
	if(!liqapp_fileexists(thumbfn))
	{
		//liqapp_log("autothumb '%s' no thumb yet",thumbfn);
		// must create a thumb
			//################################## load in a thumbnail of the image
			liqimage *imgfull = liqcell_getimage(self);   //bigimagepreloaded;//liqimage_newfromfile(bigfilename,0,0,1);
			if(imgfull)
			{
				//liqapp_log("autothumb '%s' has big image, thumbnailing",thumbfn);
				liqimage *imgthumb = liqimage_getthumbnail(imgfull,128,64);
				if(imgthumb)
				{
					//liqapp_log("autothumb '%s' made a thumb!",thumbfn);
					//################################## save it 
					
					if(liqimage_pagesavepng(imgthumb,thumbfn))
					{
						liqapp_log("autothumb_getthumb: could not store thumb buffer as: '%s'",thumbfn);
						liqimage_release(imgthumb);
						return NULL;
					}
					
					//liqapp_log("autothumb '%s' setting thumb",thumbfn);
					
					liqcell_propsets(self,"imagefilenamebig",bigfilename);
					
					liqcell_setimage(self,imgthumb);
					return 0;
					
					//return imgthumb;
					//liqimage_release(imgthumb);
				}
				//liqimage_release(imgfull);
			}
			else
			{
				// no thumbnail available, and nothing to work from, lets just wait
				return -1;
			}
	}
	
	liqapp_log("autothumb '%s' loading",thumbfn);
	
	liqcell_propsets(self,"imagefilenamebig",bigfilename);
	liqcell_propsets(self,"imagefilename",thumbfn);
	
	liqcell_threadloadimage(self);
	return 0;
	//return liqimage_newfromfile(thumbfn,0,0,1);
}

	
//#####################################################################
//#####################################################################
//##################################################################### liqrecentphotoselect :: by gary birkett 
//#####################################################################
//#####################################################################


















	static liqcell *quickdialog_create()
	{
		liqcell *self = liqcell_quickcreatewidget("editoverlay","edit", 800,480);
		if(self)
		{
			liqcell *c;
						c = liqcell_quickcreatevis("sketching",   "picture",   0,0,   800,480    );
						liqcell_propseti(c,"lockaspect",1);					
						liqcell_propsets(c,"imagefilename","media/lcuk_sig_headshot.png");
						//liqcell_handleradd(c,    "mouse",   widget_mouse);
						//liqcell_handleradd(c,    "click",   edit_click);
						liqcell_child_append( self, c );
		}
		return self;
	}
		
		
	
	int edit_click(liqcell *self, liqcellclickeventargs *args, void *context)
	{
		liqapp_log("hello click edit!");
		liqcell *mydialog = quickdialog_create();
		liqcell_easyrun(mydialog);
		return 1;
	}	
	
	
	//##########################################################################
	//########################################################################## latest, click event
	//##########################################################################

	/**	
	* liqrecentphotoselect_item dialog_open - the user zoomed into the dialog
	*/	
	static int liqrecentphotoselect_item_dialog_open(liqcell *self,liqcelleventargs *args, liqcell *context)
	{
		char *myimgnamebig = liqcell_propgets(self,"imagefilenamebig",NULL);
		if(myimgnamebig && *myimgnamebig)
		{
			// only set after a thumbnailing
			char *myimgname = liqcell_propgets(self,"imagefilename",NULL);
			if(myimgname && *myimgname)
			{
				// set most of the time
				liqapp_log("liqrecentphotoselect_item_dialog_open %i,  %s=%s",strcasecmp(myimgnamebig,myimgname),myimgnamebig,myimgname);
				if(strcasecmp(myimgnamebig,myimgname)==0)
				{
					// same, do nothing
				}
				else
				{
					// different!  reload mighty image
					liqcell_propsets(self,"imagefilename",myimgnamebig);
					//liqcell_propremoves(self,"imagefilenamebig");
					liqcell_threadloadimage(self);
				}				
			}
		}
				
	   return 0;
	}
	
	/**	
	* liqrecentphotoselect_item dialog_close - the dialog was closed
	*/	
	static int liqrecentphotoselect_item_dialog_close(liqcell *self,liqcelleventargs *args, liqcell *context)
	{
		char *myimgnamebig = liqcell_propgets(self,"imagefilenamebig",NULL);
		if(myimgnamebig && *myimgnamebig)
		{
			char *myimgname = liqcell_propgets(self,"imagefilename",NULL);
			if(myimgname && *myimgname)
			{
				if(strcasecmp(myimgnamebig,myimgname)==0)
				{
					// same
					//liqcell_propsets(self,"imagefilename",myimgnamebig);
					liqcell_propremoves(self,"imagefilenamebig");
					autothumb_getthumb(self,myimgname);
				}
				else
				{
					// different!
				}				
			}
		}
	   return 0;
	}

	//##########################################################################
	//########################################################################## latest, click event
	//##########################################################################

	static int liqrecentphotoselect_item_click(liqcell *self, liqcellclickeventargs *args, void *context)
	{
		// lets cheat a little
	//	liqcell_propseti(self,"imagewantbig",1);
	//	liqcell_setimage(self,NULL);
    
    
	//	args->newdialogtoopen = self;//liqcell_child_lookup( self, "body" );
    
        liqcell *p=liqcell_getlinkparent(self);
        liqcell *c=liqcell_getlinkchild_visual(p);
        while(c)
        {
            //if(c!=self)
                if(liqcell_getselected(c)) liqcell_setselected(c,0);
            c=liqcell_getlinknext_visual(c);
        }
        liqcell_setselected(self,1);
    /*
        if(liqcell_getselected(self))
        {
			liqcell_setselected(self,0);
			//liqcell_propremoves(self, "textcolor"   );
		//	liqcell_propremoves(self, "backcolor"   );
		//	liqcell_propremoves(self, "bordercolor" );
        }
        else
        {
			liqcell_setselected(self,1);
			//liqcell_propsets(self, "textcolor",   "rgb(0,255,0)" );
		//	liqcell_propsets(self, "backcolor",   "rgb(0,0,40)" );
		//	liqcell_propsets(self, "bordercolor", "rgb(255,255,255)" );
        }
    */    return 1;
	}

/*
	static int tagitem_click(liqcell *self, liqcellclickeventargs *args, void *context)
	{
		//liqapp_log("general click");
		//args->newdialogtoopen = self;

		return 1;
	}
 */

	//##########################################################################
	//########################################################################## later, lazy loaded event, try to replace with a thumb
	//##########################################################################
	
	static int liqrecentphotoselect_item_imageloaded(liqcell *self, liqcelleventargs *args, void *context)
	{
		// this occurs when the laxy loader has finished loading the image for this item
		liqapp_log("item loaded");
		liqimage *myimg = liqcell_getimage(self);
		if(myimg && liqcell_propgets(self,"imagefilenamebig",NULL)==NULL)
		{
			liqapp_log("item loaded, we loaded the full image, but we want the thumbnail");
			// the image assigned should be the BIG image
			// shall we throw it away and replace it with a thumb?
			// seems awfully wasted
			char *myimgname = liqcell_propgets(self,"imagefilename",NULL);
			if(myimgname && *myimgname)
			{
				liqapp_log("item loaded, got its filename");
			
				if(liqcell_propgeti(self,"imageisthumb",0) ==0)
				{
					liqapp_log("item loaded, its not a thumb yet");
					// this is not a thumbnail
					autothumb_getthumb(self,myimgname);
					/*
					if(thumb)
					{
						liqapp_log("item loaded, we got a thumb!");
						// replace the large image with a thumb :)
						liqcell_setimage(self,thumb);
						liqcell_propseti(self,"imageisthumb",1);
					}
					*/
				}

			}
		}
		
		return 0;
	}

	//##########################################################################
	//########################################################################## shown event, try to grab the thumb
	//##########################################################################

	static int liqrecentphotoselect_item_shown(liqcell *self, liqcelleventargs *args, void *context)
	{
		liqapp_log("item shown");
		liqimage *myimg = liqcell_getimage(self);
		if(!myimg)
		{
			liqapp_log("item shown, no img yet");
			char *myimgname = liqcell_propgets(self,"imagefilename",NULL);
			if(myimgname && *myimgname)
			{
				liqapp_log("item shown got filename though");
				autothumb_getthumb(self,myimgname);
				/*
				if(thumb)
				{
					liqapp_log("item shown, got a thumb!");
					// take a short cut!
					liqcell_setimage(self,thumb);
					liqcell_propseti(self,"imageisthumb",1);
				}
				*/
			}
		}
		
		return 0;
	}

static int liqcell_scan_folder_for_images(liqcell *self,char *path)
{
	liqcell *body= liqcell_child_lookup(self, "body");
	
		char *widgetpath = path;
		DIR           *	dir_p;
		struct dirent *	dir_entry_p;
		char 			fn[FILENAME_MAX+1];
		char          * ft;
		
		dir_p = opendir( widgetpath );			
		if(!dir_p)
		{
			liqapp_log("liqcell_scan_folder_for_images opendir failed: '%s'",widgetpath);
			return -1;			// heh thanks kot :)
		}				
		while( NULL != (dir_entry_p = readdir(dir_p)))
		{
			if( dir_entry_p->d_name[0]=='.' )
				continue;
			
			ft=dir_entry_p->d_name;
			
			snprintf(fn , FILENAME_MAX , "%s/%s", widgetpath , ft);
			
			struct stat     statbuf;
			if(stat(fn, &statbuf) == -1)
			{
				liqapp_log("liqcell_scan_folder_for_images stat failed: '%s'",fn);
				return -1;
			}
			// got the information we need
			if ( S_ISDIR(statbuf.st_mode) )
			{
				//printf(" DIR  ");
				liqcell_scan_folder_for_images(self,fn);	
			}
			else
			// got the information we need
			if ( S_ISREG(statbuf.st_mode) )
			{
				const char *ext=liqapp_filename_walktoextension(ft);
				if(!ext || !*ext)
				{
					// nothing to see here..
				}

				
				else
				if(
					strcasecmp(ext,"png")==0  ||
					strcasecmp(ext,"jpg")==0  ||
					strcasecmp(ext,"jpeg")==0
				  )
				
				{
					
						struct tm     *pictm;
						pictm = localtime(&statbuf.st_mtime);
						char   picdate[64];
						
						strftime(picdate,sizeof(picdate), "%Y%m%d_%H%M%S",pictm);
						
						
						char pickey[FILENAME_MAX];
						
						snprintf(pickey,sizeof(pickey),"%s_%s",picdate,ft);
						
						

						//    struct tm     *tm;
						//           tm = localtime(&statbuf.st_mtime);
						//           strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
								   
	


						liqcell *c = liqcell_quickcreatevis(pickey,   "picture",   1,1,1,1    );
						liqcell_propseti(c,"lockaspect",1);
						liqcell_propsets(c,"imagefilename",fn);
						//liqcell_handleradd(c,    "mouse",   widget_mouse);
						liqcell_handleradd(c,    "shown",         (void *)liqrecentphotoselect_item_shown);
						liqcell_handleradd(c,    "click",         (void *)liqrecentphotoselect_item_click);
						liqcell_handleradd(c,    "imageloaded",   (void *)liqrecentphotoselect_item_imageloaded);
						liqcell_handleradd_withcontext(c, "dialog_open", (void *)liqrecentphotoselect_item_dialog_open ,self);
						liqcell_handleradd_withcontext(c, "dialog_close", (void *)liqrecentphotoselect_item_dialog_close ,self);
                        
                        
                        //if(liqcell_child_countvisible(body)==0) liqcell_setselected(c,1);


						liqcell_child_insertsortedbyname( body, c, 0 );




				}
			}
		}
		closedir(dir_p);
		
		return 0;
}





























//#####################################################################
//#####################################################################
//#####################################################################
//#####################################################################
//#####################################################################



	static int liqrecentphotoselect_filter(liqcell *self, liqcellfiltereventargs *args, void *context)
	{
		// using the filter provided (which might be blank)
		
					int islike = 1;//liqcell_propgeti(  self, "filterlike", 1 );
		
		char *searchterm = NULL;
		
		if(args) args->resultoutof=0;
		if(args) args->resultshown=0;
		if(args) searchterm = args->searchterm;
		
		// examine each tag and if matches the search show it, otherwise dont..
		liqcell *body = liqcell_child_lookup(self,"body");
		
		liqcell *c = liqcell_getlinkchild_visual(body);
		while(c)
		{
			//if(liqcell_isclass(c,"picture"))
			{
				if(searchterm && *searchterm)
				{

					//struct pagefilename pfn;

					//pagefilename_breakapart(&pfn,c->name);
					
					
					char *key = c->name;
                    char *ifn = liqcell_propgets(c,"imagefilename",NULL);
                    if(ifn && *ifn)key=ifn;


					
					int isok = (key!=NULL) && (*key |= 0);
					if(isok)
					{
						if(islike)
						{
							// anywhere in string
							isok = ( stristr(key,searchterm) != NULL );
						}
						else
						{
							// only from the start
							isok = ( c->name == stristr(key,searchterm) );
						}
					}
					
					if(!isok)
					{
						// see if we can show it anyway
						if(liqcell_getselected(c)) isok=1;
					}

					if( isok ) // strstr(c->name,searchterm) )
					{
						// found a match!
						liqcell_setvisible(c,1);
						if(args) args->resultshown++;
					}
					else
					{
						// no match :(
						liqcell_setvisible(c,0);
					}
					if(args) args->resultoutof++;
				}
				else
				{
					// nothing to search for, show it
					liqcell_setvisible(c,1);
					
					if(args) args->resultoutof++;
					if(args) args->resultshown++;
				}
			}
			c=liqcell_getlinknext_visual(c);
		}
		liqcell_setrect(body,   0,0,self->w,self->h);
		liqcell_child_arrange_makegrid(body,3,3);
		//liqcell_propseti(self,"arrangecomplete",0);
		//liqcell_propseti(body,"easytileflyisfinished",0);

		return 1;
		
	}















/**	
 * liqrecentphotoselect layout - make any adjustments to fill the content as are required
 */	
static int liqrecentphotoselect_layout(liqcell *self,liqcelleventargs *args, liqcell *context)
{	
	liqcell *body= liqcell_child_lookup(self, "body");
	
		// make a normal grid
		liqcell_setrect( body, 0, 0, liqcell_getw(self),liqcell_geth(self) );
		liqcell_child_arrange_makegrid(body,3,3);		
 
	return 0;
}

liqcell *liqrecentphotoselect_create()
{
	liqcell *self = liqcell_quickcreatewidget("liqrecentphotoselect","form", 800,480);

	if(self)
	{
	//	liqcell_propseti(self,"idle_lazyrun_wanted",1);			 // :)
	//	liqcell_propseti(self,"multitouch_test_range",5);
	
	liqcell_handleradd_withcontext(self, "layout", (void *)liqrecentphotoselect_layout ,self);

/*
		//############################# title:titlebar
		liqcell *title = liqcell_quickcreatevis("title", "titlebar", 0,0, 800, 50);
		liqcell_setfont(	title, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
		liqcell_setcaption(title, "Recent photos and pictures" );
		liqcell_propsets(  title, "imagefilename", "/usr/share/liqbase/media/titlebanner_left.png" );
		liqcell_propsets(  title, "textcolor", "rgb(255,255,0)" );
		//liqcell_propsets(  title, "backcolor", "rgb(0,0,60)" );
		liqcell_propseti(  title, "lockaspect", 0 );
		liqcell_propseti(  title, "textalign", 0 );
		liqcell_child_append(  self, title);


			//############################# cmddraw:label
			liqcell *cmddraw = liqcell_quickcreatevis("cmddraw", "label", 580, 0, 86, 48);
			liqcell_setfont(	cmddraw, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
			liqcell_setcaption(cmddraw, "draw" );
			liqcell_propsets(  cmddraw, "textcolor", "rgb(255,255,255)" );
			liqcell_propsets(  cmddraw, "backcolor", "rgb(0,64,64)" );
			liqcell_propsets(  cmddraw, "bordercolor", "rgb(200,100,100)" );
			liqcell_propseti(  cmddraw, "textalign", 2 );
			//liqcell_handleradd_withcontext(cmddraw, "click", cmddraw_click, self );
			liqcell_child_append(  title, cmddraw);
			//############################# cmdzoom:label
			liqcell *cmdzoom = liqcell_quickcreatevis("cmdzoom", "label", 492, 0, 86, 48);
			liqcell_setfont(	cmdzoom, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
			liqcell_setcaption(cmdzoom, "zoom" );
			liqcell_propsets(  cmdzoom, "textcolor", "rgb(255,255,255)" );
			liqcell_propsets(  cmdzoom, "backcolor", "rgb(0,64,64)" );
			liqcell_propsets(  cmdzoom, "bordercolor", "rgb(200,100,100)" );
			liqcell_propseti(  cmdzoom, "textalign", 2 );
			//liqcell_handleradd_withcontext(cmdzoom, "click", cmdzoom_click, self );
			liqcell_child_append(  title, cmdzoom);
			//############################# cmdsel:label
			liqcell *cmdsel = liqcell_quickcreatevis("cmdsel", "label", 404, 0, 86, 48);
			liqcell_setfont(	cmdsel, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (29), 0) );
			liqcell_setcaption(cmdsel, "sel" );
			liqcell_propsets(  cmdsel, "textcolor", "rgb(255,255,255)" );
			liqcell_propsets(  cmdsel, "backcolor", "rgb(0,64,64)" );
			liqcell_propsets(  cmdsel, "bordercolor", "rgb(200,100,100)" );
			liqcell_propseti(  cmdsel, "textalign", 2 );
			//liqcell_handleradd_withcontext(cmdsel, "click", cmdsel_click, self );
			liqcell_child_append(  title, cmdsel);

 */



		liqcell *body = liqcell_quickcreatevis("body","frame",0 ,0,   self->w,self->h);

			// create a headskip blank
//			liqcell *headskip = liqcell_quickcreatevis("__headskip", NULL, 0, 0, liqcell_getw(title),liqcell_geth(title));
//			liqcell_child_append(  body, headskip);
			
			
		liqcell_child_insert( self, body );

	

			char buf[FILENAME_MAX];		
										snprintf(buf,sizeof(buf),"%s/MyDocs/.images",app.homepath);
			liqcell_scan_folder_for_images(self,buf);
	
										snprintf(buf,sizeof(buf),"%s/MyDocs/.camera",app.homepath);
			liqcell_scan_folder_for_images(self,buf);


										snprintf(buf,sizeof(buf),"%s/MyDocs/DCIM",app.homepath);
			liqcell_scan_folder_for_images(self,buf);

	
			//							snprintf(buf,sizeof(buf),"/home/user/MyDocs/.camera");
			//liqcell_scan_folder_for_images(self,buf);
	
			//							snprintf(buf,sizeof(buf),"/home/user/MyDocs/.images");
			//liqcell_scan_folder_for_images(self,buf);
		
		
		
		liqcell_handlerrun(self,"layout",NULL);
		
		
		
		liqcell_propsets(  self, "monitorpath" , buf);
		//liqcell_propsets(  self, "watchpattern" , "liq.*");

		liqcell *c=NULL;

		//liqcell_child_arrange_makegrid(body,3,3);
		

		c=liqcell_getlinkchild_visual(body);
        if(c)liqcell_setselected(c,1);

		liqcell_handleradd(body,    "mouse",   (void *)liqcell_easyhandler_kinetic_mouse );
		
		
		//liqcell_handleradd(self,    "click",   float_click);
/*
//#ifdef USE_INOTIFY
		//############################# timer1:liqtimer
		liqcell *timer1=liqcell_quickcreatevis("timer1",   "liqtimer",   0,0,   0,0 );
		// store ourselves on the tag for use later
		// this does feel like a workaround, but hell, it works!
		//liqcell_settag(timer1,liqcell_hold(self));
		liqcell_propseti(timer1,"timerinterval", 1 );
		liqcell_handleradd_withcontext(timer1,"timertick",timer_tick,self);
		liqcell_setenabled(timer1,1);
		liqcell_child_insert( self,timer1);
//#endif		
 */

        liqcell_handleradd(self,    "filter",   (void *)liqrecentphotoselect_filter);

	}
	
	return self;
}

#ifdef __cplusplus
}
#endif

