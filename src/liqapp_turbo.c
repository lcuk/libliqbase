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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

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

//######################################################## cpu


int 		cpufreq_governor_read(char *result,int resultmaxlength);
int 		cpufreq_governor_write(char *newgovernor);
int 		cpufreq_governor_changeto(char *newgovernor);





static char turbo_orig[255]="\0";	// original governor
static int turbo_inuse=0;


void liqapp_turbo_start()
{
	turbo_inuse=0;
	int ri;
	liqapp_log("liqapp turbo: activating bacon");
	ri=cpufreq_governor_read(turbo_orig, 255);
	if(ri!=0){liqapp_warnandcontinue(-1,"liqapp turbo: cannot open governor for reading"); return;}
	liqapp_log("liqapp turbo: current = '%s'",turbo_orig);
	if(strcmp("performance",turbo_orig))
	{
		liqapp_log("liqapp turbo: upgrading to 'performance' now");
		ri=cpufreq_governor_write("performance");
		if(ri<0){liqapp_warnandcontinue(-1,"liqapp turbo: cannot open governor for writing"); return;}
		liqapp_log("liqapp turbo: performance mode activated, enjoy your breakfast");
		turbo_inuse=1;
	}
	else
	{
		liqapp_log("liqapp turbo: No action required yet, we are already 'performance'");
		
	}
}





void liqapp_turbo_reset()
{
	int ri;
	if(turbo_inuse)
	{
		liqapp_log("liqapp turbo: stepping back down from 'performance' to '%s'",turbo_orig);
		ri=cpufreq_governor_write(turbo_orig);
		if(ri!=0){liqapp_log("ERROR: turbo: cannot restore governor information, we are stuck at 'performance' until full reboot"); return;}
		liqapp_log("liqapp turbo: done");
		turbo_inuse=0;
	}
}




//#########################################################
//lcuk: cpufreq functions for reading/writing to the governor
//#########################################################

static const char *cpufreq_governor_filename = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
int cpufreq_governor_read(char *result,int resultmaxlength)
{
	FILE *fd;
	char *rc;
	fd = fopen(cpufreq_governor_filename, "r");
	if(fd==NULL){ liqapp_log("cpufreq, cannot open governor for reading\n"); return -1;}
	rc=fgets(result, resultmaxlength, (FILE*) fd);
	fclose(fd);
	if(rc==NULL){ liqapp_log("cpufreq, cannot read governor information\n"); return -2;}
	//lcuk: cleaning off any trailing \n
	char *b=result;	while(*b){  if(*b=='\n'){ *b=0;break; } b++; };
	return 0;
}



int cpufreq_governor_write(char *newgovernor)
{
	//lcuk: instead of directly writing this, I now use an alternative method
	char newcmd[FILENAME_MAX];

	snprintf(newcmd,FILENAME_MAX,"/usr/bin/liqbase-cpu-%s",newgovernor);
	
	if(-1==system(newcmd))
	{
		// the cpu change failed
		{ liqapp_log("cpufreq, write: cannot run cmd: '%s'\n",newcmd); return -1; }
	}
	
	//###################################################
    char buff_orig[80]="\0";	// re-read governor
	int ri;
	ri=cpufreq_governor_read(buff_orig,80);
	if(ri!=0){ liqapp_log("cpufreq, write: cannot read from governor\n"); return -1; }
	if(strcmp(newgovernor,buff_orig) == 0)
	{
        //lcuk: the setting was applied
		return 0;
	}
	// the setting is different
	{ liqapp_log("cpufreq, cannot write to governor '%s', %s, %s\n",newcmd,newgovernor,buff_orig); return -2; }

}


int cpufreq_governor_changeto(char *newgovernor)
{
    char buff_orig[80]="\0";	// original governor
	int ri;
	ri=cpufreq_governor_read(buff_orig,80);
	if(ri!=0){ liqapp_log("cpufreq, cannot read from governor\n"); return -1; }
	if(strcmp(newgovernor,buff_orig) == 0)
	{
        //lcuk: nothing to do..
	    return 0;
	}
    ri=cpufreq_governor_write(newgovernor);
    if(ri!=0){ liqapp_log("cpufreq, cannot change the governor\n"); return -2; }
    return 0;
}

