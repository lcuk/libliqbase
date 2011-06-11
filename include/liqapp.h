/* liqbase
 * Copyright (C) 2008 Gary Birkett
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
 */

/*
 *
 * Header for app level helper functions
 *
 */




#ifndef liqapp_H
#define liqapp_H

#include <stdio.h>
#include <dirent.h>

#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks

#ifdef __cplusplus
extern "C" {
#endif

//#######################################################

typedef struct 
{
	// technically this is a session
	char *	title;
	int 	argc;
	char *	*argv;
	int 	infologgingenabled;
	char 	*version;
	void *	tag;
	
	
	
	char    *startpath;			// startin path  cwd
	char    *homepath;			// ~
	char    *userdatapath;			// ~/.liqbase
	char    *codepath;			// /usr/share/liqbase		// where code files live (should contain media and widgets subfolders)

	char    *username;			// lcuk or whatever is in ~/.liqbase/liqbase.prefs

	
} liqapp;

//#######################################################

extern liqapp app;

//#######################################################
unsigned long liqapp_GetTicks();
int 		liqapp_init(int argc, char* argv[], const char *title, const char *version);
int 		liqapp_log(const char *logentry, ...);
int 		liqapp_errorandfail(int returnstatus, const char *logentry);
int 		liqapp_warnandcontinue(int returnstatus, const char *logentry);
int 		liqapp_close();


void        liqapp_log_setforwarding( void *logfunction_voidpcontext_strtime_strmsg, void *contextdata );

char *		liqapp_gettitle();

const char *liqapp_hardware_product_get();
int         liqapp_hardware_product_ispowerful_get();

float 		liqapp_fps(unsigned long ts,unsigned long te,unsigned long framecount);
int 		liqapp_getopt_find(char *optname);
int 		liqapp_getopt_exist(char *optname);
int 		liqapp_getopt_hasarg(char *optname);
char *		liqapp_getopt_str(char *optname,char *def);
int 		liqapp_getopt_int(char *optname,int def);
int 		liqapp_sleep(unsigned long millisec);



int 		liqapp_formatnow(char *buffer,int buffersize, const char *format);
// 20090718_163211 lcuk : example of use:

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



int liqapp_datestamp_to_date(char *datestamp,struct tm *timebuf);		// convert a liqbase datestamp "yyyymmdd_hhmmss" into a tm struct


char * 		liqapp_format_strftime(char *buffer,int buffersize,char *strftime_fmt);			// use format specifiers to display time
char * 		liqapp_format_strftime_human(char *buffer,int buffersize,struct tm *ts);		// print a human readable (english only) string of time

int 		liqapp_folderexists(const char *pathname);
int   		liqapp_pathexists(const char *pathname);
int   		liqapp_fileexists(const char *filename);
int         liqapp_filesize(const char *filename);
const char *		liqapp_filename_walkoverpath(const char *filename);
const char *		liqapp_filename_walktoextension(const char *filename);
int   		liqapp_file_copy (const char * from, const char * to, int allowoverwrite);
int         liqapp_usernamechange(const char *newusername);

int         liqapp_url_wget(const char *url, char *resultfilename,int resultbufsize);

//#######################################################

void 		liqapp_turbo_start();
void 		liqapp_turbo_reset();

void 		liqapp_ensurecleanusername(char *usernamewhichismodifiable);

// had to exist somewhere
char *stristr(const char *String, const char *Pattern);

//#######################################################

#ifdef __cplusplus
}
#endif

#endif





