

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <X11/keysym.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>
#include <sched.h>



// 20090517_112443 lcuk : define this to enable heavyweight debugging of the render, it shows LOTS of detail and slows things down
//#define __tz_easypaint_debug 1




#include "liqbase.h"
#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"

//liqfont *easyinfofont;

// for the vgraph
// make a zoom tree
// starts with a root cell
// which is the size of the display
// i then zoom in
// that zoom has a related internal dimension
// ie a reference to the current cell

int liqcell_showdebugboxes=0;



liqimage *easypaint_isloading_image = NULL;
liqimage *easypaint_barcode_image = NULL;

//qr_barcode.png





int thread_createwithpriority(pthread_t *tid,int threadpriority,void *(*func)(void *),void *arg)
{
//pthread_t 		tid;
pthread_attr_t 	tattr;
struct sched_param 	param;
int ret;
int newprio = threadpriority;//20;


	// initialized with default attributes
	ret = pthread_attr_init(&tattr);
	// safe to get existing scheduling param
	ret = pthread_attr_getschedparam(&tattr, &param);
	// set the priority; others are unchanged

	//liqapp_log("thread schedparam=%i (current)",param.sched_priority);

	param.sched_priority = newprio;
	// setting the new scheduling param
	ret = pthread_attr_setschedparam(&tattr, &param);
	// with new priority specified

	ret = pthread_create(tid, &tattr, func, arg);


//	ret = pthread_create(tid, NULL, func, arg);
	return ret;
}




static int mainthread_inprogress=0;




void *mainthread(void* mainthread_data)
{

	//liqapp_sleep(100 + (rand() % 4000));
	//liqapp_sleep(100 + (rand() % 2000));
	
	
do
{
	//liqapp_sleep(10 + (rand() % 100));
	liqapp_sleep(10 + (50));
}
while(mainthread_inprogress>1);

	mainthread_inprogress++;
	
	
	

	//
	liqcell *self = (liqcell *)mainthread_data;

	// we are here to load the image named on self and then finish


	liqimage *img = liqcell_getimage(self);

	if(img == easypaint_isloading_image)
	{


		char *fn = liqcell_propgets(self,"imagefilename",NULL);
		if(fn)
		{
			// now actually load the main data
			// this should raise the dirty flag and ensure it is rerendered
			//liqapp_log("picloader loading  %s",fn);

			char cachefn[2048];
			snprintf(cachefn,sizeof(cachefn), "%s", fn  );
			// ok, let me try one thing..

			if( strncmp(fn,"http://",7) == 0 )
			{
				// hmmmm, interesting, download the data..

				liqapp_log("http detected..");
				//########################## grab filename, and clean it quickly
				snprintf(cachefn,sizeof(cachefn), "%s", &fn[7] );
				char *t=cachefn;
				while(*t)
				{
					if(*t==':' || *t=='.' || *t==',' || *t=='=' || *t=='&' || *t=='/' || *t=='\\' || *t=='\'' || *t=='\"' || *t=='?') *t='_';
					t++;
				}

				//########################## now, add on the cache path


				char longbuf[2048];

				snprintf(longbuf,sizeof(longbuf), "liqbasecache/%s", cachefn );

				strncpy(cachefn,longbuf,1024);


				liqapp_log("http checking '%s'",cachefn);


				//########################## does it already exist?

				
				if(liqapp_filesize(longbuf)>0)
				{
					// the file has already been downloaded! sweet!
					liqapp_log("http got valid file already..");
					strncpy(cachefn,longbuf,1024);
				}
				else
				{
					// file might actually exist
					if( !liqapp_fileexists(longbuf) )
					{
						char killbuf[2048];
						snprintf(killbuf,sizeof(killbuf),"rm %s",longbuf);
						// hack to try to make sure theres a liqbasecache folder
						system(killbuf);
					}
					

/*
				if(liqapp_fileexists(longbuf))
				{
					// the file has already been downloaded! sweet!
					liqapp_log("http got file already..");
					strncpy(cachefn,longbuf,1024);
				}
				else
				{
					
					
*/					
					
					
					
					
					liqapp_log("http about to download '%s' into '%s'",fn,cachefn);
					// doesn't exist yet
					if(!liqapp_pathexists("liqbasecache"))
					{
						// hack to try to make sure theres a liqbasecache folder
						system("mkdir liqbasecache");
					}


					// do the downloading
					char cmdbuf[2048];
					snprintf(cmdbuf,sizeof(cmdbuf), "wget '%s' -N -q --output-document='%s'", fn , cachefn );
					
					//snprintf(cmdbuf,sizeof(cmdbuf), "wget '%s' -q --output-document='%s'", fn , cachefn );
					
					liqapp_log("http command: %s",cmdbuf);
					
					int sok=system(cmdbuf);


					liqapp_log("http download result: %i, exists? %i",sok,liqapp_fileexists(cachefn));
					


				}

				fn=cachefn;
			}


			if( strncmp(fn,"file://",7) == 0 )
			{
				// hmmmm, interesting, remove this portion..
				fn=&fn[7];
			}

			int imageallowalpha = liqcell_propgeti(self,"imageallowalpha",1);


			//liqapp_log("picloader loading  '%s' imageallowalpha=%i",fn,imageallowalpha);


			liqimage *imgnew = liqimage_cache_getfile(fn,0,0, imageallowalpha );
			if(imgnew)
			{
				liqcell_setimage( self, imgnew );

				int isautosize = liqcell_propgeti(self,"autosize",0);
				if(isautosize)
				{
					liqcell_setsize(self,imgnew->width,imgnew->height);
				}



				liqcell_setdirty(self,1);
				liqcell_release(self);
			}
			else
			{
				// clear the utility of the "loading" image
				liqcell_setimage( self, NULL );
				liqcell_release(self);

			}


		}

	}
	
	mainthread_inprogress--;

	pthread_exit(0);
	return NULL;
}



int liqcell_threadloadimage(liqcell *self)
{
				if(!easypaint_isloading_image)
				{
					//easypaint_isloading_image=liqimage_newfromfile("/usr/share/liqbase/libliqbase/media/sun.png",0,0,0);
					easypaint_isloading_image=liqimage_newfromfile("/usr/share/liqbase/libliqbase/media/pleasewait.png",0,0,0);
				}
				if(easypaint_isloading_image)
				{
					liqcell_hold(self);
					// only if we have a valid replacer can we do this trickery :)
					liqcell_setimage(self, liqimage_hold(easypaint_isloading_image) );
					// now we must start the thread off
					pthread_t 		tid;
					
					//int tres=thread_createwithpriority(&tid,0,mainthread,self);
					
					int tres=pthread_create(&tid,NULL,mainthread,self);
					
					
					if(tres)
					{
						liqapp_log("liqcell_easypaint: thread create fail %s :: %i",liqcell_getcaption(self),tres);
						//liqcliprect_release(cr);
						liqcell_release(self);
						return;
					}
				}

}

















static float calcaspect(int captionw,int captionh,int availw,int availh)
{
	if(captionw==0)return 0;
	if(captionh==0)return 0;
	float ax = (float)availw / (float)captionw;
	float ay = (float)availh / (float)captionh;
	float ar = (ax<=ay ? ax : ay);
	return ar;

}


unsigned int decodecolor(char *source,unsigned char *ry,unsigned char *ru,unsigned char *rv)
{
	char inbuf[1024];
	snprintf(inbuf,1024,source);
	char *indat=inbuf;
	char *s1=NULL;
	char *s2=NULL;
	char *s3=NULL;
	//char *s4=NULL;  // used for alpha (not in place..)
	if(strncmp(indat,"rgb(",4) == 0 )
	{
		indat+=4;
		s1=indat;
		while(*indat>='0' && *indat<='9')indat++;
		if(*indat++!=',') return 0;
		s2=indat;
		while(*indat>='0' && *indat<='9')indat++;
		if(*indat++!=',') return 0;
		s3=indat;
		while(*indat>='0' && *indat<='9')indat++;
		if(*indat++!=')') return 0;
		if(s1==s2 || s2==s3) return 0;
		s2[-1]=0;
		s3[-1]=0;
		indat[-1]=0;
		//liqapp_log("%s %i,%i,%i",source,atoi(s1),atoi(s2),atoi(s3) );
		int R=atoi(s1);
		int G=atoi(s2);
		int B=atoi(s3);
		//ry=atoi(s1);
		//ru=atoi(s2);
		//rv=atoi(s3);
			// convert RGB -> YUV
			//http://msdn.microsoft.com/en-us/library/ms893078.aspx
			// yes, microsoft are useful :)
		*ry = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
		*ru = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
		*rv = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
		return 1;
	}
	if(strncmp(indat,"yuv(",4) == 0 )
	{
		indat+=4;
		s1=indat;
		while(*indat>='0' && *indat<='9')indat++;
		if(*indat++!=',') return 0;
		s2=indat;
		while(*indat>='0' && *indat<='9')indat++;
		if(*indat++!=',') return 0;
		s3=indat;
		while(*indat>='0' && *indat<='9')indat++;
		if(*indat++!=')') return 0;
		if(s1==s2 || s2==s3) return 0;
		s2[-1]=0;
		s3[-1]=0;
		indat[-1]=0;
		//liqapp_log("%s %i,%i,%i",source,atoi(s1),atoi(s2),atoi(s3) );
		*ry=atoi(s1);
		*ru=atoi(s2);
		*rv=atoi(s3);
		return 1;
	}
	return 0;
}







#define SGN(x) ( ((x)<0) ? -1 : 1 )


	//self->kineticx=kx;
	//self->kineticy=ky;


static int liqcell_kineticboiloff(liqcell *self)
{
	if(self->kineticx || self->kineticy)
	{

		liqcell * par = liqcell_getlinkparent(self);


		int dx=0;
		int dy=0;


		if(self->kineticx) dx=SGN(self->kineticx);
		if(self->kineticy) dy=SGN(self->kineticy);


		//int px = self->x + dx;
		//int py = self->y + dy;

		int px = self->x + self->kineticx;
		int py = self->y + self->kineticy;




		if(px>0){px=0;  dx=self->kineticx; } // make sure kinetics are stopped
		if(py>0){py=0;  dy=self->kineticy; } // make sure kinetics are stopped

		if( (self->w > par->w) && ((px+self->w) < par->w) ){ px = par->w - self->w;  dx=self->kineticx; } // make sure kinetics are stopped
		if( (self->h > par->h) && ((py+self->h) < par->h) ){ py = par->h - self->h;  dy=self->kineticy; } // make sure kinetics are stopped

		//liqapp_log("kinetic pre motion... k=%2i,%2i  d=%i,%i    p=%i,%i s=%i,%i", self->kineticx , self->kineticy,   dx,dy,    px,py , self->x,self->y );


		liqcell_setpos(self,px,py);

		liqcell_setkinetic(self, self->kineticx - dx, self->kineticy - dy );

		liqcell_setdirty(self,1);

		//liqapp_log("kinetic aft motion... k=%2i,%2i  d=%i,%i    p=%i,%i s=%i,%i", self->kineticx , self->kineticy,   dx,dy,    px,py , self->x,self->y );

		return 1;
	}
	return 0;
//
}




void liqcell_easypaint(liqcell *self,liqcliprect *crorig,    int x,int y,    int w,int h)
{


	//liqapp_log("#################################### liqcell easypaint (%i,%i) :: %s   xy(%i,%i)-wh(%i,%i)  cr==can.cr ? %i",self->w,self->h,self->name,x,y,w,h,crorig==liqcanvas_getcliprect());
	
	

// 20090517_012357 lcuk : turn logging on or off
#ifdef __tz_easypaint_debug
#define tzmax 25
char 	tstr[tzmax][8]={{0}};
unsigned long 	tz[tzmax]={0};
long tzused=0;
#define __tz_one(code) { if(tzused<tzmax){  snprintf(tstr[tzused],32,"%s",(code));      tz[tzused++] = liqapp_GetTicks(); } liqapp_log("%s $$ %s",self->name,code); }
#else
#define __tz_one(code) {  }
#endif



__tz_one("start");




//char cy,cu,cv;
	if(w<1 || h<1)
	{
		//liqapp_log("size0 bail!");
		return;
	}
	if(self->w==0 || self->h==0)
	{
		//liqapp_log("box0 bail!");
		return;
	}
	if(!self->visible)
	{
		//liqapp_log("vis bail!");
		return;
	}
	
	//liqapp_log("easypaint 0");
	
__tz_one("sizeok");

	liqcliprect *cr= liqcliprect_new();
	liqcliprect_copy(cr,crorig);
	liqcliprect_shrink(cr,x,y,x+w,y+h);

//unsigned long 	tz0=liqapp_GetTicks();

__tz_one("clipgot");
	//liqapp_log("easypaint 1");

	if(!liqcliprect_isvalid(cr))
	{
		//liqapp_log("cr bail!");
		liqcliprect_release(cr);
		return;
	}
__tz_one("clipok");

	if(self->classname)
	{
		if( strcmp(self->classname,"overlay") ==0 )
		{
			//
			if( ((w)>(cr->surface->width/2)) || ((h)>(cr->surface->height/2)) )
			{
				// suitably large, let it be shown
			}
			else
			{
				// not big enough
				//liqapp_log("edit bail!");
				liqcliprect_release(cr);
				return;
			}
		}
	}
__tz_one("overlayok");
	//########################################
	// fraction of default scale this
	// 28jan:gb:
	//float zoomw=((float)w)/((float)self->w);
	//float zoomh=((float)h)/((float)self->h);
	//float ar = (zoomw<zoomh) ? zoomw : zoomh;
	//float aw=((float)self->w) * ar;
	//float ah=((float)self->h) * ar;
	//w=aw;
	//h=ah;
	
	
	//liqapp_log("easypaint 2");



//	liqapp_log("paint %s:%s (%i,%i) :: (%i,%i)  k(%i,%i) ",self->name,self->classname,self->w,self->h, w,h, self->kineticx,self->kineticy);//,aw,ah);

	if(w<1 || h<1)
	{
		//liqapp_log("size bail!");
		liqcliprect_release(cr);
		return;
	}
	
__tz_one("sizeok2");	
	//liqapp_log("easypaint 3");
	
	
	if(liqcell_getshown(self)==0)
	{
		// 20090513_201740 lcuk : this control has never been seen on screen before, lets give it a chance :)
		liqcell_handlerrun(self,"shown",NULL);
		liqcell_setshown(self,1);
	}
	
__tz_one("showndone");

	// try it..
	
	
	if(liqcell_handlerfind(self,"paint"))
	{

		// 20090621_120251 lcuk : paint NEEDS to pass a valid args with valid graph object here, shit it doesnt

		vgraph *graph = vgraph_new();
		vgraph_setscaleaspectlock(graph,  1);
		
		vgraph_setcliprect(graph, cr           );
		//vgraph_settarget(graph,   cr->surface  );
		vgraph_settarget_coord( graph,  x,y,w,h      );
		vgraph_setwindow( graph,  (self)       );
		//vgraph_setwindow_coord( graph,  x,y,w,h      );

		liqcellpainteventargs paintargs;
		//paintargs.cr = liqcanvas_getcliprect();
		paintargs.graph = graph;
		paintargs.runfast=0;
		
		paintargs.ox=x;
		paintargs.oy=y;
		paintargs.mx=w;
		paintargs.my=h;
		
		liqcell_handlerrun(self,"paint",&paintargs);
		
		//vgraph_setwindow( graph,  NULL       );
		//vgraph_settarget(graph,   NULL  );
		//vgraph_setcliprect(graph,   NULL  );
		vgraph_release(graph);
	}
	
	// 20090526_230043 lcuk : todo: test whether bailing if painting "dealt with" harms anything
	// 20090526_230103 lcuk : i dont think it does right now
	
__tz_one("paintdone");



	char *t=NULL;
	unsigned char bcy=255;
	unsigned char bcu=128;
	unsigned char bcv=128;




	{
		t = liqcell_propgets(self,"backcolor",NULL);
		if(t)
		{
			//liqapp_log("textcolor :: '%s'",t);
			if(decodecolor(t, &bcy, &bcu, &bcv ))
			{
				liqcliprect_drawboxfillcolor(cr,x,y,w,h,bcy,bcu,bcv);
			}
		}
	}
__tz_one("backdone");
		//liqcliprect_drawimagecolor(cr,camimage,x,y,w,h,0);

	int ishttp(char *filename)
	{
		if(tolower(filename[0])=='h' && tolower(filename[1])=='t' && tolower(filename[2])=='t' && tolower(filename[3])=='p' && tolower(filename[4])==':')
			return 1;
		return 0;
	}


	if(!self->image)
	{
		char *fn = liqcell_propgets(self,"imagefilename",NULL);
		if(fn && (liqapp_fileexists(fn) || ishttp(fn)))
		{
		
			// 20090619_125841 lcuk : do a quick check in the image cache
			// 20090619_125850 lcuk : if the file exists, then we just grab that :)
			int imageallowalpha = liqcell_propgeti(self,"imageallowalpha",1);
			liqimage *imgnew = liqimage_cache_lookuponly(fn,0,0, imageallowalpha );
			if(imgnew)
			{
				liqcell_setimage( self, imgnew );
				int isautosize = liqcell_propgeti(self,"autosize",0);
				if(isautosize)
				{
					liqcell_setsize(self,imgnew->width,imgnew->height);
				}
				liqcell_setdirty(self,1);
			}			
			else
			{
				
				// i need to do the following:
				// set a "please wait, loading" icon
				// create a low priority thread with this object and nothing else
				
				if(!easypaint_isloading_image)
				{
					//easypaint_isloading_image=liqimage_newfromfile("/usr/share/liqbase/libliqbase/media/sun.png",0,0,0);
					easypaint_isloading_image=liqimage_newfromfile("/usr/share/liqbase/libliqbase/media/pleasewait.png",0,0,0);
				}
				if(easypaint_isloading_image)
				{
					liqcell_hold(self);
					// only if we have a valid replacer can we do this trickery :)
					liqcell_setimage(self, liqimage_hold(easypaint_isloading_image) );
					// now we must start the thread off
					pthread_t 		tid;
					
					//int tres=thread_createwithpriority(&tid,0,mainthread,self);
					
					int tres=pthread_create(&tid,NULL,mainthread,self);
					
					
					if(tres)
					{
						liqapp_log("liqcell_easypaint: thread create fail %s :: %i",fn,tres);
						liqcliprect_release(cr);
						liqcell_release(self);
						return;
					}
				}
			}
			
		
			
		//	int imageallowalpha = liqcell_propgeti(self,"imageallowalpha",1);
		//	liqimage *imgnew = liqimage_cache_getfile(fn,0,0, imageallowalpha );
		//	liqcell_setimage(self,imgnew);
			
		}
	}
	
	
	
	
	
	
__tz_one("imageprep");

	if(self->image)
	{
		// 20090812_213106 lcuk : see if we should be floating this image
		// 20090812_213118 lcuk : floating image is slightly larger than its default area and in constant slow motion
		
		char *fstr=liqcell_propgets(self,"imagefloat",NULL);
		if(fstr && *fstr)
		{
		//	liqapp_log("imagefloat: start '%s' %i,%i",self->name,w,h);
/*

			//liqapp_log("imagefloat: start");
			// when floating, the image is scaled 130%
			int cx=0;					// cell dimensions
			int cy=0;
			int cw=liqcell_getw(self);
			int ch=liqcell_geth(self);
			
			int tx=0;					// total dimensions
			int ty=0;
			int tw=cw*1.3;
			int th=ch*1.3;
			
			int frw=tw-cw;
			int frh=th-ch;
			
			float fax=0.03 * (float)(rand() % 100 - 50);	// acceleration is random
			float fay=0.03 * (float)(rand() % 100 - 50);			
			float fvx=0;									// velocity is some of the accel
			float fvy=0;	
			float fpx=0;									// position is the field
			float fpy=0;
			
			//liqapp_log("imagefloat: reading");
			int rs=sscanf(fstr,"%f,%f,%f,%f",&fvx,&fvy,&fpx,&fpy);
			
			//liqapp_log("imagefloat: read, got %i",rs);
			
			#define dt 0.2
			#define maxv 10
			
			if(rs!=4){  fvx=0;  fvy=0;  fpx=0;  fpy=0; }
			
			//if(rs==4)
			{
				
				
				fvx += dt * fax;
				fvy += dt * fay;
				
				if(fvx<-maxv){fvx=-maxv; } else{ if(fvx>maxv){fvx=maxv;}}
				if(fvy<-maxv){fvy=-maxv; } else{ if(fvy>maxv){fvy=maxv;}}
				
				fpx += 0.1 * fvx;
				fpy += 0.1 * fvy;
				
				if(fpx<0){ fpx=0;fvx=0;}else{ if(fpx>frw){fpx=frw;fvx=0;} }
				if(fpy<0){ fpy=0;fvy=0;}else{ if(fpy>frh){fpy=frh;fvy=0;} }
				
				//liqapp_log("imagefloat: writing new");
				
				liqcell_propsets_printf(self,"imagefloat","%f,%f,%f,%f",fvx,fvy,fpx,fpy);
				
				
				//liqapp_log("imagefloat: getting dims");
				
				int rw=w*1.3;
				int rh=h*1.3;
				
				int xx=x- (fpx*w/cw);
				int yy=y- (fpy*h/ch);
				
 
			}

 */
			
			{


				//################################################## lj variaton
				unsigned long timer1 = liqapp_GetTicks();
				
				float secs=0;// = ((float)timer1 / 1000.0);
				
				
				
				
				int rs=sscanf(fstr,"%f",&secs);
				
				
				
				liqcell_propsets_printf(self,"imagefloat","%f,%f,%f,%f",secs+0.1,0,0,0);
				
				
				

				
				unsigned int rndofname = *(unsigned int *)&self->name;
				
				secs+=rndofname;



		
				#define PI 3.141592654
				
				float lrw=(w*0.3)/2;
				float lrh=(h*0.3)/2;
				
				float lfx=2;
				float lfy=3;
				
				float lalpha = 2 * PI * secs / 50;
				float lx = lrw * cos(lfx * lalpha);
				float ly = lrh * sin(lfy * lalpha);
				
				int rw=w*1.3;
				int rh=h*1.3;

				int xx=x-(lrw-lx);
				int yy=y-(lrh-ly);
				
				
				//http://jrgraphix.net/research/flash-lissajous-curves.php
		
		
		

				//liqapp_log("imagefloat: '%s' orig  %4i,%4i - %4i,%4i     into     %4i,%4i - %4i,%4i",self->name,x,y,w,h,  xx,yy,rw,rh );


				if(liqcell_propgeti(self,"lockaspect",1)==1)
					liqcliprect_drawimagecolor(cr,self->image,xx,yy,rw,rh,1);
				else
					liqcliprect_drawimagecolor(cr,self->image,xx,yy,rw,rh,0);
					
				//liqapp_log("imagefloat: making dirty");
					
				liqcell_setdirty(self,1);
				
				//liqapp_log("imagefloat: done");

			}
		}
		else
		{

				//liqapp_log("imagestd: '%s' draw  %4i,%4i - %4i,%4i    ",self->name,x,y,w,h );

			// standard..
			if(liqcell_propgeti(self,"lockaspect",1)==1)
				liqcliprect_drawimagecolor(cr,self->image,x,y,w,h,1);
			else
				liqcliprect_drawimagecolor(cr,self->image,x,y,w,h,0);
		}
		
	}

__tz_one("imagedone");




	liqcell *content = liqcell_getcontent(self);
	if(content)
	{
		//liqapp_log("content1 = %s",content->name);
		// 20090414_012212 lcuk : allow for content which is not visible to be added but make sure we dont render it
		if(liqcell_getvisible(content))
		{
			//liqapp_log("content2 = %s",content->name);
			// we have an content to work with
			// technically we need to map the onscreen coordinate system from the C based ones into its content
			if(liqcell_propgeti(self,"lockaspect",0)==1)
			{
				//liqapp_log("content3 = %s",content->name);

				float ar = calcaspect(liqcell_getw(content),liqcell_geth(content), w,h );
				int sw = liqcell_getw(content) * ar;
				int sh = liqcell_geth(content) * ar;
				int sx = x+(w-sw) / 2;
				int sy = y+(h-sh) / 2;
				//liqapp_log("content4 = %s   %i,%i   (%i,%i)",content->name,sw,sh,  liqcell_getw(content), liqcell_geth(content));
				liqcell_easypaint(content,cr,sx,sy, sw,sh);
			}
			else
			{
				//liqapp_log("content5 = %s",content->name);
				
				liqcell_easypaint(content,cr,x,y,w,h);
			}
		}
	}


__tz_one("contentdone");






	if(!self->sketch)
	{
		char *fn = liqcell_propgets(self,"sketchfilename",NULL);
		if(fn && liqapp_fileexists(fn))
		{
			liqcell_setsketch( self, liqsketch_newfromfile(fn) );
		}
	}


__tz_one("sketchprep");

	if(self->sketch)
	{

		//liqcliprect_drawimagecolor(cr,camimage,x,y,w,h,0);
		if(liqcell_propgeti(self,"sketchediting",0)==1)
		{
			//liqapp_log("edit mode ahoy!");
			liqcliprect_drawsketch(cr,self->sketch,x,y,w,h,4);
		}
		else
		{
			liqcliprect_drawsketch(cr,self->sketch,x,y,w,h,0);
		}


	}

__tz_one("sketchdone");



	if(!self->font)
	{
		char *fn = liqcell_propgets(self,"fontname",NULL);
		if(fn)
		{
			//
			int fs = liqcell_propgeti(self,"fontsize",0);
			if(fs>0)
			{
				// todo: lookup from font name to font filename, fot now, pass filename
				liqcell_setfont( self, liqfont_cache_getttf(fn, fs, 0) );
			}
		}
	}

__tz_one("fontprep");

	if(self->font)
	{
		//############################################################ setup the font to render

		if(0==liqfont_setview(self->font, (float)w/(float)self->w, (float)h/(float)self->h ))
		{
			//############################################################ get the caption to display

			char *caption = liqcell_getcaption(self);

			if(caption)
			{


				char timebuf[128]="";
				if( (self->classname) && (strcmp(self->classname,"time") ==0) )
				{
					char *timefmt = liqcell_propgets(  self,"timeformat",NULL);
					if(timefmt)
					{
						liqapp_format_strftime(timebuf,128,timefmt);
						caption = timebuf;
					}
				}


				int captionlen = strlen(caption);


				int xx=x;
				int yy=y;

				//############################################################ size it up

				int fw = liqfont_textwidth(self->font,caption);
				int fh = liqfont_textheight(self->font);


				//############################################################ get cursor

				int selstart = liqcell_propgeti(  self,"selstart",-1);
				int sellength = liqcell_propgeti(  self,"sellength",0);
				int cursorpos = liqcell_propgeti(  self,"cursorpos",-1);
				
				
				if(selstart>captionlen)selstart=captionlen;
				if(selstart+sellength>captionlen)sellength=captionlen-selstart;
				if(cursorpos>captionlen)cursorpos=captionlen;




				//############################################################ get textcolor
				unsigned char tcy=255;
				unsigned char tcu=128;
				unsigned char tcv=128;
				{
					t = liqcell_propgets(self,"textcolor",NULL);
					if(t)
					{
						//liqapp_log("textcolor :: '%s'",t);
						decodecolor(t, &tcy, &tcu, &tcv );
					}
				}



				//############################################################ get textalign
				// left, centre, right
				int ta = liqcell_propgeti(  self,"textalign",0);
						if(ta==0)
						{

						}
						else if(ta==1)
						{
							x+=w-fw;
						}
						else
						{
							x+=(w-fw)/2;
						}

				// always centre in y for now
				// add new as required
						//y+=(h-fh)/2;
						
				int tay = liqcell_propgeti(  self,"textaligny",0);
						if(tay==0)
						{

						}
						else if(tay==1)
						{
							y+=h-fh;
						}
						else
						{
							y+=(h-fh)/2;
						}
				//############################################################ draw it now

				if(selstart>=0)
				{
					if(fw>w)
					{

						// adjust offset of the text rendering to make sure cursor is on screen
						char *sel=strndup(caption,cursorpos);
						int tcx = liqfont_textwidth(self->font,sel);
						if(tcx > w)
						{
							x-= (tcx - w);
						
						
							char *more=strndup(caption+cursorpos,4);
							int tex = liqfont_textwidth(self->font,more);
							x-= tex;
							x-= 16;
							if(more)free(more);
							
							
						}
						if(sel)free(sel);
						
						

						// pre   ss     curs     se      
						// P1....P2......P3......P4.........
						//char *p1=strndup(caption                   ,selstart                                );
						//char *p2=strndup(caption+selstart          ,cursorpos-selstart                      );
						//char *p3=strndup(caption+selstart          ,sellength                               );
						//char *p4=strndup(caption+selstart+sellength,captionlen-(caption+selstart+sellength) );
											
					}
						
					
					
					// with selection
					if(sellength==0)
					{
						// draw normally, but add caret as well
						liqcliprect_drawtext_color( cr, self->font,  x,y, caption, tcy,tcu,tcv);
					}
					else
					{
						// draw upto selstart in normal color
						// draw all of sellength
						// draw the remainder
						
						
						int tx=x;



						char *alline = caption;


						if(selstart>0)
						{

							x=liqcliprect_drawtextn_color(  cr, self->font,  x,y, alline, selstart, tcy,tcu,tcv);
							alline+=selstart;
						}

						if(sellength>0)
						{
							char *sel=strndup(alline,sellength);
							int ttt = liqfont_textwidth(self->font,sel);
							if(sel)free(sel);
							liqcliprect_drawboxfillcolor(cr,x,y,ttt,fh,tcy,tcu,tcv);
							x=liqcliprect_drawtextn_color(  cr, self->font,  x,y, alline, sellength, 255-tcy,tcv,tcu);
							alline+=sellength;
						}
						{
						// now draw the last bit
							x=liqcliprect_drawtext_color(  cr, self->font,  x,y, alline,    tcy,tcu,tcv);
							alline+=sellength;
						}
						x=tx;
					}

					if(cursorpos>=0)
					{
						//x=xx;
						char *sel=strndup(caption,cursorpos);
						int ttt = liqfont_textwidth(self->font,sel);
						if(sel)free(sel);
						liqcliprect_drawboxfillcolor(cr,x+ttt-1,y,1,fh,40,40,20);
						liqcliprect_drawboxfillcolor(cr,x+ttt  ,y,1,fh,255,40,20);
						liqcliprect_drawboxfillcolor(cr,x+ttt+1,y,1,fh,40,40,20);

					}
					

				}
				else
				{
					// normal
					//liqcliprect_drawtextinside_color( cr, self->font,  x,y, w,h,caption, ta, tcy,tcu,tcv);
					liqcliprect_drawtext_color( cr, self->font,  x,y, caption, tcy,tcu,tcv);
				}


				x=xx;
				y=yy;

			}
		}
	}


__tz_one("fontdone");




	//liqcell *c=liqcell_lastchild(self);
	liqcell *c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{
			// work it!

			int ww = w;
			int sw = self->w;
			int hh = h;
			int sh = self->h;

			// 20090412_174750 lcuk : adjusted again because overflows were still occuring, this only effects the factors and shouldnt have a visual effect at all
			while(ww>16000){  ww>>=1; sw>>=1; }
			while(hh>16000){  hh>>=1; sh>>=1; }

			liqcell_easypaint(c,cr,x+c->x* ww/sw,y+c->y* hh/sh,c->w * ww /sw ,c->h * hh / sh);

			//liqcell_easypaint(c,cr,x+c->x* w/self->w,y+c->y* h/self->h,c->w * w /self->w ,c->h * h / self->h);

		}
		//c=liqcell_getlinkprev(c);
		c=liqcell_getlinknext(c);
	}


__tz_one("childdone");

	if(liqcell_getselected(self))
	{
		liqcell *overlay = liqcell_global_lookup(self,"overlay");
		if(overlay)
		{
			liqcell_easypaint(overlay,cr,x,y,w,h);
		}
	}


__tz_one("overlaydone");


	t = liqcell_propgets(self,"bordercolor",NULL);
	if(t)
	{
		//liqapp_log("textcolor :: '%s'",t);
		if(decodecolor(t, &bcy, &bcu, &bcv ))
		{

			liqcliprect_drawboxlinecolor(cr,x,y,w,h,bcy,bcu,bcv);
		}
	}


__tz_one("borderdone");

	{
			liqcell *p = liqcell_getlinkparent(self);
			if(p)
			{
				if(p->w < self->w)
				{
					// show a position indicator bar..
					
					
				}
				if(p->h < self->h)
				{
					// show a position indicator bar..
					
					//float sx = self->x;
					float sy = self->y;
					//float ww = w;
					//float sw = self->w;
					//float pw = p->w;
					float hh = h;
					float sh = self->h;
					float ph = p->h;		
					// ph = 480
					// sh = 2000
					// sy = -200
					// get the relative size of the knob as a fraction of total height             == (0.25)
					float f  = ph    / sh;
					// get the relative offset of the knob as a fraction of total height           == (0.10)
					float fy = (-sy) / sh;
					// transform the knob size (0.25) into the context of the parent height (480/2000)  == (0.06)
					float f2 = f * (ph / sh);						
					// transform the knob offset (0.1) into the context of the parent height 0.1 + (480/2000) == (0.124) and starting from the parent offset
					float fy2 = fy + (fy * (ph / sh));					
					liqcliprect_drawboxwashcolor(cr,x+w-4,y+(fy2*hh) ,12,(f2*hh),0,255);
					
				}
			}
	}

 
__tz_one("scrolldone");


	if(liqcell_getenabled(self)==0)
	{	

		liqcliprect_drawboxfadeoutcolor(cr,x,y,w,h, 100,128,128,  180);
	}


__tz_one("disablerdone");





	//liqcliprect_drawcolorcube(cr,x,y,w,h,80);
	//liqcliprect_drawboxlinecolor(cr,cr->sx,cr->sy, cr->ex-cr->sx, cr->ey-cr->sy ,128,240,28);





	//liqcliprect_drawboxlinecolor(cr,x,y,w,h,80,18,128);



	//######################################## debug boxes

// 20090422_010005 lcuk : ive switched these on often enough to want a proper flag
// 20090422_010018 lcuk : it cannot be long before i need proper param saving like original liqbase
// 20090422_010032 lcuk : however i must iron out how outside applications will handle preferences like this

	if(liqcell_showdebugboxes)
	{

		liqapp_log("#################################### liqcell easypaint (%i,%i) :: %s   xy(%i,%i)-wh(%i,%i)  ",self->w,self->h,self->name,x,y,w,h);

		//t = liqcell_propgets(self,"bordercolor",NULL);
		t = "rgb(100,100,100)";
		if(t)
		{
			//liqapp_log("textcolor :: '%s'",t);
			decodecolor(t, &bcy, &bcu, &bcv );

			liqcliprect_drawboxlinecolor(cr,x,y,w,h,bcy,bcu,bcv);


			static liqfont *infofont;
			if(!infofont)
			{
				infofont = liqfont_cache_getttf("/usr/share/fonts/nokia/nosnb.ttf", 14, 0);
			}
			if(0==liqfont_setview(infofont, 1,1 ))
			{
				char *cap=liqcell_getcaption(self);
				if(!cap || !*cap)
				{

				}
				else
				{
					char info[128];
					snprintf(info,sizeof(info),"%s xy(%i,%i) wh(%i,%i)",liqcell_getcaption(self), self->x,self->y,self->w,self->h);

					int ww=liqfont_textwidth (infofont,info);
					int hh=liqfont_textheight(infofont);
					int ys=y+(h-hh)/2;

					liqcliprect_drawtextinside_color( cr, infofont,  x,ys,    ww,hh, info,0, 255,128,128);
					
					
					if(liqcell_getclassname(self))
					{
				
						liqcliprect_drawtextinside_color( cr, infofont,  x,ys+hh, ww,hh, liqcell_getclassname(self)  ,0, 255,68,128);
					}
				}
			}
		}
	}
 


	//liqcliprect_drawboxfillcolor(cr,x,y,2,h,((tz[tzused-1]-tz[0]) % 16) * 16,((tz[tzused-1]-tz[0]) % 32) * 8,((tz[tzused-1]-tz[0]) % 64) * 4);


	// fully on screen now
	//if(self->dirty)liqcell_setdirty(self,0);


	liqcliprect_release(cr);

__tz_one("reldone");


// 20090614_185314 lcuk : moved kinetic till AFTER i am finished, seems reasonable enough :)



// 20090613_031908 lcuk : kinetic scaling hack test.  result: not too convincing in practice.  removed, leaving code for academic interest
/*	
	if( self->kineticy && self->linkparent->h)
	{
		float fy = ((float)self->kineticy) / ((float)self->linkparent->h);
		if(fy<0)fy=-fy;
		fy*=2;
		int nh = ((float)h) * (1-fy);
		//if(nh>h)nh=h;
		
		liqapp_log("kin h=%3i ky=%3i fy=%3.3f  nh=%3i",self->linkparent->h,self->kineticy,fy,nh);
		
		y += (h-nh)/2;
		h=nh;
		if(h<1)
		{
			liqcliprect_release(cr);
			return;
		}
		
		x+=(w-(w*(1-fy)))/2;
		w=(w*(1-fy));
		
	}
 
__tz_one("kinzoomdone");

*/	


	// 20090614_184351 lcuk : mmm in the kinetic boiloff, i adjust the position of the cell.
	// technically, this should also effect the current position I am attempting to render for..

	liqcell_kineticboiloff(self);






__tz_one("kindone");





// whole logging block put in place because of a serious performance bottleneck in some test code
// results 


// bad system took 871 units to render a very complex scene
// this is very bad


// 457 if i remove all fonts and backgrounds and colors, this is still bad because nothing is occuring


// something is wrong
// viewing the results of this logging shows the children of a control using 47ms
// but individually, each child only takes much less
// this is very curious, perhaps the tree is messed up.

// doubly confirmed, the inner child loop up there is messed up
// putting the loop back the way it is meant to be reverts the speed to normal

// 20090517_014125 lcuk : no it doesnt
// there is something else amiss here
// i am getting low fps on the calendar
// this never used to happen

#ifdef __tz_easypaint_debug

char buf[1024]="";
int i=0;
	while(i<tzused)
	{
		char buf2[32];
		snprintf(buf2,sizeof(buf2),"%3lu%c%c",  (i>0) ? tz[i]-tz[i-1] : 0   ,  tstr[i][0],tstr[i][1] );
		strncat(buf,  buf2, sizeof(buf)-strlen(buf));
		i++;
	}
	liqapp_log(">>%3lu:%s:%s",tz[tzused-1]-tz[0],buf,self->name);

#endif

}
