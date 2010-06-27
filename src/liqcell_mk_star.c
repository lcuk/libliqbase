/**
 * @file	liqcell_mk_star.c
 * @author  Gary Birkett
 * @brief 	small cell building helper functions
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


		

#include "liqcell.h"
#include "liqcell_easyrun.h"
#include "liqcell_prop.h"

#include "liqcell_mk_star.h"

#ifdef __cplusplus
extern "C" {
#endif

//###############################################################################
//############################################################################### meta_*
//###############################################################################


/**
 * construct a meta_info group based on ...
 *
 */

liqcell *mkmeta_group(liqcell *first,...)
{
	va_list arg;
	va_start(arg, first);
	liqcell *self= mkgroupa("meta","meta",first,arg);
	//self->visible=0;
	va_end(arg);
	return self;
}


	  



/**
 * construct a meta_title element
 *
 */

liqcell *mkmeta_title(char *key)
{
	liqcell *self= liqcell_quickcreatedata("meta_title","prop.s",key);
	return self;
}

/**
 * construct a meta_description element
 *
 */

liqcell *mkmeta_author(char *key)
{
	liqcell *self= liqcell_quickcreatedata("meta_author","prop.s",key);
	return self;
}

/**
 * construct a meta_description element
 *
 */
liqcell *mkmeta_description(char *key)
{
	liqcell *self= liqcell_quickcreatedata("meta_description","prop.s",key);
	return self;
}

/**
 * construct a meta_version element
 *
 */
liqcell *mkmeta_version(char *key)
{
	liqcell *self= liqcell_quickcreatedata("meta_version","prop.s",key);
	return self;
}



//###############################################################################
//############################################################################### Group base
//###############################################################################

/**
 * construct a group of non visual cells based on a va_list
 *
 */

liqcell *mkgroupa(char *key,char *classname,liqcell *first,va_list arg)
{
	//app_log("grp %s start",key);
	liqcell *self= liqcell_quickcreatenameclass(key,classname);
	//self->visible=1;
	
	// add some special sauce ;)
	//liqcell_childappend(self,mkhot("hot"));
	
	if(first)
	{
		//app_log("grp %s appending first",key);
		liqcell_child_append(self,first);
		//va_list arg;
		//va_start(arg, first);
		while(1)
		{
			liqcell *c = va_arg(arg, liqcell *);
			if(!c)break;
			//app_log("grp %s appending n",key);
			liqcell_child_append(self,c);
		};
		//va_end(arg);
	}
	return self;
}


/**
 * construct a group of cells based on ...
 *
 */

liqcell *mkgroup(char *key,liqcell *first,...)
{
	va_list arg;
	va_start(arg, first);
	liqcell *self= mkgroupa(key,"group",first,arg);
	//self->layoutmode=1;
	va_end(arg);
	return self;
}

#ifdef __cplusplus
}
#endif

