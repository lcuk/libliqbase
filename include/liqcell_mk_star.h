#ifndef liqcell_MK_STAR_H
#define liqcell_MK_STAR_H 1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file	liqcell_mk_star.h
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


//###############################################################################
//############################################################################### meta
//###############################################################################

/**
 * construct a meta_info group based on ...
 *
 */

liqcell *mkmeta_group(liqcell *first,...);

/**
 * construct a meta_title element
 *
 */

liqcell *mkmeta_title(char *key);
/**
 * construct a meta_description element
 *
 */

liqcell *mkmeta_author(char *key);
/**
 * construct a meta_description element
 *
 */
liqcell *mkmeta_description(char *key);

/**
 * construct a meta_version element
 *
 */
liqcell *mkmeta_version(char *key);




/*
	typical usage of meta areas:
	
	we can add more as required.
	
	
		liqcell *meta = mkmeta_group(
							mkmeta_title(        "liqbase"),
							mkmeta_description(  "this is a test description"),
							mkmeta_author(       "liquid@gmail.com"),
							mkmeta_version(      "1.0"),
							NULL);
		liqcell_child_append(self,meta);
	
 */



		
//###############################################################################
//############################################################################### Group
//###############################################################################

/**
 * construct a group of non visual cells based on a va_list
 *
 */

liqcell *mkgroupa(char *key,char *classname,liqcell *first,va_list arg);


/**
 * construct a group of cells based on ...
 *
 */

liqcell *mkgroup(char *key,liqcell *first,...);


/**
 * construct a meta_info group based on ...
 *
 */

#ifdef __cplusplus
}
#endif

#endif
