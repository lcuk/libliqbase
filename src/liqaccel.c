/**
 * @file	liqaccel.c
 * @author  Gary Birkett
 * @brief 	accel reader interface.  reads and returns smooth values  :)  best at high framerates
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




#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>            
#include <fcntl.h>                                                                             
#include <unistd.h>
#include <errno.h>

#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks
#include "liqapp.h"

#include "liqcell.h"

#include "liqapp_prefs.h"
#include "liqapp_hildon.h"

static int ocnt=0;
static int oax=0;
static int oay=0;
static int oaz=0;

static float oangle=0;
	
static const char *accel_filename = "/sys/class/i2c-adapter/i2c-3/3-001d/coord";

int liqaccel_read(int *ax,int *ay,int *az)
{
	FILE *fd;
	int rs;
	fd = fopen(accel_filename, "r");
	if(fd==NULL){ liqapp_log("liqaccel, cannot open for reading"); return -1;}	
	rs=fscanf((FILE*) fd,"%i %i %i",ax,ay,az);	
	fclose(fd);	
	if(rs != 3){ liqapp_log("liqaccel, cannot read information"); return -2;}
	
	// patch to allow smoothing to be configurable
	char *prefsmooth = liqapp_pref_getvalue_def("liqaccel_usesmoothing","yes");
	
	if( ocnt>0 && (prefsmooth && *prefsmooth == 'y') )
	{
		int bx=*ax;
		int by=*ay;
		int bz=*az;
		*ax=oax+(bx-oax)*0.1;
		*ay=oay+(by-oay)*0.1;
		*az=oaz+(bz-oaz)*0.1;
	}
	oax=*ax;
	oay=*ay;
	oaz=*az;
	ocnt++;
	oangle=0;
	return 0;
}

int liqaccel_inuse()
{
	return (ocnt!=0);
}

void liqaccel_reset()
{
	// turn ourselves off
	// any new user will instantly switch it back on again
	ocnt=0;
}
float liqaccel_getangle()
{
	// try not to calc this too often
	// just reread when exactly 0
	// 
	if(ocnt==0)return 0;
	if(oangle==0)
	{
		float faax = oax;
		float faay = oay;
		oangle=atan2(faax,-faay );
	}
	return oangle;
	
}
