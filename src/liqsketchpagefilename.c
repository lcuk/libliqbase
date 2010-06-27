

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <ctype.h>


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




#include "liqsketchpagefilename.h"

#ifdef __cplusplus
extern "C" {
#endif




static char *filename_walkoverpath(char *filename)
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
		// or use a proper function ..
		if(*fnend=='/') fnstart=fnend+1;
		fnend++;
	}
	return fnstart;
}






char * pagefilename_rebuild(struct pagefilename *self,char *bufferresultfilename,int buffermax)
{
	// 20090422_202048 lcuk : todo make this account for the liqapp.username when its available
	// 20090422_202108 lcuk : it used to in the old system
		if(self->filepath[0])
		{
			if(self->fileuser[0])
				snprintf(bufferresultfilename,buffermax, "%s/liq.%s.%s.page.%s",self->filepath,self->filedate,self->fileuser,self->filetitle);
			else
				snprintf(bufferresultfilename,buffermax, "%s/liq.%s.%s.page.%s",self->filepath,self->filedate,"user",self->filetitle);
		}
		else
		{
			if(self->fileuser[0])
				snprintf(bufferresultfilename,buffermax,    "liq.%s.%s.page.%s",               self->filedate,self->fileuser,self->filetitle);
			else
				snprintf(bufferresultfilename,buffermax,    "liq.%s.%s.page.%s",                self->filedate,"user",self->filetitle);
		}
		
		
		
	//	snprintf(bufferresultfilename,buffermax,    "liq.%s.%s.page.%s",                "20090227_000120","gary","test");
		
		
		return bufferresultfilename;
}











// one of the most evil functions in the entire system


int pagefilename_breakapart(struct pagefilename *self,char *filename)
{
	//todo:parse this properly
	
	// "liq.[date].[user].page.[keyword]"
	// "liq.[date].[user].page"
	// "liq.[user].[date].page.[keyword]"
	// "liq.[user].[date].page"
	// "liq.[date].page.[keyword]"
	// "liq.[date].page"
	
	
	// nullify input
	self->filepath[0]=0;
	//self->filename[0]=0;
	self->fileuser[0]=0;
	self->filedate[0]=0;
	self->fileclass[0]=0;
	self->filetitle[0]=0;
	
	

	
	
	
	
	
	
	


	if(!filename || *filename==0)
	{
		return -1;
	}
	char *pthstart = filename;
	char *fnstart = filename_walkoverpath(filename);
	char *fnend  = filename;
	while(*fnend)
	{
		fnend++;
	}

	int len=0;

	len = fnstart-pthstart;
	if(len>0)len--;
	if(len>255) len=255;

	strncpy(self->filepath,pthstart,len);
	self->filepath[len+1]=0;
	
	//liqapp_log("cols :: trying :: %s",fnstart);
	
	//########################################### break into columns, removing the '.'s as we go
	char * inorig = strdup(fnstart);
		char *cols[80]={"X","X","X","X","X","X","X","X"};
		int colcount=0;
		char *indat=inorig;
		cols[0] = indat;
		colcount++;
		while(*indat && colcount<6)
		{
			if(*indat=='.')
			{
				*indat=0;
				indat++;
				cols[colcount++]=indat;
			}
			else
				indat++;
		}
		
		
		
		//liqapp_log("X cols :: %i :: %s :: %s :: %s :: %s :: %s",colcount,cols[0],cols[1],cols[2],cols[3],cols[4]);
		
		if(colcount<1)
		{
			liqapp_log("no cols :: trying :: %s",fnstart);
			// invalid..
			return -1;
		}

		
		if(strcmp(cols[0],"liq")!=0)
		{
			return -1;
		}
		//if(strcmp(cols[2],"page")!=0) return -1;
		//if(strcmp(cols[3],"page")!=0) return -1;
		
		switch(colcount)
		{
			case 3:
					
				//liqapp_log("cols :: %i :: %s :: %s :: %s",colcount,cols[0],cols[1],cols[2]);
					
				if(isdigit(*cols[1]) && (strcmp(cols[2],"page")==0))
				{
					// liq.[date].page  // ancient
					
					strncpy(self->filedate, cols[1],15); self->filedate[15]=  0;
					//strncpy(self->fileuser, cols[ ],15);self->fileuser[15]=0;
					strncpy(self->fileclass,cols[2],20);self->fileclass[20]=0;
					//strncpy(self->filetitle,cols[ ],20);self->filetitle[20]=0;
					free(inorig);
					return 0;
				}
				// should never occur.. heh, final last words
				free(inorig);
				return -1;

			case 4:
				
				
				//liqapp_log("cols :: %i :: %s :: %s :: %s :: %s",colcount,cols[0],cols[1],cols[2],cols[3]);
				
				if(isdigit(*cols[1]) && (strcmp(cols[2],"page")==0))
				{
					// liq.[date].page.[keyword]  // ancient
					strncpy(self->filedate, cols[1],15); self->filedate[15]=  0;
					//strncpy(self->fileuser, cols[ ],15);self->fileuser[15]=0;
					strncpy(self->fileclass,cols[2],20);self->fileclass[20]=0;
					strncpy(self->filetitle,cols[3],20);self->filetitle[20]=0;
					free(inorig);
					return 0;
				}
				
				if(isdigit(*cols[1]) && (strcmp(cols[3],"page")==0))
				{
					// liq.[date].[user].page  // new
					strncpy(self->filedate, cols[1],15); self->filedate[15]=  0;
					strncpy(self->fileuser, cols[2],15);self->fileuser[15]=0;
					strncpy(self->fileclass,cols[3],20);self->fileclass[20]=0;
					//strncpy(self->filetitle,cols[ ],20);self->filetitle[20]=0;
					free(inorig);
					return 0;
				}

				if(isdigit(*cols[2]) && (strcmp(cols[3],"page")==0))
				{
					// liq.[user].[date].page  // ancient
					//liqapp_log("cough");
					strncpy(self->filedate, cols[2],15); self->filedate[15]=  0;
					strncpy(self->fileuser, cols[1],15);self->fileuser[15]=0;
					strncpy(self->fileclass,cols[3],20);self->fileclass[20]=0;
					//strncpy(self->filetitle,cols[ ],20);self->filetitle[20]=0;
					free(inorig);
					return 0;
				}


				// should never occur.. heh, final last words
				free(inorig);
				return -1;
			
			case 5:
				
				//liqapp_log("cols :: %i :: %s :: %s :: %s :: %s :: %s",colcount,cols[0],cols[1],cols[2],cols[3],cols[4]);
				
				if(isdigit(*cols[1]) && (strcmp(cols[3],"page")==0))
				{
					// liq.[date].[user].page.[keyword]    // new
					strncpy(self->filedate, cols[1],15); self->filedate[15]=  0;
					strncpy(self->fileuser, cols[2],15);self->fileuser[15]=0;
					strncpy(self->fileclass,cols[3],20);self->fileclass[20]=0;
					strncpy(self->filetitle,cols[4],20);self->filetitle[20]=0;
					free(inorig);
					return 0;
				}
				
				if(isdigit(*cols[2]) && (strcmp(cols[3],"page")==0))
				{
					// liq.[user].[date].page.[keyword]    // ancient
					strncpy(self->filedate, cols[2],15); self->filedate[15]=  0;
					strncpy(self->fileuser, cols[1],15);self->fileuser[15]=0;
					strncpy(self->fileclass,cols[3],20);self->fileclass[20]=0;
					strncpy(self->filetitle,cols[4],20);self->filetitle[20]=0;
					free(inorig);
					return 0;
				}

				// should never occur.. heh, final last words
				free(inorig);
				return -1;
		}
		
	
	free(inorig);	

	return 0;

}



int pagefilename_test(char *filename)
{
	struct pagefilename self;

	if(	pagefilename_breakapart(&self,filename) == 0)
	{
		// got it ok, lets confirm...

		liqapp_log("pagefilename: ok  path:'%s', dat:'%s', cls:'%s', tit:'%s'",self.filepath,self.filedate,self.fileclass,self.filetitle);
		return 1;
	}
	else
	{
		liqapp_log("pagefilename: bad '%s'",filename);
		return 0;
		// invalid
	}
	//return 0;
}

#ifdef __cplusplus
}
#endif

