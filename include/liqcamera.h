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
 * Header for camera helper functions
 *
 */




#ifndef LIQCAMERA_H
#define LIQCAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liqimage.h"


int liqcamera_start(int argCAMW,int argCAMH,int argCAMFPS,liqimage * argCAMdestimage,void (*argCAMUpdateCallback)(),void *argCAMtag );
void liqcamera_stop();

liqimage * liqcamera_getimage(); // return the current image of this camera, if NULL camera is switched off



int liqcamera_settorch(int newvalue);
int liqcamera_setfocus(int newvalue);

#ifdef __cplusplus
}
#endif

#endif





