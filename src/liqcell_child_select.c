/**
 * @file	liqcell_child_select.c
 * @author  Gary Birkett
 * @brief	This arrange module contains functions selecting/deslecting child members 
 * 			of visual cells
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
#include "liqcell_prop.h"

#ifdef __cplusplus
extern "C" {
#endif

void liqcell_child_selectall(liqcell *self)
{
	liqcell *c;
	c=liqcell_getlinkchild_visual(self);
	while(c)
	{
		if( !liqcell_getselected(c) ) liqcell_setselected(c,1);
		c=liqcell_getlinknext_visual(c);
	}
}

void liqcell_child_selectnone(liqcell *self)
{
	liqcell *c;
	c=liqcell_getlinkchild_visual(self);
	while(c)
	{
		if( liqcell_getselected(c) ) liqcell_setselected(c,0);
		c=liqcell_getlinknext_visual(c);
	}
}

void liqcell_child_selectinv(liqcell *self)
{
	liqcell *c;
	c=liqcell_getlinkchild_visual(self);
	while(c)
	{
		if( liqcell_getselected(c) )
			liqcell_setselected(c,0);
		else
			liqcell_setselected(c,1);
		c=liqcell_getlinknext_visual(c);
	}
}

void liqcell_child_selectfirst(liqcell *self)
{
	liqcell_child_selectnone(self);
	liqcell *c;
	c=liqcell_getlinkchild_visual(self);
	if(c)
	{
		liqcell_setselected(c,1);
		//c=liqcell_getlinknext_visual(c);
	}
}
#ifdef __cplusplus
}
#endif

