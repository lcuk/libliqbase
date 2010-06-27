
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>            
#include <fcntl.h>                                                                             
#include <unistd.h>
#include <errno.h>

#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks
#include "liqapp.h"

#ifdef __cplusplus
extern "C" {
#endif

//###################### not used yet, its still directly inside liqcell_easypaint



int liqapp_url_wget(const char *url, char *resultfilename, int resultbufsize)
{

	const char *fn = url;
	

			char cachefn[2048];
			snprintf(cachefn,sizeof(cachefn), "%s", fn  );
			// ok, let me try one thing..
			
			if( strncmp(fn,"http://",7) == 0)
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

				snprintf(longbuf,sizeof(longbuf), "%s/liqbasecache/%s",app.userdatapath, cachefn );
				
				strncpy(cachefn,longbuf,1024);
				
				
				liqapp_log("http checking '%s'",cachefn);
				
				
				//########################## does it already exist?
			/*
				if(liqapp_filesize(longbuf)>0)
				{
					// the file has already been downloaded! sweet!
					liqapp_log("http got valid file already..");
					strncpy(cachefn,longbuf,1024);
				}
				else
			*/
				{
			/*		
					// file might actually exist
					if( !liqapp_fileexists(longbuf) )
					{
						char killbuf[2048];
						snprintf(killbuf,sizeof(killbuf),"rm %s",longbuf);
						// hack to try to make sure theres a liqbasecache folder
						system(killbuf);
					}
			*/		
					
					
					liqapp_log("http about to download '%s' into '%s'",fn,cachefn);
					// doesn't exist yet
                    char xbuf[FILENAME_MAX]={0};
                    snprintf(xbuf,FILENAME_MAX,"%s/liqbasecache",app.userdatapath);
					if( !liqapp_pathexists(xbuf) )
					{
						// hack to try to make sure theres a liqbasecache folder
                        snprintf(xbuf,FILENAME_MAX,"mkdir %s/liqbasecache",app.userdatapath);
						system(xbuf);
					}
					
					
					
					// do the downloading
					char cmdbuf[2048];					
					snprintf(cmdbuf,sizeof(cmdbuf), "wget '%s' -N -q --user-agent=liqbase.sweb --output-document='%s'", fn , cachefn );
					
					liqapp_log("http command: %s",cmdbuf);
					
					int sok = system(cmdbuf);
					
					liqapp_log("http download result: %i, exists? %i",sok,liqapp_fileexists(cachefn));
					
					// check the result
					if(sok==0)
					{
						
						// ok
					}
					else
					{
						// fail
						return -1;
					}
				}
				
				fn=cachefn;
			}
			
			if( strncmp(fn,"file://",7) == 0 )
			{
				// hmmmm, interesting, remove this portion..
				fn=&fn[7];
			}
			
			strncpy(resultfilename,fn,resultbufsize);
			
			return 0;
}

#ifdef __cplusplus
}
#endif

