
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks
#include "liqapp.h"


#include "liqapp_prefs.h"

#include "liqcell.h"

#ifdef __cplusplus
extern "C" {
#endif

liqcell *prefsroot=NULL;


static liqcell *qpref(char *key,char *data)
{
	liqcell *self=liqcell_quickcreatedata(key,"pref.x",data);
	//liqcell_keychange(self,key);
	//liqcell_titlechange(self,data);
	return self;
}




int liqapp_prefs_load()
{

	// ok, now we load user preferences
	
	liqapp_log("liqapp_prefs load :: %s","liqbase");
	
	if(!prefsroot)
	{
		liqapp_log("liqapp_prefs allocating root");
		prefsroot = liqcell_quickcreatenameclass("liqbase","prefs");
	}
	
	

	// lines will be something like:
	// todo: build proper parser for prefs trees or use some other library
	
	// begin [section]
	//     [field] = [data]
	//     [field] = [data]
	//     [field] = [data]
	// end
	

	
	
		
	FILE *fn;
	
	char buf[FILENAME_MAX+1];
	snprintf(buf,FILENAME_MAX,"%s/%s.prefs",app.userdatapath,"liqbase");
	
	fn=fopen(buf,"r");
	if(fn)
	{
		char lineraw[512];
		int linemax=512;
		char *line=NULL;
		while(!feof(fn))
		{
			char * rc;
			rc=fgets(lineraw,linemax, (FILE*) fn);
			if(!rc)break;

			line=lineraw;
			if(*line)
			{
				while(*line==' ' || *line=='\t')line++;
				
				if(*line=='#')
				{
					// comment line
				}
				else
				{
					
					//char *data=instr(line,"=");
					
					char *data=strchr(line,'=');
					if(data && data>line)
					{
						
						//liqapp_log("Test '%c' '%s'",*data,data);
						*data=' ';
						// rtrimming
						char *t=data;
						while(t>line && (*t==' ' || *t=='\t'))
						{
							*t-- = 0;
						}
						
						data++;
						// ltrimming to avoid the spaces
						while(*data==' ' || *data=='\t') data++;
						
						// do some rtrimming :)
						char *rtrim=data;
						while(*rtrim)rtrim++;
						if(rtrim>data && *rtrim==0)rtrim--;
					
						while(rtrim>=data)
						{
							if(*rtrim=='\n' || *rtrim=='\r' || *rtrim==10 || *rtrim==13 || *rtrim==' ' || *rtrim=='\t')
							{
								//liqapp_log("rtrim cut '%c':%i",*rtrim,*rtrim);
								*rtrim-- = 0;
							}
							else
							{
								//liqapp_log("rtrim fin '%c':%i",*rtrim,*rtrim);
								break;
							}
						}
						//liqapp_log("pref: '%s'='%s'  %i,%i,%i",line,data,data[0],data[1],data[2]);
						
						//char blankstr[4]={0,0,0,0};
						
						liqcell_child_insertsorted( prefsroot, qpref(line,strdup(data)) );
						

					}
				}
			}
		}
		
		fclose(fn);
		
		liqapp_log("liqapp_prefs_load read '%s'",buf);
		//return 0;
	}
	else
	{
		liqapp_log("liqapp_prefs_load could not open '%s'",buf);
		return -1;
	}

	return 0;
	
}



char * liqapp_pref_setvalue_vprintf(char *prefkey,char *prefformat, va_list arg)
{
    char       buf[2048];
	vsnprintf(buf,2048,prefformat,arg);
	return liqapp_pref_setvalue(prefkey,buf);
}
char * liqapp_pref_setvalue_printf(char *prefkey,char *prefformat, ...)
{
	va_list arg;
	va_start(arg, prefformat);
	char *res = liqapp_pref_setvalue_vprintf(prefkey,prefformat, arg);
	va_end(arg);
	return res;
}



char * liqapp_pref_setvalue(char *prefkey,char *prefvalue)
{
	if(prefvalue)	prefvalue=strdup(prefvalue);
	liqcell *p=liqcell_child_lookup(prefsroot,prefkey);
	if(p)
	{
		char *x=(char *)liqcell_getdata(p); if(x)free(x);
		liqcell_setdata(p,prefvalue);
		return prefvalue;
	}
	liqcell_child_insertsorted( prefsroot, qpref(prefkey,prefvalue) );
	return NULL;
}
char * liqapp_pref_getvalue_def(char *prefkey,char *defaultifmissing)
{
	liqcell *p=liqcell_child_lookup(prefsroot,prefkey);
	if(p)
	{
		return (char *)liqcell_getdata(p);
	}
	else
	{
		return defaultifmissing;
	}
	return NULL;
}

char * liqapp_pref_getvalue(char *prefkey)
{
	liqcell *p=liqcell_child_lookup(prefsroot,prefkey);
	if(p)
	{
		return (char *)liqcell_getdata(p);
	}
	return NULL;
}
liqcell *liqapp_pref_getitem(char *prefkey)
{
	liqcell *p=liqcell_child_lookup(prefsroot,prefkey);
	if(p)
	{
		return p;
	}
	return NULL;
}
int liqapp_pref_checkexists(char *prefkey)
{
	liqcell *p=liqcell_child_lookup(prefsroot,prefkey);
	if(p)
	{
		return 1;
	}
	return 0;
}	
	
int liqapp_prefs_save()
{
	FILE *fn;
	char buf[FILENAME_MAX+1];
	snprintf(buf,FILENAME_MAX,"%s/%s.prefs",app.userdatapath,"liqbase");
	
	fn=fopen(buf,"w");
	if(fn)
	{

		char 		fmtnow[255];
	 	liqapp_formatnow(fmtnow,255,"yyyymmdd_hhmmss");

		fprintf(fn,"# liqbase :: %s preferences : %s\n","liqbase",fmtnow);
		fprintf(fn,"begin prefs\n");
		liqcell *c=prefsroot->linkchild;
		while(c)
		{
			fprintf(fn,"\t%s=%s\n",c->name,(char *)liqcell_getdata(c));		// todo ensure multiline data pushed ok, and pulled and extracted correctly
			c=c->linknext;
		}
		fprintf(fn,"end\n");
		fclose(fn);
	}
	else
	{
		liqapp_log("liqapp_prefs_save could not open '%s'",buf);
		return -1;
	}
	
	return 0;
}

#ifdef __cplusplus
}
#endif

