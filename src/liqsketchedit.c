#include <stdlib.h>
#include <string.h>
#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks

#include "liqapp_prefs.h"
#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqcell_easypaint.h"


// /usr/share/icons/hicolor/40x40/hildon/qgn_stat_battery_full100.png
// /usr/share/icons/hicolor/40x40/hildon/qgn_stat_battery_full75.png
// /usr/share/icons/hicolor/40x40/hildon/qgn_stat_battery_full50.png
// /usr/share/icons/hicolor/40x40/hildon/qgn_stat_battery_full25.png

// /usr/share/icons/hicolor/40x40/hildon/qgn_stat_displaybright1.png
//#####################################################################
//#####################################################################
//##################################################################### liqsketchedit :: by gary birkett :)
//#####################################################################
//#####################################################################


#include <curl/curl.h>


#ifdef __cplusplus
extern "C" {
#endif



struct curl_memorybuffer
{
	char *memory;
	size_t size;
};



static size_t curl_memorybuffer(void *ptr, size_t size, size_t nmemb, void *data)
{
	struct curl_memorybuffer *mem = (struct curl_memorybuffer *) data;
	int realsize = size * nmemb;

	mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory)
	{
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}
	return realsize;
}

















int post_to_liqbase_net(char *filename,char *datakey,int replyid)
{
	if(!datakey)datakey="liqbase";
 
 
    int newid=0;

	char *username = app.username;
	char *userpassmd5 = liqapp_pref_getvalue("userpassmd5");
	if(!userpassmd5 || !*userpassmd5 || !username || !*username)
	{
		liqapp_log("post_to_liqbase_net not performed, no username or pass configured");
		return -1;
	}
    
    liqapp_log("post_to_liqbase_net username: '%s'",username);
    liqapp_log("post_to_liqbase_net upload '%s' starting",filename);
    
    
    
    struct curl_memorybuffer resultchunk = {NULL,0};

	CURL* easyhandle = curl_easy_init();
    
    
    char replyidstr[32];
    snprintf(replyidstr,sizeof(replyidstr),"%i",replyid);
	
	curl_easy_setopt(easyhandle, CURLOPT_URL, "http://liqbase.net/liqbase_mediapush.php");
	curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, curl_memorybuffer);
	curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, (void *)&resultchunk);
    

	
	struct curl_httppost *post=NULL;  
	struct curl_httppost *last=NULL;  
	curl_formadd(&post, &last,   CURLFORM_COPYNAME, "username",         CURLFORM_COPYCONTENTS, username,       CURLFORM_END);
	curl_formadd(&post, &last,   CURLFORM_COPYNAME, "userpass",         CURLFORM_COPYCONTENTS, userpassmd5,        CURLFORM_END);

	curl_formadd(&post, &last,   CURLFORM_COPYNAME, "datakey",          CURLFORM_COPYCONTENTS, datakey,    CURLFORM_END);
	curl_formadd(&post, &last,   CURLFORM_COPYNAME, "datafile",         CURLFORM_FILE,         filename,            CURLFORM_END);
	curl_formadd(&post, &last,   CURLFORM_COPYNAME, "userto",           CURLFORM_COPYCONTENTS, username,              CURLFORM_END);
    
    
	curl_formadd(&post, &last,   CURLFORM_COPYNAME, "replyid",           CURLFORM_COPYCONTENTS, replyidstr,              CURLFORM_END);
 
 	curl_easy_setopt(easyhandle, CURLOPT_HTTPPOST, post);

	int uperr=curl_easy_perform(easyhandle);


	curl_formfree(post);
    
    liqapp_log("post_to_liqbase_net upload finished, got response '%i'",uperr);

    if(resultchunk.memory)
    {
        liqapp_log("post_to_liqbase_net upload got result! [[[[%s]]]]",resultchunk.memory);
        
        if( sscanf(resultchunk.memory,"newid=%i",&newid)==1)
        {
            // got id!
            liqapp_log("post_to_liqbase_net upload got id %i!",newid);
        }
        else
        {
            // failed
            liqapp_log("post_to_liqbase_net upload no id :(");
        }
        
        // parse the result.  numeric code and returnid expected
        // the numeric code should indicate ok!
        // the returnid is contained within the 2nd line
        free(resultchunk.memory);
    }
    else
    {
        liqapp_log("post_to_liqbase_net upload no result");
    }
    
    liqapp_log("post_to_liqbase_net upload finished");
    return newid;
	
}

 





	static int liqsketchedit_save(liqcell *self)
	{
	
		liqapp_log("moo");
		
		
		char *fn=liqcell_propgets(self,"sketcheditfilename",NULL);
		if(fn)
		{



			// 20090422_000423 lcuk : make sure we do not overwrite the file
			if(liqapp_fileexists(fn))
			{
				char filedate[256];	
				liqapp_formatnow(filedate,sizeof(filedate),"yyyymmdd_hhmmss");
				char *filetitle =liqapp_filename_walkoverpath(fn);
				// got a file
				char s[FILENAME_MAX*3];
				snprintf(s,sizeof(s),"mv %s %s.old.%s",fn,fn,filedate);
				system(s);
				liqapp_log("sketch aging cmd: %s",s);
			}


			// 20090421_233231 lcuk : save it now with the special assigned name

			
			liqsketch_filesave(liqcell_getsketch(self), fn  );
			liqcell_settag(self,0);
			return 1;
		}
		
		liqcell *notes = liqcell_child_lookup(self,"notes");
		char *key;
		
		
		if(notes)
		{
			key = liqcell_getcaption(notes);
		}
		else
		{
			key="";
		}
		

		char filedate[256];

		liqapp_formatnow(filedate,sizeof(filedate),"yyyymmdd_hhmmss");

		char filenamebuffer[FILENAME_MAX];
		if(key && *key)
		{
			snprintf(filenamebuffer,sizeof(filenamebuffer), "%s/sketches/liq.%s.%s.page.%s",    app.userdatapath,    filedate,    app.username,key );
			
		}
		else
		{	
			snprintf(filenamebuffer,sizeof(filenamebuffer), "%s/sketches/liq.%s.%s.page",    app.userdatapath,    filedate,    app.username );
		}

		liqsketch_filesave(liqcell_getsketch(self), filenamebuffer );


		liqcell_propsets(self,"sketchfilenamelast",filenamebuffer);
		
		post_to_liqbase_net(filenamebuffer,key,0);
		
		liqcell_settag(self,0);
		
	

		return 1;
	}


	static int liqsketchedit_shown(liqcell *self, liqcelleventargs *args, void *context)
	{
        // shown for the first time :)
        liqcell *cover = liqcell_child_lookup(self,"cover");
        if(liqcell_getlinkparent( self) == NULL )
        {
            // no parent..
            // make sure we show the cover
            liqcell_setvisible(cover,1);
        }
        else
        {
            // got a parent..
            // make sure we hide the cover
            liqcell_setvisible(cover,0);
        }
    }

	static int liqsketchedit_dialog_open(liqcell *self, liqcelleventargs *args, void *context)
	{
        
        liqcell *cover = liqcell_child_lookup(self,"cover");
        if(liqcell_getlinkparent( self) == NULL )
        {
            // no parent..
            // make sure we hide the cover
            liqcell_setvisible(cover,0);
        }
        else
        {
            // got a parent..
            // make sure we hide the cover
            liqcell_setvisible(cover,0);
        }
    }




	static int liqsketchedit_dialog_close(liqcell *self, liqcelleventargs *args, void *context)
	{
        
        liqcell *cover = liqcell_child_lookup(self,"cover");
        if(liqcell_getlinkparent( self) == NULL )
        {
            // no parent..
            // make sure we show the cover
            liqcell_setvisible(cover,1);
        }
        else
        {
            // got a parent..
            // make sure we hide the cover
            liqcell_setvisible(cover,0);
        }        
		// save the sketch?

		//liqcell *content = liqcell_child_lookup( self,"content");
		liqsketch *sketch = liqcell_getsketch(self);
		if(!sketch || sketch->strokecount==0)
		{
			return 0;
		}
		if(!liqcell_gettag(self)) return 0;

		liqsketchedit_save(self);
		
		//liqcell_setsketch(self,NULL);
		

		return 1;
	}



	static int liqsketchedit_undo_click(liqcell *self, liqcellclickeventargs *args, void *context)
	{
		liqcell *editor = liqcell_getlinkparent(self);

		liqsketch *sketch = liqcell_getsketch(editor);
		if(!sketch)
		{
			return 0;
		}
		
		//if(!liqcell_gettag(editor)) return 0;
		
		if(!sketch->strokelast) return 0;
		
		liqsketch_strokeremove(sketch,sketch->strokelast);

		liqcell_settag(editor, (void*)1);
		
		liqcell_handlerrun(editor,"undo",NULL);

		return 1;
	}



	static int liqsketchedit_clear_click(liqcell *self, liqcellclickeventargs *args, void *context)
	{
		liqcell *editor = liqcell_getlinkparent(self);

		liqsketch *sketch = liqcell_getsketch(editor);
		if(!sketch)
		{
			return 0;
		}

		liqsketch_clear(sketch);

        
		char *fn=liqcell_propgets(editor,"sketcheditfilename",NULL);        // fixed name bug would not reload, thanks javispedro
		if(fn)
		{
			// 20090421_233231 lcuk : save it now with the special assigned name
			liqsketch_fileload(sketch, fn );

			return 1;
		}
		
		liqcell_handlerrun(editor,"cleared",NULL);
		
		liqcell_settag(editor,0);

		return 1;
	}

	static int liqsketchedit_del_click(liqcell *self, liqcellclickeventargs *args, void *context)
	{
		
		liqcell *editor = liqcell_getlinkparent(self);

		liqsketch *sketch = liqcell_getsketch(editor);
		if(!sketch)
		{
			
			return 0;
		}
		
		

		liqsketch_clear(sketch);

		char *fn=liqcell_propgets(editor,"sketcheditfilename",NULL);      // fixed name bug would not delete, thanks javispedro
		if(fn)
		{
			// 20090422_000423 lcuk : delete the actual file now
			if(liqapp_fileexists(fn))
			{
				char filedate[256];	
				liqapp_formatnow(filedate,sizeof(filedate),"yyyymmdd_hhmmss");
				char *filetitle =liqapp_filename_walkoverpath(fn);
				// got a file
				char s[FILENAME_MAX*3];
				snprintf(s,sizeof(s),"mv %s %s.del.%s",fn,fn,filedate);
				system(s);
				liqapp_log("sketch delete cmd: %s",s);
			}
		}
		
		liqcell_setsketch(editor,NULL);
		
		liqcell_settag(editor, (void*)1);


		liqcell_handlerrun(editor,"cleared",NULL);
		
		return 1;
	}

	static int liqsketchedit_save_click(liqcell *self, liqcellclickeventargs *args, void *context)
	{
		liqcell *editor = liqcell_getlinkparent(self);

		liqsketch *sketch = liqcell_getsketch(editor);
		if(!sketch)
		{
			liqapp_log("liqsketchedit_save_click nothing to save");
			return 0;
		}

		liqsketchedit_save(editor);

		liqsketch_clear(sketch);
		
		liqcell_handlerrun(editor,"cleared",NULL);
		
		liqcell_settag(editor,0);


		return 1;
	}

	static int liqsketchedit__cmdnull_mouse(liqcell *self, liqcellmouseeventargs *args, void *context)
	{
		return 1;
	}
		
	static int liqsketchedit_mouse(liqcell *self, liqcellmouseeventargs *args, void *context)
	{
		liqsketch *sketch = liqcell_getsketch(self);
		if(!sketch)
		{
			// 20090421_215728 lcuk : make the sketch now - very late bound :)
			//return 0;
			// mmm dont know about using this
			liqsketch *s = liqsketch_new();
			
			
					//s->pixelwidth =liqcell_getw(self);
					//s->pixelheight=liqcell_geth(self);
					
					s->pixelwidth =liqcanvas_getwidth();
					s->pixelheight=liqcanvas_getheight();
					
					s->dpix=225;	// damn, dont like using this here
					s->dpiy=225;
			liqcell_setsketch( self, s );
			sketch=s;
		}
		liqstroke *stroke;
		// this was only to see high intensity quickly
		//args->mez/=2;
		
		//liqapp_log("sss me %i,%i   o %i,%i   ss %i,%i",args->mex,args->mey,    args->ox,args->oy,   args->mex-args->ox,args->mey-args->oy);

		if(args->mcnt==1)
		{
			// starting, allocate and insert a new stroke
            
            
			stroke = liqstroke_new();
			stroke->pen_y = 255;
			stroke->pen_u = 128;
			stroke->pen_v = 128;
            
            {
                char *t=NULL;
                unsigned char bcy=255;
                unsigned char bcu=128;
                unsigned char bcv=128;
                unsigned char bca=255;
                unsigned char bcc=0;
                    t = liqcell_propgets(self,"pencolor",NULL);
                    if(t)
                    {
                        //liqapp_log("pencolor :: '%s'",t);
                        if(decodecolor(t, &bcy, &bcu, &bcv, &bca, &bcc ))
                        {
                            stroke->pen_y = bcy;
                            stroke->pen_u = bcu;
                            stroke->pen_v = bcv;
                        }
                    }
            }
            
			// ewww yellow experiment looks bad in non aa
			//stroke->pen_y = 255;
			//stroke->pen_u = 255;
			//stroke->pen_v = 1;

			liqstroke_start(stroke,args->mex-args->ox,args->mey-args->oy,args->mez,args->met);

			liqsketch_strokeinsert(sketch,stroke);
		}
		else
		{
			// continuing, get hold of the last stroke
			stroke = sketch->strokelast;
			liqstroke_extend(stroke,args->mex-args->ox,args->mey-args->oy,args->mez,args->met);
			liqsketch_strokeupdate(sketch,stroke);

			if(args->mez!=0)
			{
				// carry on
			}
			else
			{
				// finishing	
			}
		}
		// 20090421_232509 lcuk : make sure we mark ourselves as dirty
		liqcell_settag(self, (void*)1);

		return 1;
	}





static int liqsketchedit_resize(liqcell *self, liqcelleventargs *args, void *context)
{
	//liqcell *base = liqcell_getbasewidget(self);
	liqcell *cover = liqcell_child_lookup(self,"cover");
	liqcell *undo = liqcell_child_lookup(self,"undo");
	liqcell *clear = liqcell_child_lookup(self,"clear");
	liqcell *save = liqcell_child_lookup(self,"save");
	liqcell *del = liqcell_child_lookup(self,"del");
	liqcell *notes = liqcell_child_lookup(self,"notes");
	
	int ww=liqcell_getw(self);
	int hh=liqcell_geth(self);
	
	liqcell_setrect(cover,  0,  0,            ww*1.0,hh*1.0);
	liqcell_setrect(undo,  ww*0.9,  0,        ww*0.1,hh*0.3);
	liqcell_setrect(clear, ww*0.9,  hh*0.3,   ww*0.1,hh*0.3);
	liqcell_setrect(save , ww*0.9,  hh*0.6,   ww*0.1,hh*0.3);
	liqcell_setrect(del ,  ww*0.9,  hh*0.9,   ww*0.1,hh*0.1);
	liqcell_setrect(notes ,  ww*0.25,  hh*1,   ww*0.5,hh*0.1);
	
	
	
}

	static int liqsketchedit_paint(liqcell *self, liqcellpainteventargs *args,liqcell *liqsketchedit)
	{
		liqcell *notes = liqcell_child_lookup(liqsketchedit,"notes");
		char *cap=liqcell_getcaption(notes);
		if(cap && *cap)
		{
			if( liqcell_gety(notes) > ( liqcell_geth(liqsketchedit) - liqcell_geth(notes) )  )
			{
				// move it a bit more onscreen
				int dif = liqcell_gety(notes) - ( liqcell_geth(liqsketchedit) - liqcell_geth(notes) );
				if(dif>5)dif=5;
				liqcell_setpos( notes, liqcell_getx(notes), liqcell_gety(notes) - dif );
				liqcell_setdirty(liqsketchedit,1);
			}
		}
		else
		{
			if( liqcell_gety(notes) != liqcell_geth(liqsketchedit) )
			{
				int dif = liqcell_geth(liqsketchedit) - liqcell_gety(notes);
				if(dif>5)dif=50;				
				liqcell_setpos(notes, liqcell_getx(notes),liqcell_gety(notes) + dif);
				liqcell_setdirty(liqsketchedit,1);
			}
		}
	}


liqcell *liqsketchedit_create()
{
	liqcell *self = liqcell_quickcreatewidget("liqsketchedit","form", 800,480);

	if(self)
	{

		//liqcell *self = liqcell_quickcreatevis(name,  NULL,   l,t,w,h );

		liqcell_propseti(self,"sketchediting",1);		// mark it as editing :)

			// 20090421_215752 lcuk : dont make it now, leave it for ultra late binding
			//// give us something to draw onto
			//liqsketch *s = liqsketch_new();
			//		s->pixelwidth =liqcell_getw(self);
			//		s->pixelheight=liqcell_geth(self);
			//		s->dpix=225;	// damn, dont like using this here
			//		s->dpiy=225;
			//liqcell_setsketch( self, s );

		
		liqcell *b;

		b = liqcell_quickcreatevis("cover","picture",  0,0,   800,480 );
        liqcell_propsets(  b, "imagefilename", "/usr/share/liqbase/libliqbase/media/liqsketch_cover.png" );
        liqcell_setvisible(b,0);
		liqcell_child_insert( self, b );

		b = liqcell_quickcreatevis("undo","button",  800-180,20,   160,160 );
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd(b,    "click",   (void*)liqsketchedit_undo_click);
		liqcell_propsets(  b,    "backcolor", "xrgb(100,0,100)" );
		liqcell_handleradd(b,    "mouse",   (void*)liqsketchedit__cmdnull_mouse);
		liqcell_child_insert( self, b );


		b = liqcell_quickcreatevis("clear","button",  800-180,20,   160,160 );
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd(b,    "click",   (void*)liqsketchedit_clear_click);
		liqcell_propsets(  b,    "backcolor", "xrgb(0,0,100)" );
		liqcell_handleradd(b,    "mouse",   (void*)liqsketchedit__cmdnull_mouse);
		liqcell_child_insert( self, b );
	

		b = liqcell_quickcreatevis("save","button",  800-180,200,   160,160 );
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd(b,    "click",   (void*)liqsketchedit_save_click);
		liqcell_propsets(  b,    "backcolor", "xrgb(0,100,0)" );
		liqcell_handleradd(b,    "mouse",   (void*)liqsketchedit__cmdnull_mouse);
		liqcell_child_insert( self, b );


		b = liqcell_quickcreatevis("del","button",  800-180,200,   160,160 );
		liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		liqcell_handleradd(b,    "click",   (void*)liqsketchedit_del_click);
		liqcell_propsets(  b,    "backcolor", "xrgb(100,0,0)" );
		liqcell_handleradd(b,    "mouse",   (void*)liqsketchedit__cmdnull_mouse);
		liqcell_child_insert( self, b );


		b = liqcell_quickcreatevis("notes","textbox",  200,480,   480,80 );
		liqcell_setcaption(b,"");
		//liqcell_setfont(   b, liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", (24), 0) );
		//liqcell_handleradd(b,    "click",   liqsketchedit_del_click);
		//liqcell_propsets(  b,    "backcolor", "rgb(100,0,0)" );
		//liqcell_handleradd(b,    "mouse",   liqsketchedit__cmdnull_mouse);
		liqcell_child_insert( self, b );
		//liqcell_setvisible(b,0);





		liqsketchedit_resize(self,NULL, NULL);

        liqcell_handleradd_withcontext(self,    "shown",   (void*)liqsketchedit_shown,   self);
		liqcell_handleradd_withcontext(self,    "dialog_open",   (void*)liqsketchedit_dialog_open,   self);
		liqcell_handleradd_withcontext(self,    "dialog_close",   (void*)liqsketchedit_dialog_close,   self);
		liqcell_handleradd_withcontext(self,    "mouse",   (void*)liqsketchedit_mouse,   self);
		liqcell_handleradd_withcontext(self,    "resize",   (void*)liqsketchedit_resize,   self);
		liqcell_handleradd_withcontext(self,    "paint",   (void*)liqsketchedit_paint,   self);
		
		
	}
	return self;

}

#ifdef __cplusplus
}
#endif

