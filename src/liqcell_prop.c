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
// 20090709_013938 lcuk : todo : big, replace this with lightweight prop token structure
// 20090709_013954 lcuk : something much more lightweight

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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Remove a string prop
 * @param self The liqcell to remove the prop from
 * @param name The prop to remove
 * @return int Success or Failure
 */
int liqcell_propremoves(liqcell *self, const char *name)
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

/**
 * Remove an integer prop
 * @param self The liqcell to remove the prop from
 * @param name The prop to remove
 * @return int Success or Failure
 */
int liqcell_propremovei(liqcell *self, const char *name)
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


/**
 * Remove a pointer prop
 * @param self The liqcell to remove the prop from
 * @param name The prop to remove
 * @return int Success or Failure
 */
int liqcell_propremovep(liqcell *self, const char *name)
{
	liqcell *c=liqcell_local_lookup_nameclass(self,name,"prop.p");
	if(c)
	{
		liqcell_child_remove(self,c);
		//liqcell_release(c);
		return 0;
	}
	return -1;
}


/**
 * Get an integer prop
 * @param self The liqcell to get the prop from
 * @param name The name of the prop to get
 * @param valueifnotfound Return this value if prop not found
 * @return int The integer prop or valueifnotfound
 */
int liqcell_propgeti(liqcell *self, const char *name,int valueifnotfound)
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
 * Get a pointer prop
 * @param self The liqcell to get the prop from
 * @param name The name of the prop to get
 * @param valueifnotfound Return this value if prop not found
 * @return int The integer prop or valueifnotfound
 */
void *liqcell_propgetp(liqcell *self, const char *name,void *valueifnotfound)
{
	liqcell *c=liqcell_local_lookup_nameclass(self,name,"prop.p");
	if(c)
	{
		return (void *)c->tag;
	}
	// i should now also check the ancestor
	// if i find a match there i should perhaps duplicate it here?
	return valueifnotfound;
}


/**
 * Create a child liqcell for the parent (self) with .tag defined as the
 * pointer value provided. This uses liqcell nameclass "prop.i".
 * 
 * @param self The parent liqcell
 * @param name The name of the prop
 * @param value The value to set the child's .tag to
 * @return pointer value
 * 
 */
void * liqcell_propsetp(liqcell *self, const char *name,void * value)
{
	liqcell *c=liqcell_child_lookup_nameclass(self,name,"prop.p");
	if(!c) c = liqcell_child_insert(self, liqcell_quickcreatenameclass(name,"prop.p") );
	if(c)
	{
		c->tag = (unsigned int)value;
	}
	else
	{
		
	}
	return value;
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
int liqcell_propseti(liqcell *self, const char *name,int value)
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

/**
 * Get a string prop
 * @param self The liqcell to get the prop from
 * @param name The name of the prop to get
 * @param valueifnotfound Return this value if prop not found
 * @return char* The string prop or valueifnotfound
 */
char *liqcell_propgets(liqcell *self, const char *name,char *valueifnotfound)
{
	liqcell *c=liqcell_local_lookup_nameclass(self,name,"prop.s");
	if(c)
	{
		return (char *)c->tag;
	}
	return valueifnotfound;
}

/**
 * Set a string prop
 * @param self The liqcell to set the prop for
 * @param name The name of the prop to set
 * @param value The value to set to prop "name"
 * @return char* The value provided
 */
char *liqcell_propsets(liqcell *self, const char *name,char *value)
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

	return value;
}

//############################################################# 
//############################################################# 
//############################################################# 

/** Helper function for liqcell_propsets_printf */
char *liqcell_propsets_vprintf(liqcell *self, const char *name, const char *format, va_list arg)
{
    //time_t     now;
    //struct tm  *ts;
    char       buf[1024];
	vsnprintf(buf,1023,format, arg);
	return liqcell_propsets(self,name,buf);
}

/**
 * Set formatted string prop and 
 * @param self The liqcell to set the prop for
 * @param name The name of the prop to set
 * @param format Format of the string prop
 * @param ... va_list args
 * @return char* The formatted string prop
 */
char *liqcell_propsets_printf(liqcell *self, const char *name, const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	char *res=liqcell_propsets_vprintf(self,name,format, arg);
	va_end(arg);
	return res;
}

#ifdef __cplusplus
}
#endif

