
/**
 * @file	liqcell_dllcache.c
 * @author  Gary Birkett
 * @brief 	This file is the master file which loads and manages all dynamic widgets 
 * 			in the subdirectory
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


#include  <stdio.h>
#include  <stdlib.h>
#include  <dlfcn.h>
#include <elf.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>

#include "liqcell.h"
#include "liqcell_dllcache.h"


// for now, only use this function

// liqcell *dllcache_runconstructor(char *classname);

// small class dedicated to managing and handling requests for widgets of a specified class

// when the constructor: liqcell_quickcreatevis(name,classname,x,y,w,h) is called
// the classname is passed into here to attempt to construct the instance
// this will inturn examine all the *.so and */*.so files inside the widgets/ folder
// if a match is found then the contructor for that dll will be called

// note, a potential gotcha will occur with cyclic references and this should be catered for

// the first time this function is used, it will build up a cache of all the available .so files it finds

// this can be used to automatically initialize things like labels and boxes and stuff easily



//################################################### dllcache class


//typedef struct dllcacheitem
//{
//	char *key;
//	char *filename;
//	void *dll;
//	liqcell *(*constructor)();
//}
//	dllcacheitem;




//################################################### storage for the cache

static dllcacheitem *      dllcache     =NULL;
static int                 dllcache_size=0;
static int                 dllcache_used=0;




static dllcacheitem *runstack[256];
static int           runstack_size=256;
static int           runstack_used=0;


/**
 * prepare the cache
 */

int dllcache_init()
{
	// allow upto 256 at this point, may allow extension later but its enough to get started :)
	dllcache_size = 256;
	dllcache_used = 0;
	dllcache = (dllcacheitem*)calloc(dllcache_size, sizeof(dllcacheitem));
	if (dllcache == NULL)
	{
    	{ return liqapp_warnandcontinue(-1,"dllcache init, can't allocate data memory"); }
	}
	return 0;
}


/**
 * free the cache
 */

int dllcache_close()
{
	// close and free everything from the cache
	while(dllcache_used>0)
	{
		dllcacheitem * dllcacheitem = &dllcache[ --dllcache_used ];
		
		if(dllcacheitem->key)         { free(dllcacheitem->key);       dllcacheitem->key=NULL;         };
		if(dllcacheitem->filename)    { free(dllcacheitem->filename);  dllcacheitem->filename=NULL;    };
		if(dllcacheitem->dll)         { dlclose(dllcacheitem->dll);    dllcacheitem->dll=NULL;         };
		if(dllcacheitem->constructor) {                                dllcacheitem->constructor=NULL; };
	}
	free(dllcache);
	dllcache=NULL;
	dllcache_used=0;
	dllcache_size=0;
	return 0;
}













int dllcache_scan_dllfile(char *dll_filename)
{
	// we are gonna expand our dllcache
	
	if(dllcache_used==dllcache_size)
	{
		//
	   	{ return liqapp_warnandcontinue(-1,"dllcache_scan_dllfile, no more slots for this dll"); }
	}
	
	dllcacheitem * dllcacheitem=NULL;

	
	if(dll_filename==NULL)
	{
		// self, special case :)
		
		dll_filename = app.title;
		
		//################################################# alloc and initialize the cache item :)
		dllcacheitem = &dllcache[ dllcache_used++ ];
		dllcacheitem->key        = strdup(app.title);
		dllcacheitem->filename   = strdup(app.title);
		dllcacheitem->dll        = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL );//RTLD_NOW);
		dllcacheitem->constructor= NULL;

	}
	else
	{
	
		// initialize it in the normal way
		
		//################################################# prepare the name for usage as a key
		
		char *filetitle= liqapp_filename_walkoverpath(dll_filename);
			if(!filetitle){ return liqapp_warnandcontinue(-1,"dllcache_scan_dllfile, no filetitle"); }
		char *fileext  = liqapp_filename_walktoextension(filetitle);
			if(!fileext){ return liqapp_warnandcontinue(-1,"dllcache_scan_dllfile, no fileext"); }
		char filetitlenoext[256];
		// bug will exist here if i dont watch for it
		// 20090317_0231 lcuk : todo fix this still
		snprintf(filetitlenoext,((int)(fileext-filetitle))>255 ? 255 : ((int)(fileext-filetitle)),"%s",filetitle);
	
	
		//################################################# alloc and initialize the cache item :)
		dllcacheitem = &dllcache[ dllcache_used++ ];
		dllcacheitem->key        = strdup(filetitlenoext);
		dllcacheitem->filename   = strdup(dll_filename);
		dllcacheitem->dll        = dlopen(dll_filename, RTLD_LAZY | RTLD_GLOBAL );//RTLD_NOW);
		dllcacheitem->constructor= NULL;
	}
	
	//################################################# check for errors
	const char  *mydll_error;
	mydll_error = dlerror();
	if( mydll_error )
	{
		liqapp_log("dllcache_scandllfile failed to open library: '%s' err '%s'",dll_filename, mydll_error);
		return -1;
	}
	// finished ok
	
	liqapp_log("dllcache_scan_dllfile ok '%s'",dll_filename);
	return 0;
}



int dllcache_scan_folder(char *widgetpath)
{

	// scan all files inside the /widgets folder

	DIR           *	dir_p;
	struct dirent *	dir_entry_p;
	char 			fn[FILENAME_MAX+1];
	char          * ft;
	dir_p = opendir( widgetpath );
	if(!dir_p)
	{
		liqapp_log("dllcache_scanfolder opendir failed: '%s'",widgetpath);
		return -1;			// heh thanks kot :)
	}
	while( NULL != (dir_entry_p = readdir(dir_p)))
	{
		if( dir_entry_p->d_name[0]=='.' )
			continue;

		ft=dir_entry_p->d_name;
		if(*widgetpath)
			snprintf(fn , FILENAME_MAX , "%s/%s", widgetpath , ft);
		else
			snprintf(fn , FILENAME_MAX , "%s", ft);

		struct stat     statbuf;
		if(stat(fn, &statbuf) == -1)
		{
			liqapp_log("dllcache_scanfolder stat failed: '%s'",fn);
			return -1;
		}
		// check what kind of filesystem object we have
		if ( S_ISDIR(statbuf.st_mode) )
		{
			// its a subfolder
			char tryname[FILENAME_MAX];
			snprintf(tryname , FILENAME_MAX , "%s/%s.so", fn,ft);
			if(liqapp_fileexists(tryname))
			{
				//liqapp_log("dllcache_scanfolder deep trying to load dll: '%s'",tryname);				
				if( dllcache_scan_dllfile(tryname)!=0 )
				{
					// failed!
				}				
			}
		}
		else
		if ( S_ISREG(statbuf.st_mode) )
		{
			// its a regular file
			
			char *ext=liqapp_filename_walktoextension(ft);
			if(!ext || !*ext)
			{
				// nothing to see here..
			}
			else
			if(
				strcasecmp(ext,"so")==0
			  )
			{
				
				//liqapp_log("dllcache_scanfolder trying to load dll: '%s'",fn);				
				if( dllcache_scan_dllfile(fn)!=0 )
				{
					// failed!
				}
				// got it!
			}
		}
	}
	closedir(dir_p);
	return 0;
}


/**
 * scan the files to initialize the cache
 */

int dllcache_scan()
{
	if(dllcache_size==0)
	{
		if(dllcache_init()!=0)
		{
		   	{ return liqapp_warnandcontinue(-1,"dllcache scan, error init cache"); }			
		}
	}
	
	if(dllcache_used>0)
	{
		   	{ return liqapp_warnandcontinue(-1,"dllcache scan, already initialized"); }			
	}
	
	runstack_used=0;
	
	dllcache_scan_dllfile(NULL);
	
	if(dllcache_scan_folder(app.startpath)!=0)
	{
		//{ return liqapp_warnandcontinue(-1,"dllcache scan, error while scan '.'"); }			
	}	
	if(dllcache_scan_folder("widgets")!=0)
	{
		//{ return liqapp_warnandcontinue(-1,"dllcache scan, error while scan 'widgets'"); }			
	}
	
	if(dllcache_scan_folder("src/widgets")!=0)
	{
		// 20090607_193342 lcuk : cheat but might actually work
		//{ return liqapp_warnandcontinue(-1,"dllcache scan, error while scan 'src/widgets'"); }			
	}
	



	// 20090614_234707 lcuk : always make sure runstack used includes THIS dll
	// 20090614_234720 lcuk : &dllcache[ idx ];

	if(dllcache_used>0)
	{
			runstack[runstack_used++] = &dllcache[ 0 ];
			//liqcell *res = constructor();
			//runstack_used--;
	}


		
	return 0;
}





liqcell *dllcache_runconstructorinner(char *classname)
{
	// must have at least one item in the stack
	if(runstack_used==0)return NULL;
	
	//liqapp_log("runconstructorinner, looking for: '%s' (stack contains %i items)",classname,runstack_used);
	
	char symname[255];
	snprintf(symname,255,"%s_create",classname);

	// first thing to try is 
	int idx=0;
	for(idx=runstack_used-1;idx>=0;idx--)
	{
		dllcacheitem * dllcacheitem = runstack[ idx ];


		//liqapp_log("runconstructorinner, checking stack %i,%s for %s",idx,dllcacheitem->key,classname);
		
		
		liqcell *   	(*constructor)() = NULL;
		
		if(strcasecmp(dllcacheitem->key,classname)==0 )
		{
			// to be on the list here we MUST already have the default constructor..
			constructor = dllcacheitem->constructor;
		}
		else
		{
			// lets try and use whats available to us :)
			constructor = dlsym( dllcacheitem->dll, symname);
		}
		if(constructor)
		{
			//liqapp_log("runconstructorinner, found sym  %i,%s for %s",idx,dllcacheitem->key,classname);
			
			// only interested if we got it, dont care for errors really if we didnt
			
			// store the pointer to the library on the stack 
			runstack[runstack_used++] = dllcacheitem;
			liqcell *res = constructor();
			runstack_used--;
			if(res) return res;
			
			liqapp_log("runconstructorinner, error running : '%s.%s_create'",dllcacheitem->key,classname);
			return NULL;
			
		}
	}
	
	

}













/**
 * run a specified constructor and return the result
 */

liqcell *dllcache_runconstructor(char *classname)
{
	if(dllcache_size==0)
	{
		if(dllcache_scan()!=0)
		{
		   	{ liqapp_warnandcontinue(-1,"dllcache scan, error init cache"); }
			return NULL;
		}
	}
	//liqapp_log("runconstructor, looking for: '%s'",classname);
int idx=0;



	char *dot=strchr(classname,'.');
	if(dot)
	{
		//
		
		char lib[256];
		int libl = (dot-classname)+1;
		if(libl>255)libl=255;
		snprintf(lib,libl,classname);
		//liqapp_log("has dot! '%s' lib=='%s'",dot,lib);
		for(idx=0;idx<dllcache_used;idx++)
		{
			
			dllcacheitem * dllcacheitem = &dllcache[ idx ];
			
			//liqapp_log("runconstructor, testing: '%s' == '%s' ??",classname,dllcacheitem->key);
			
			if(strcasecmp(dllcacheitem->key,lib)==0 )
			{
				//liqapp_log("found lib.! '%s'",dot);
				runstack[runstack_used++] = dllcacheitem;
				liqcell *res = dllcache_runconstructor(&dot[1]);
				runstack_used--;
				if(res) return res;
				
			}
		}
	}
	
	
	
	liqcell *res = dllcache_runconstructorinner(classname);
	if(res)
	{
		// obtained by clandestine means!
		return res;
	}
	
	

	for(idx=0;idx<dllcache_used;idx++)
	{
		
		dllcacheitem * dllcacheitem = &dllcache[ idx ];
		
		//liqapp_log("runconstructor, testing: '%s' == '%s' ??",classname,dllcacheitem->key);
		
		if(strcasecmp(dllcacheitem->key,classname)==0 )
		{
			// this is the correct dll
			//
			//liqapp_log("runconstructor, matched! '%s'",classname);
			
			if(dllcacheitem->constructor==NULL)
			{
				liqapp_log("runconstructor, initializing constructor '%s'",classname);
				
				//################################################# find the constructor
				char symname[255];
				snprintf(symname,255,"%s_create",dllcacheitem->key);
				
				//################################################# get it!
				dllcacheitem->constructor = dlsym( dllcacheitem->dll, symname);

				//################################################# check for errors
				const char  *mydll_error;
				mydll_error = dlerror();
				if( mydll_error )
				{
					liqapp_log("dllcache_runconstructor missing constructor: '%s' err '%s'",symname, mydll_error);
					return NULL;
				}
			}
			
			//################################################# run the constructor
			
			//liqapp_log("runconstructor, running constructor '%s.%s_create'",dllcacheitem->key,classname);
			liqcell *   	(*constructor)();
			
			constructor = dllcacheitem->constructor;
			
			runstack[runstack_used++] = dllcacheitem;
			liqcell *res = constructor();
			runstack_used--;
			if(res) return res;
			
			liqapp_log("runconstructor, error running : '%s.%s_create'",dllcacheitem->key,classname);
			return NULL;
		}
	}
	// finding constructors is not good, but at the same time is it life threatening?
	//liqapp_log("runconstructor, error finding constructor for: '%s'",classname);

	return NULL;	
}


dllcacheitem *dllcache_getbase()
{
	if(dllcache_size==0)
	{
		if(dllcache_scan()!=0)
		{
		   	{ liqapp_warnandcontinue(-1,"dllcache_getbase, error init cache"); }
			return NULL;
		}
	}
	return dllcache;
}
int           dllcache_getsize()
{
	if(dllcache_size==0)
	{
		if(dllcache_scan()!=0)
		{
		   	{ liqapp_warnandcontinue(-1,"dllcache_getsize, error init cache"); }
			return 0;
		}
	}	
	return dllcache_size;
}
int           dllcache_getused()
{
	if(dllcache_size==0)
	{
		if(dllcache_scan()!=0)
		{
		   	{ liqapp_warnandcontinue(-1,"dllcache_getused, error init cache"); }
			return 0;
		}
	}	
	return dllcache_used;	
}
