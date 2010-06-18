



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
#include "liqcell_prop.h"




liqcell * liqcell_parse_filename(char *filename)
{
	// this is a non conflicting tree
	// each branch will have a unique name
	liqcell *self=NULL;
		struct stat     statbuf;
		if(stat(filename, &statbuf) == -1)
		{
			liqapp_log("liqcell_parse_filename stat failed: '%s'",filename);
			return self;
		}
		// got the information we need
		if ( S_ISDIR(statbuf.st_mode) )
		{
			//printf(" DIR  ");

			// create a cell indicating the folder

			self=liqcell_quickcreatenameclass(filename,"dir");
            liqcell_propseti(self,"filesize", statbuf.st_size);
            liqcell_propseti(self,"filecount",1);
			//liqcell_propsets(self,"filestamp",statbuf.st_datestamp);

			// now, recurse :)

			DIR           *	dir_p;
			struct dirent *	dir_entry_p;
			char 			fn[FILENAME_MAX+1];			
			dir_p = opendir(filename);			
			if(!dir_p)
			{
				liqapp_log("liqcell_parse_filename opendir failed: '%s'",filename);
				return self;			// heh thanks kot :)
			}				
			while( NULL != (dir_entry_p = readdir(dir_p)))
			{
				if( dir_entry_p->d_name[0]=='.' )
					continue;
				snprintf(fn , FILENAME_MAX , "%s/%s", filename , dir_entry_p->d_name);
				liqcell *c=liqcell_parse_filename(fn);
				if(c)
				{
					
					// make sure we account for the size :)
					liqcell_propseti(self,"filesize"  , liqcell_propgeti(self,"filesize",0)  + liqcell_propgeti(c,"filesize", 0) );
					liqcell_propseti(self,"filecount" , liqcell_propgeti(self,"filecount",0) + liqcell_propgeti(c,"filecount",1) );

					// its a folder.. it should have a range of datestamps within
					// we should do same for others, will add later					
					//if( strcmp( liqcell_propgets(self,"filedatemin",""), liqcell_propgets(c,"filedate","") ) < 0)
					//{
					//	liqcell_propsets(self,"filedatemin", liqcell_propgets(c,"filedate","") );
					//}
					//if( strcmp( liqcell_propgets(self,"filedatemax",""), liqcell_propgets(c,"filedate","") ) > 0)
					//{
					//	liqcell_propsets(self,"filedatemax", liqcell_propgets(c,"filedate","") );
					//}
					
					//liqcell_propsets(self,"filedatemin","statbuf.st_datestamp");
					//liqcell_propsets(self,"filedatemax","statbuf.st_datestamp");
					
					
					
					liqcell_child_insert(self,c);
					
					
					
					
				}
			}
			closedir(dir_p);
			


		}
		else
		if ( S_ISREG(statbuf.st_mode) )
		{
			//printf(" FILE  ");
			
			
			
			
			char *ext=liqapp_filename_walktoextension(filename);
			if(!ext || !*ext)
			{
				// nothing to see here..
                self=liqcell_quickcreatenameclass(filename,"file.unknown");
			}
			else
			if(
				strcasecmp(ext,"liqbrain")==0
			  )
			{
				// scan using the tree builder :)
				self=liqcell_quickcreatenameclass(filename,"file.liqbrain");
			}
			else			
			if(
				strcasecmp(ext,"sketch")==0
			  )
			{
				// scan as a sketch
				self=liqcell_quickcreatenameclass(filename,"file.sketch");
			}
			else
			if(
				strcasecmp(ext,"vfont")==0
			  )
			{
				// its a font
				self=liqcell_quickcreatenameclass(filename,"file.font");
			}
			else
			if(
				strcasecmp(ext,"jagernote")==0
			  )
			{
				// its a jager :) eye poppingly good introduction to real audio on this device
				// this is a recording made during a users session
				self=liqcell_quickcreatenameclass(filename,"file.jagernote");
			}
			else	
			if(
				   strcasecmp(ext,"jpeg")==0
				|| strcasecmp(ext,"jpg")==0
				|| strcasecmp(ext,"png")==0
			  )
			{
				// now we have an image
				// i should have a liqbrain of states and actions for use here
				self=liqcell_quickcreatenameclass(filename,"file.image");
			}
			
			else	
			if(
				   strcasecmp(ext,"txt")==0
				|| strcasecmp(ext,"rtf")==0
				|| strcasecmp(ext,"doc")==0
			  )
			{
				// now we have a document
				// i should have a liqbrain of states and actions for use here
				self=liqcell_quickcreatenameclass(filename,"file.document");
			}
			else
			if(
				   strcasecmp(ext,"mp3")==0
				|| strcasecmp(ext,"wma")==0
				|| strcasecmp(ext,"wav")==0
			  )
			{
				// now we have an audio file
				self=liqcell_quickcreatenameclass(filename,"file.sound");
			}
			else
			{
				self=liqcell_quickcreatenameclass(filename,"file.unknown");
				// we now deligate to the user driven filetypes
				// as specified within the current liqbrain
				//liqcell *fh = liqbrain_lookup("filehandlers");
			}
            
            
            
            if(self)
            {
                liqcell_propseti(self,"filesize", statbuf.st_size);
                liqcell_propseti(self,"filecount",1);
				//liqcell_propsets(self,"filedate",statbuf.st_datestamp);
            }
			
			
		}
	
		
	return self;
}

