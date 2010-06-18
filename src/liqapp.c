/**
 * @file	liqapp.c
 * @author  Gary Birkett
 * @brief 	App level helper functions
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

#include "liqcell.h"

#include "liqapp_prefs.h"
#include "liqapp_hildon.h"

liqcell *liqapp_logbase=NULL;

liqapp app={0};



int (*liqapp_log_forwarding)(void *contextdata, char *logtime, char *logdata) = NULL;

void * liqapp_log_context=NULL;

void liqapp_log_setforwarding( void *logfunction_voidpcontext_strtime_strmsg, void *contextdata )
{
    liqapp_log_forwarding = logfunction_voidpcontext_strtime_strmsg;
    liqapp_log_context=contextdata;
}

// using komodo edit configured to look like vb
// using winscp towards the device and putty
// ssh in windows, openssh from here http://sshwindows.sourceforge.net/download/
// can komodo use ssh based commands for "make"?
// i hope it can because then i get error highlighting and direct no messing builds

char *liqapp_pwd=NULL;
int   liqapp_is_basefs=0;

void liqapp_initpwd()
{
	// get initial PWD
	char buf[FILENAME_MAX + 1];
	if (getcwd(buf, sizeof(buf)) == NULL)
	{
		liqapp_log("pwd: failed");
	}
	// now test to see if we have a liqbase_base_fs available :)
	char buf2[FILENAME_MAX + 1];
	snprintf(buf2, FILENAME_MAX,"%s/liqbase_base_fs", buf);
	struct stat     statbuf;
	if(stat(buf2, &statbuf) == -1)
	{
		// no base_fs to work in
		liqapp_pwd = strdup(buf);
		liqapp_is_basefs = 0;
	}
	else
	{
		// we have a base_fs available, ensure all MEDIA calls are prepended with this
		liqapp_pwd = strdup(buf);
		liqapp_is_basefs = 1;
	}
}


char *liqapp_gettitle()
{
	return app.title;
}


//#######################################################################
//#######################################################################
//#######################################################################

	static const char *hardware_product_filename = "/proc/component_version";
    static char *hardware_product[32]={0};
	static int   hardware_product_ispowerful;
	int hardware_product_read()
	{
        hardware_product[0]=0;
		FILE *fd;
		int rs;
		fd = fopen(hardware_product_filename, "r");
		if(fd==NULL){ liqapp_log("hardware_product_read, cannot open for reading"); return -1;}	
		char tit[32]={0};
		char dat[32]={0};
        while(!feof(fd))
		{
            dat[0]=0;
            tit[0]=0;
            rs=fscanf((FILE*) fd,"%10s %10s\n",tit,dat);
            tit[31]=0;
            dat[31]=0;
            //liqapp_log("hardware_product_read version data line: '%s'='%s' %i",tit,dat,rs);
            if(rs != 2){ liqapp_log("hardware_product_read, cannot read information"); fclose(fd); return -2;}
            if(strcasecmp(tit,"product")==0)
            {
                strncpy(hardware_product,dat,32);
                break;
            }       
        }        
        fclose(fd);
        if(strcasecmp(tit,"product")==0)
        {
            snprintf(hardware_product,sizeof(hardware_product),"%s",dat);
            return 0;
        }      
        liqapp_log("hardware_product_read, did not find product line");
        return 0;
	}

/**
 * hardware_product_read return the "product" field from /proc/component_version
 * @return string containing product field "RX-44" etc
 */


	char * liqapp_hardware_product_get()
    {
        if(hardware_product[0]==0)
        {
			hardware_product_ispowerful = 0;
			
            if( hardware_product_read() != 0 )
				return NULL;
			
			// quick check for devices we know require limiting
			
			if( strcasecmp("RX-34", hardware_product ) == 0)
			{
				hardware_product_ispowerful = 0;
			}
			else
			if( strcasecmp("RX-44", hardware_product) == 0)
			{
				hardware_product_ispowerful = 0;
			}
			else
			{
				// unlimited
				hardware_product_ispowerful = 1;
			}
			
        }
        return hardware_product;
    }
    
 /*   
    if( strcasecmp("RX-44", liqapp_hardware_product_get() ) ==0 )
    {
        // this is n810
    }
 */
 
 
	int         liqapp_hardware_product_ispowerful_get()
	{
		if(hardware_product[0]==0)
        {
            liqapp_hardware_product_get();
        }
		return hardware_product_ispowerful;
 	}

//#######################################################################
//#######################################################################
//#######################################################################
static int __nsleep(const struct timespec *req, struct timespec *rem)
{
    struct timespec temp_rem={0};
    if(nanosleep(req,rem)==-1 && errno == EINTR)
	{
		//liqapp_log("early ret %ul,%ul",temp_rem.tv_sec,temp_rem.tv_sec);
        return __nsleep(rem,&temp_rem);
	}
	else
        return 1;
}

/**
 * put the calling thread to sleep for a duration
 * @param duration in milliseconds
 * @return 1 always
 */
int liqapp_sleep(unsigned long millisec)
{
	
    struct timespec req={0},rem={0};
    time_t sec=(int)(millisec/1000);
    millisec=millisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=millisec*1000000L;
    __nsleep(&req,&rem);
    return 1;
}
//#######################################################################
//####################################################################### GetTicks
//#######################################################################

/**
 * return system tick count
 * @return unsigned long for system tick count
 */

unsigned long liqapp_GetTicks()
{
	struct timeval now;
	unsigned long ticks;

	gettimeofday(&now, NULL);
	ticks=(now.tv_sec)*1000+(now.tv_usec)/1000;
	return(ticks);
}

/**
 * obtain an FPS value based on duration and frames completed in that period
 * @param starting time
 * @param ending time
 * @param number of frames completed in the period
 * @return float number of frames per second
 */


float liqapp_fps(unsigned long ts,unsigned long te,unsigned long framecount)
{
	if(te==ts || framecount==0)
		return 0;
	
	return (float)framecount / (   (float)(te-ts)   *    0.001   );
}
//#######################################################################
//####################################################################### Opt handling
//#######################################################################

int liqapp_getopt_find(char *optname)
{
	// return -1 if the option is not listed
	// otherwise, return the position
	// pass in a null pointer to indicate first unnamed option
	if(optname==NULL)
	{
		return 0;
	}
	else
	{
		int opt;
		for(opt=1;opt<app.argc;opt++)
		{	
			char *label=app.argv[opt];
			if( label[0] == '-'   &&   (strcmp( &label[1] , optname )==0) )
			{
				return opt;
			}
		}
		return -1;
	}	
}

int liqapp_getopt_exist(char *optname)
{
	// check if an option exists
	int opt = liqapp_getopt_find(optname);
	if(opt==-1)	// not found
		return 0;
	return 1;
}
int liqapp_getopt_hasarg(char *optname)
{
	// check if an option has arguments
	int opt = liqapp_getopt_find(optname);
	if(opt==-1)	// not found
		return 0;
	if(app.argc<opt+1) // no value to go along with it
		return 0;
	if( app.argv[opt][0] == '-' )	// no parameter specified
		return 0;
	return 1;
}
char *liqapp_getopt_str(char *optname,char *def)
{
	// get a named option and return as a string
	int opt = liqapp_getopt_find(optname);
	if(opt==-1)	// not found
		return def;
	if(app.argc<=opt+1) // no value to go along with it
		return def;
char *res=app.argv[opt+1];
	if( res[0] == '-' )	// no parameter specified (we moved onto another -option without specifying anything)
		return def;
	return res;
}




int liqapp_getopt_int(char *optname,int def)
{
	// get a named option and return as an integer value
char *param = liqapp_getopt_str(optname,NULL);
	if(param==NULL)
		return def;

char *paramend=NULL;
int res = strtol(param, &paramend, 0);
	if(paramend==NULL)
		return def;
	return res;
}









/**
 * check if a folder is valid and exists and can be 'stat'ed
 * @param char *pathname including path
 * @return int 1 if success, 0 otherwise
 */

int liqapp_folderexists(char *pathname)
{
	struct stat     statbuf;
	if(stat(pathname, &statbuf) == -1)
	{
		return 0;
	}
	if ( S_ISDIR(statbuf.st_mode) )
	{
		// its a folder!
		return 1;
	}
	// its a file..
	return 0;
}

/**
 * check if a pathname is valid and exists and can be 'stat'ed
 * @param char *pathname including path
 * @return int 1 if success, 0 otherwise
 */

int liqapp_pathexists(char *pathname)
{
	struct stat     statbuf;
	if(stat(pathname, &statbuf) == -1)
	{
		return 0;
	}
	return 1;
}

/**
 * check if a filename is valid and exists and can be 'stat'ed
 * @param char *filename including path
 * @return int 1 if success, 0 otherwise
 */
int liqapp_fileexists(char *filename)
{
	struct stat     statbuf;
	if(stat(filename, &statbuf) == -1)
	{
		return 0;
	}
	if ( S_ISDIR(statbuf.st_mode) )
	{
		// its a folder..
		return 0;
	}
	// its a file (or at least, not a folder..)!
	return 1;
}


/**
 * return the stat filesize (-1 if not found)
 * @param char *filename including path
 * @return int size if success, -1 otherwise
 */
int liqapp_filesize(char *filename)
{
	struct stat     statbuf;
	if(stat(filename, &statbuf) == -1)
	{
		return -1;
	}
	
	return statbuf.st_size;
}



/**
 * walk over the filename to remove all path parts
 * @param char *filename including path
 * @return char * to the first character of the filename itself, or to the start of the filename if no '/' detected 
 */
char *liqapp_filename_walkoverpath(char *filename)
{
	if(!filename || *filename==0)
	{
		return filename;
	}
	char *fnstart = filename;
	char *fnend  = filename;
	// walk quickly to the end
	while(*fnend)
	{
		//todo:make path handling safe
		if(*fnend=='/') fnstart=fnend+1;
		fnend++;
	}
	return fnstart;
}
/**
 * find and return the extension part of a filename
 * @param char *filename to get the extension from
 * @return char * to the first character of the extension, or to the start of the filename if no '.' detected 
 */
char *liqapp_filename_walktoextension(char *filename)
{
	filename = liqapp_filename_walkoverpath(filename);
	
	if(!filename || *filename==0)
	{
		return filename;
	}
	
	char *fnstart = filename;
	char *fnend  = filename;
	// walk quickly to the end
	while(*fnend)
	{
		//todo:make path handling safe
		if(*fnend=='.') fnstart=fnend+1;
		fnend++;
	}
	return fnstart;
}


//http://www.koders.com/cpp/fid0B921B251D9F6C17A083FBD5C9565285C637C785.aspx?s=md5
int liqapp_file_copy (char * from, char * to, int allowoverwrite)
{
    size_t nmemb;
    //int nmemb;
    FILE *ifp, *ofp;
    char buf[BUFSIZ];

    if (!allowoverwrite) {
        if (access(to, F_OK) == 0) {
            //OUTLOG((FUNC, TRWRN, "file %s already exists\n", to));
            return(-1);
        }
        else if (errno != ENOENT) {
            //OUTLOG((FUNC, TRERR, "access(%s, F_OK) failed\n", to));
            return(-2);
        }
    }

    if ((ifp=fopen(from, "r")) == NULL) {
        //OUTLOG((FUNC, TRERR, "%s doesn't exist\n", from));
        return(-3);
    }
    
    if ((ofp=fopen(to, "w+")) == NULL) {
        //OUTLOG((FUNC, TRERR, "can't create %s\n", to));
        fclose(ifp);
        return(-4);
    }

    while ((nmemb=fread(buf, 1, sizeof(buf), ifp)) > 0) {
        if (fwrite(buf, 1, nmemb, ofp) != nmemb) {
            //OUTLOG((FUNC, TRERR, "fwrite failed\n"));
            fclose(ifp);
            fclose(ofp);
            return(-5);
        }
    }

    fclose(ifp);
    fclose(ofp);

    return (0);
}




//#######################################################################
//####################################################################### app init
//#######################################################################



/**
 * Initiates the Liqbase Application to be ran, including the liqbase filesystem
 * @param argc Command line arguement count
 * @param argv Command line arguement strings
 * @param title The title of the application 
 * @param version The version of the application
 * @return int 0 for success
 */
int 		liqapp_init(int argc, char* argv[],char *title,char *version)
{
	

	app.infologgingenabled = 1;
	app.argc = argc;
	app.argv = argv;
	app.title = strdup(title);
	app.version = strdup(version);
	app.infologgingenabled = 1;
	
	app.username=NULL;
	
	

	liqapp_initpwd();
	
	







	{
		//
		char buf2[FILENAME_MAX+1];
		snprintf(buf2,FILENAME_MAX,"%s/.liqbase/%s.run.log",getenv("HOME"),app.title);
		if(liqapp_fileexists(buf2))
		{
			// try to remove it
			remove(buf2);
			// could we? do we care? not for now
		}
		
	}
	
	
	
	
	
	liqapp_log("");
	liqapp_log("########################################################");
	liqapp_log("");
	
	
	liqapp_log("Welcome to %s ver %s",app.title,app.version );
	
	
	
	

	

	//liqapp_log("%s",ver);
	liqapp_log("You passed %i arguments",argc);
	int i;
	for(i=0;i<argc;i++)
	{
		liqapp_log("Argument %i = %s",i,argv[i]);
		if( strcmp(argv[i],"--showdebugboxes")==0)
		{
			liqcell_showdebugboxes = 1;
		}
		if( strcmp(argv[i],"--showfps")==0)
		{
			liqcell_showfps = 1;
		}		
	}




	liqapp_hildon_init();
	
	// 20090413_143410 lcuk : starting off in turbo mode, i keep forgetting :)
#ifdef SUPPORT_GOVERNOR
	liqapp_turbo_start();
#endif


	//####################################################### initialize base folders
	char cwd[FILENAME_MAX + 1]="";
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		liqapp_log("liqapp error could not get cwd");
	}
	

	char *envhome = getenv("HOME");
	if(!envhome)
	{
		// wrong, but will suffice
		envhome="/home/user";
	}
	
	char buf[FILENAME_MAX+1];
	
	
	//####################################################### force root redirection (so that it finds a filled .prefs file and stuff)
	
	snprintf(buf,FILENAME_MAX,"%s/.liqbase/liqbase.force.home.user",envhome);
	
	
	if(liqapp_fileexists(buf))
	{
		// just to try to make sure we use here instead
		envhome="/home/user";
	}


	// 20090724_154719 lcuk : todo jaffa says use this: http://standards.freedesktop.org/basedir-spec/basedir-spec-0.6.html
	// 20090724_154726 lcuk : which makes sense really.
	
	
	//liqapp_log("~~~ Using std folder config");
	app.startpath = strdup(cwd);
	app.homepath = strdup(envhome);
	app.codepath = strdup("/usr/share/liqbase");
	snprintf(buf,FILENAME_MAX,"%s/.liqbase",app.homepath);
	app.userdatapath=strdup(buf);		


	//####################################################### make sure our data folder exists

	
		int trymakepath(char *path)
		{
			if(!liqapp_pathexists(path))
			{
				int status;
				status = mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				if(status)
				{
					liqapp_log("liqapp error: could not mkdir '%s'",path);
					return -1;			
				}
			}
			return 0;
		}


	snprintf(buf,FILENAME_MAX,"%s",app.userdatapath);
	trymakepath(buf);

	snprintf(buf,FILENAME_MAX,"%s/sketches",app.userdatapath);
	trymakepath(buf);

	snprintf(buf,FILENAME_MAX,"%s/cal",app.userdatapath);
	trymakepath(buf);	

	snprintf(buf,FILENAME_MAX,"%s/tags",app.userdatapath);
	trymakepath(buf);	

	snprintf(buf,FILENAME_MAX,"%s/ratings",app.userdatapath);
	trymakepath(buf);	

	snprintf(buf,FILENAME_MAX,"%s/thumbs",app.userdatapath);
	trymakepath(buf);	
	
	snprintf(buf,FILENAME_MAX,"%s/historythumb",app.userdatapath);
	trymakepath(buf);	

	snprintf(buf,FILENAME_MAX,"%s/liqbasecache",app.userdatapath);
	trymakepath(buf);
    
	snprintf(buf,FILENAME_MAX,"%s/layouts",app.userdatapath);
	trymakepath(buf);	
        
	//####################################################### load prefs

	liqapp_prefs_load();
	

	//####################################################### sort out principle user
	
	char *un = liqapp_pref_getvalue("username");
	if(un)
	{
		liqapp_log("got username from preferences '%s'",un);
		// we have the preference :)
		{
			if(app.username){free(app.username); app.username=NULL;}
			app.username=strdup(un);
			
			liqapp_ensurecleanusername(app.username);
		
		}
	}
	if(!app.username) app.username=strdup("user");


	liqapp_log("#############");
	liqapp_log("app.startpath    =%s",app.startpath);
	liqapp_log("app.codepath     =%s",app.codepath);
	liqapp_log("app.homepath     =%s",app.homepath);
	liqapp_log("app.userdatapath =%s",app.userdatapath);
	liqapp_log("#############");
    liqapp_log("app.hardware     =%s", liqapp_hardware_product_get() );
	liqapp_log("#############");
	liqapp_log("app.username     =%s",app.username);
	liqapp_log("#############");


	// run tests :)
	if(strcmp(app.title,"liqflow")==0)
	{
		liqcell_parse_liqbrain_test();
	}
	
	liqcell_showfps =        1 == atoi(liqapp_pref_getvalue_def("showfps","0"));
	liqcell_showdebugboxes = 1 == atoi(liqapp_pref_getvalue_def("showdebugboxes","0"));
	
	return 0;
	
}



void liqapp_ensurecleanusername(char *usernamewhichismodifiable)
{
	char *nu=usernamewhichismodifiable;
	while(*nu)
	{
		if(*nu=='\\' || *nu=='/' || *nu=='\'' || *nu=='~' || *nu==' ' || *nu=='\t' || *nu=='\'' || *nu=='"' || *nu==':' || *nu=='|' ||*nu=='#' || *nu=='.' )
			*nu++ = '_';
		else
			nu++;
	}
						
	
}


int liqapp_usernamechange(char *newusername)
{
    if(app.username){ free(app.username); app.username=NULL; }
    
    app.username = strdup( newusername );
    return 0;
}


static int liqapp_log_forwarding_count=0;


//############################################################# deeplog runs whenever called
/**
 * internal console logging function, va_list version
 * @param log entry template, uses printf formatting
 * @param list of parameters for completing the formatting
 * @return int 0 for success
 */
int liqapp_vdeeplog(char *logentry, va_list arg)
{
    time_t     now;
    struct tm  *ts;
    char       buf[80];
    time(&now);
    ts = localtime(&now);
    strftime(buf, sizeof(buf), "%H:%M:%S", ts);
    
    
    if( (liqapp_log_forwarding) && (liqapp_log_forwarding_count==0))
    {
        liqapp_log_forwarding_count++;
        
        char bufx[FILENAME_MAX+1];
        vsnprintf(bufx,sizeof(bufx),logentry,arg);
        liqapp_log_forwarding(liqapp_log_context, buf,bufx);
        
        liqapp_log_forwarding_count--;
        
    }
	
	/*
	
	// do some heavy duty log logging ;)
	if(liqapp_logbase==NULL)
	{
		liqapp_logbase = liqcell_quickcreatenameclass("liqapp_logbase","logbase");
	}
	{
		char msg[1024];
		vsnprintf(msg,sizeof(msg),logentry, arg);
		liqcell *logcell = liqcell_quickcreatenameclass(msg,"logitem");
		liqcell_propsets(logcell,"logentry", msg);
		liqcell_propsets(logcell,"date",     buf);
		liqcell_propsets(logcell,"user",    "gary");
		liqcell_propsets(logcell,"computer","home-gb");
		
		liqcell_child_append(liqapp_logbase,logcell);
	}
	*/	
	//printf("%s: %s: ",buf,app.title);
	printf("%s [%s] ",buf,app.username);

	vprintf(logentry, arg);
	
	puts("");	// dang! this feels bad
	fflush(stdout);

	// VERY temporary logging
	char buf2[FILENAME_MAX+1];
	snprintf(buf2,FILENAME_MAX,"%s/%s.run.log",app.title,app.userdatapath);
	FILE *fp=fopen(buf2,"a");
	if(fp)
	{
		fprintf(  fp, "%s "   , buf);		// append time
		vfprintf( fp, logentry, arg );		// now the log item
		fputs("\n",fp);
		fclose(fp);
	}

	return 0;
}
/**
 * internal console logging function, varargs version
 * @param log entry template, uses printf formatting
 * @param list of parameters for completing the formatting
 * @return int 0 for success
 */
int liqapp_deeplog(char *logentry, ...)
{
	va_list arg;
	va_start(arg, logentry);
	liqapp_vdeeplog(logentry, arg);
	va_end(arg);
	return 0;
}


//############################################################# local log runs only if info messages enabled
/**
 * console logging function
 * @param log entry template, uses printf formatting
 * @param list of parameters for completing the formatting
 * @return int 0 for success
 */
int liqapp_log(char *logentry, ...)
{
	//return 0;
	if(app.infologgingenabled==0) return 1;
	va_list arg;
	va_start(arg, logentry);
	liqapp_vdeeplog(logentry, arg);
	va_end(arg);
	return 0;
}


//############################################################# error log runs always

/**
 * Log a liqbase error, and exit the program
 * @param returnstatus Value to exit and return with
 * @param logentry The error message to log
 * @return int returnstatus
 */
int liqapp_errorandfail(int returnstatus,char *logentry)
{
	char buff[255];
	char *syserror = strerror(errno); // Get latest system error, enhancement by Zach
	if(!syserror) syserror="*UNKNOWN ERROR*";
	snprintf(buff,255,"FAILED: %i : %s : System Error : %s",returnstatus, logentry, syserror);
	liqapp_deeplog(buff);
	perror("\t!--liqbase::system error message--!:"); 
	exit(returnstatus);
	return returnstatus;
}

/**
 * Log a liqbase warning, but continue running
 * @param returnstatus Value to exit and return with
 * @param logentry The warning message to log
 * @return int returnstatus
 */
int liqapp_warnandcontinue(int returnstatus,char *logentry)
{
	char buff[255];
	snprintf(buff,255,"WARN: %i : %s",returnstatus,logentry);
	liqapp_deeplog(buff);
	return returnstatus;
}
//warnandcontinue


int liqapp_close()
{
	// 20090413_143448 lcuk : make sure we close off our turboness :)
	liqapp_turbo_reset();
	
	if(app.title) {free(app.title); app.title=NULL; }

	if(app.version) {free(app.version); app.version=NULL; }

	app.argc=0;
	app.argv=NULL;
	
	liqapp_hildon_close();
	
	
	

	return 0;

}


//#########################################################
//lcuk: formatted date function
//#########################################################
//todo: add multiple date formats - it currently gives one fixed result it needs to account for params

//example:
// char datestamp[20];
// liqapp_formatnow(datestamp,sizeof(datestamp),"yyyymmdd_hhmmss");

int liqapp_formatnow(char *buffer,int buffersize,char *format)
{

	struct tm 	*local;
	time_t 		t;
	  			t = time(NULL);
	  			local = localtime(&t);
if(strcmp(format,"yyyymmdd_hhmmss")==0)
{
	snprintf(buffer,buffersize,"%04i%02i%02i_%02i%02i%02i", 
			 1900+local->tm_year,
			 local->tm_mon+1,			// damn 0 based month..
			 local->tm_mday,
			 local->tm_hour,
			 local->tm_min,
			 local->tm_sec
			 );
}
else
if(strcmp(format,"yyyy/mm/dd")==0)
{
	snprintf(buffer,buffersize,"%04i/%02i/%02i", 
			 1900+local->tm_year,
			 local->tm_mon+1,			// damn 0 based month..
			 local->tm_mday
			 );	
}
else
	snprintf(buffer,buffersize,"need fix liqapp_formatnow"); 

	return 0;
}


char * liqapp_format_strftime(char *buffer,int buffersize,char *strftime_fmt)		// "%H:%M:%S"
{
	time_t     now;
	struct tm  *ts;
	time(&now);
	ts = localtime(&now);
	strftime(buffer, buffersize, strftime_fmt, ts);
	return buffer;
}









/*
** Designation:  StriStr
**
** Call syntax:  char *stristr(char *String, char *Pattern)
**
** Description:  This function is an ANSI version of strstr() with
**               case insensitivity.
**
** Return item:  char *pointer if Pattern is found in String, else
**               pointer to 0
**
** Rev History:  16/07/97  Greg Thayer  Optimized
**               07/04/95  Bob Stout    ANSI-fy
**               02/03/94  Fred Cole    Original
**               09/01/03  Bob Stout    Bug fix (lines 40-41) per Fred Bulback
**
** Hereby donated to public domain.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *stristr(const char *String, const char *Pattern)
{
      char *pptr, *sptr, *start;

      for (start = (char *)String; *start != 0; start++)
      {
            /* find start of pattern in string */
            for ( ; ((*start!=0) && (toupper(*start) != toupper(*Pattern))); start++)
                  ;
            if (0 == *start)
                  return NULL;

            pptr = (char *)Pattern;
            sptr = (char *)start;

            while (toupper(*sptr) == toupper(*pptr))
            {
                  sptr++;
                  pptr++;

                  /* if end of pattern then pattern was found */

                  if (0 == *pptr)
                        return (start);
            }
      }
      return NULL;
}

















//example:
// char datestamp[20];
// liqapp_formatnow(datestamp,sizeof(datestamp),"yyyymmdd_hhmmss");

// ...

// struct tm timebuf={0};
// liqapp_datestamp_to_date(datestamp,&timebuf);
// time_t w00t = mktime(&timebuf);
// if(w00t==-1)
// {
//		// invalid
// }


		int liqapp_datestamp_to_date(char *datestamp,struct tm *timebuf)		// convert a liqbase datestamp "yyyymmdd_hhmmss" into a tm struct
		{
			char yy[5]="0000";
			char mm[3]="00";
			char dd[3]="00";
			//_
			char HH[3]="00";
			char MM[3]="00";
			char SS[3]="00";
			
			char *indat=datestamp;
			int strncpy_onlydigits(char *res,char *indat,int size)
			{
				int ch;
				for(ch=0;ch<size;ch++)
				{
					if(isdigit(*indat))
					{
						*res++=*indat++;
					}
					else
					{
						return -1;
					}
				}
				// null terminate
				*res++=0;
				return 0;
			}
			//liqapp_log("has yr? '%s'",indat);
			if( strncpy_onlydigits(yy,indat,4) == 0 )
			{
				// ok, got it
				indat+=4;
			}
			else
			{
				// failed, exit
				return -1;
			}
			//liqapp_log("has mm? '%s'",indat);
			if( strncpy_onlydigits(mm,indat,2) == 0 )
			{
				// ok, got it
				indat+=2;
			}
			else
			{
				// failed, exit
				return -1;
			}
			
			//liqapp_log("has dd? '%s'",indat);	
		
			if( strncpy_onlydigits(dd,indat,2) == 0 )
			{
				// ok, got it
				indat+=2;
			}
			else
			{
				// failed, exit
				return -1;
			}
			
			//liqapp_log("has _? '%s'",indat);
			
			// allow an OPTIONAL _
			if(*indat=='_') indat++;
			
			// 20090722_004108 lcuk : you know, the time element may be optional...
			
			
			//liqapp_log("has hh? '%s'",indat);
						
			if( strncpy_onlydigits(HH,indat,2) == 0 )
			{
				// ok, got it
				indat+=2;
			}
			else
			{
				// failed, exit
				return -1;
			}
			
			//liqapp_log("has mm? '%s'",indat);
						
			if( strncpy_onlydigits(MM,indat,2) == 0 )
			{
				// ok, got it
				indat+=2;
			}
			else
			{
				// failed, exit
				return -1;
			}
				
				
			//liqapp_log("has ss? '%s'",indat);
			if( strncpy_onlydigits(SS,indat,2) == 0 )
			{
				// ok, got it
				indat+=2;
			}
			else
			{
				// failed, exit
				return -1;
			}
			
			//liqapp_log("ok %i %i %i _ %i %i %i", atoi(yy),atoi(mm),atoi(dd), atoi(HH),atoi(MM),atoi(SS) );
			// now, fill in the details :)
			timebuf->tm_year=atoi(yy)-1900;
			timebuf->tm_mon=atoi(mm)-1;
			timebuf->tm_mday=atoi(dd)-1;
			timebuf->tm_hour=atoi(HH);
			timebuf->tm_min=atoi(MM);
			timebuf->tm_sec=atoi(SS);
			return 0;

		}
			
			
		// similar code here
		// http://www.velocityreviews.com/forums/t679803-time-issue-mktime-timet-tm.html
		// found after trying to find problem
