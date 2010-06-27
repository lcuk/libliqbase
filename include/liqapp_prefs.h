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
 * Header for app preferences
 *
 */




#ifndef LIQAPP_PREFS_H
#define LIQAPP_PREFS_H

#ifdef __cplusplus
extern "C" {
#endif
//#include <stdio.h>
//#include <dirent.h>



#include "liqapp.h"
#include "liqcell.h"


int 		liqapp_prefs_load();
char * 		liqapp_pref_getvalue(const char *prefkey);
const char *      liqapp_pref_getvalue_def(const char *prefkey, const char *defaultifmissing);
liqcell *	liqapp_pref_getitem(const char *prefkey);
int 		liqapp_pref_checkexists(const char *prefkey);
const char * 		liqapp_pref_setvalue(const char *prefkey, const char *prefvalue);
const char * 		liqapp_pref_setvalue_printf(const char *prefkey, const char *prefformat, ...);
int 		liqapp_prefs_save();

#ifdef __cplusplus
}
#endif

#endif

