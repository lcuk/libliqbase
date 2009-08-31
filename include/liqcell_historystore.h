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
 * Header for the cliprect library.  this is the prefered way to interact with graphics.
 *
 */



#ifndef LIQCELL_HISTORYSTORE_H
#define LIQCELL_HISTORYSTORE_H


#include "liqbase.h"
#include "liqcell.h"

int liqcell_historystore_historythumb_getfilename(char *buffer,int buffersize,char *classname);
int liqcell_historystore_historythumb(liqcell *self);



#endif

