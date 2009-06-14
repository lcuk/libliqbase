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
 * File buffer routines: opening a file and identifying different states
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "liqapp.h"

#include "filebuf.h"



//##########################################################################	
//##########################################################################	filebuf class.  quick and dirty way to load a named file
//##########################################################################	

//##########################################################################	

int 					filebuf_open(struct filebuf *self,char *filename)
{
	self->filename=NULL;
	self->filelength=0;
	self->filedata=NULL;
	
	//app_log("filebuf_open, '%s'",filename);
	self->filename = strdup(filename);
	if(self->filename==NULL)
	{
    	{ return liqapp_warnandcontinue(-1,"filebuf open, name strdup allocation problem"); }
	}
		
		
	//app_log("filebuf_open, get file stats (used for size)");
	struct stat filestatbuf;
	int fs;
	fs = stat(self->filename, &filestatbuf);
	if (fs == -1)
	{
    	{ return liqapp_warnandcontinue(-1,"filebuf open, get stats problem"); }
	}
	// allow whole length for mapping
	self->filelength=filestatbuf.st_size;
	
	//app_log("filebuf_open, do some sanity checking");
    // restrict normal files to only 32mb of memory for now
    if(self->filelength>1024*1024*32) 
	{
		{ return liqapp_warnandcontinue(-1,"filebuf open, length exceeds current limit (32mb)"); }
	}

	//app_log("filebuf_open, allocate the memory");
    self->filedata = (char*)calloc(self->filelength, sizeof(char));
	if (self->filedata == NULL)
	{
    	{ return liqapp_warnandcontinue(-1,"filebuf open, can't allocate data memory"); }
	}	
	
	//app_log("filebuf_open, open the file");
FILE * fd;
	fd = fopen(self->filename, "r");
	if (fd == NULL)
	{
    	{ return liqapp_warnandcontinue(-1,"filebuf can't open file"); }
	}

	//app_log("filebuf_open, read the contents");
	
    int readcnt = fread(self->filedata, sizeof(char), self->filelength, (FILE*) fd);
	if(readcnt==0)
	{
		// give a chance to close the handle
		fclose(fd);
		
    	{ return liqapp_warnandcontinue(-1,"filebuf open, problem reading file"); }		
	}
	
	//app_log("filebuf_open, close the file");
	fclose(fd);

	//app_log("filebuf_open, and we are done");
	return 0;
	
}

//##########################################################################	

int						filebuf_close(struct filebuf *self)
{
	if(self->filename){	free(self->filename); self->filename=NULL; }
	if(self->filedata){	free(self->filedata); self->filedata=NULL; }
	self->filelength=0;
	return 0;
}

