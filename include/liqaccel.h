

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
 * Header for accel reader interface.  reads and returns smooth values :)
 *
 */

#ifndef liqaccel_H
#define liqaccel_H

#ifdef __cplusplus
extern "C" {
#endif


int liqaccel_read(int *ax,int *ay,int *az);

float liqaccel_getangle();	// use most recently read coords to orient self

#ifdef __cplusplus
}
#endif


#endif

