/**
 * @file	liqcell_historystore.c
 * @author  Gary Birkett
 * @brief 	store away a thumbnail history item
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>


#include "liqbase.h"

#include "liqcell.h"
#include "liqcell_prop.h"
#include "liqcell_easyrun.h"
#include "liqcell_easypaint.h"
#include "vgraph.h"

#ifdef __cplusplus
extern "C" {
#endif


			
int liqcell_historystore_historythumb_getfilename(char *buffer,int buffersize,char *classname)
{
    *buffer=0;
    if(!classname)return -1;
    if(!*classname)return -1;
    snprintf(buffer,buffersize,"%s/historythumb/liq.%s.historythumb.png",app.userdatapath,classname);
	return 0;
	
}



			
int liqcell_historystore_historythumb(liqcell *self)
{
    if(!self)return -1;
    if(!self->classname)return -1;
    // check if the file exists?

        char buf[FILENAME_MAX]={0};
        
        liqcell_historystore_historythumb_getfilename(buf,sizeof(buf),self->classname);


	if(liqapp_fileexists(buf))
	{
		// ok, there is a file already existing, no point in regenerting right now
		// tho in future it would be better storing a live  thumb, its just a bit worrysome with my drive
		return 0;
	}
    
    liqcell_hold(self);
    
            
        
        liqapp_log("liqcell_historystore_historythumb creating image");
        liqimage *img = liqimage_newatsize(80,48,0);
        if(!img)
        {
            liqapp_log("liqcell_historystore_historythumb could not alloc image");
            liqcell_release(self);
            return -1;
        }
        
        liqapp_log("liqcell_historystore_historythumb creating cliprect");
        liqcliprect *cr = liqcliprect_newfromimage(img);
        if(!cr)
        {
            liqapp_log("liqcell_historystore_historythumb could not alloc cr");
            
            liqimage_release(img);
            liqcell_release(self);
            return -1;
        }
    
        
        liqapp_log("liqcell_historystore_historythumb clearing before cell %s",self->name);
        liqcliprect_drawclear(cr,0,128,128);
        
        liqapp_log("liqcell_historystore_historythumb painting cell %s",self->name);
        liqcell_easypaint(self,cr,0,0,80,48);
    
//        liqapp_log("liqcell_historystore_historythumb building filename class '%s'",self->classname);
    
		// already got        
     
        liqapp_log("liqcell_historystore_historythumb saving image as '%s'",buf);
    
        liqimage_pagesavepng(img,buf);
    
        
        liqapp_log("liqcell_historystore_historythumb releasing cr");
        liqcliprect_release(cr);
        
        liqapp_log("liqcell_historystore_historythumb releasing image");
        liqimage_release(img);
	
	liqapp_log("liqcell_historystore_historythumb done");
    
    liqcell_release(self);
	return 0;
	
}

#ifdef __cplusplus
}
#endif

