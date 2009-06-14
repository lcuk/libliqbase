/**
 * @file	liqcell_prop.c
 * @author  Gary Birkett
 * @brief	Manage a liqcell's .tag
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

#include "liqcell.h"

int liqcell_propremoves(liqcell *self,char *name)
{
	liqcell *c=liqcell_local_lookup_nameclass(self,name,"prop.s");
	if(c)
	{
		if(c->tag){ free((char *)c->tag); c->tag=0; }
		
		liqcell_child_remove(self,c);
		//liqcell_release(c);
		return 0;
	}
	return -1;
}

int liqcell_propremovei(liqcell *self,char *name)
{
	liqcell *c=liqcell_local_lookup_nameclass(self,name,"prop.i");
	if(c)
	{
		liqcell_child_remove(self,c);
		//liqcell_release(c);
		return 0;
	}
	return -1;
}


int     liqcell_propgeti(liqcell *self,char *name,int valueifnotfound)
{
	liqcell *c=liqcell_local_lookup_nameclass(self,name,"prop.i");
	if(c)
	{
		return (int)c->tag;
	}
	// i should now also check the ancestor
	// if i find a match there i should perhaps duplicate it here?
	return valueifnotfound;
}

/**
 * Create a child liqcell for the parent (self) with .tag defined as the
 * integer value provided. This uses liqcell nameclass "prop.i".
 * 
 * @param self The parent liqcell
 * @param name The name of the prop
 * @param value The value to set the child's .tag to
 * @return int value
 * 
 */
int     liqcell_propseti(liqcell *self,char *name,int value)
{
	liqcell *c=liqcell_child_lookup_nameclass(self,name,"prop.i");
	if(!c) c = liqcell_child_insert(self, liqcell_quickcreatenameclass(name,"prop.i") );
	if(c)
	{
		c->tag = value;
	}
	else
	{
		
	}
	return value;
}

char*   liqcell_propgets(liqcell *self,char *name,char *valueifnotfound)
{
	liqcell *c=liqcell_local_lookup_nameclass(self,name,"prop.s");
	if(c)
	{
		return (char *)c->tag;
	}
	return valueifnotfound;
}


char*   liqcell_propsets(liqcell *self,char *name,char *value)
{
	liqcell *c=liqcell_child_lookup_nameclass(self,name,"prop.s");
	if(!c) c = liqcell_child_insert(self, liqcell_quickcreatenameclass(name,"prop.s") );
	if(c)
	{
		if(c->tag) free((char *)c->tag);
		c->tag = (unsigned int)strdup(value);
		// todo fix this memory leak.  not sure yet how to handle this.  it should release the memory on closure or change.
		// there will be lots of these properties and efficient memory handling will be required.
		// maybe by storing instances and references into the token tree i can save some
		
		
	}
	else
	{
		
	}
	//
	return value;
}





//############################################################# 

char * liqcell_propsets_vprintf(liqcell *self,char *name,char *format, va_list arg)
{
    //time_t     now;
    //struct tm  *ts;
    char       buf[1024];
	vsnprintf(buf,1023,format, arg);
	return liqcell_propsets(self,name,buf);
}

char * liqcell_propsets_printf(liqcell *self,char *name,char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	char *res=liqcell_propsets_vprintf(self,name,format, arg);
	va_end(arg);
	return res;
}


