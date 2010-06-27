#include <time.h>			// req for sleep
#include <sys/time.h>		// req for getticks

//#include "vscreen.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>
#include <sched.h>

#include "liqcell.h"
#include "liqcell_easyrun.h"
#include "liqcell_easyhandler.h"
#include "liqcell_mk_star.h"

#ifdef __cplusplus
extern "C" {
#endif

//#####################################################################
//#####################################################################
//##################################################################### liqtimer :: by gary birkett
//#####################################################################
//#####################################################################




static int thread_createwithpriority(pthread_t *tid,int threadpriority,void *(*func)(void *),void *arg)
{
//pthread_t 		tid;
pthread_attr_t 	tattr;
struct sched_param 	param;
int ret;
int newprio = threadpriority;//20;


	// initialized with default attributes
	ret = pthread_attr_init(&tattr);
	// safe to get existing scheduling param
	ret = pthread_attr_getschedparam(&tattr, &param);
	// set the priority; others are unchanged

	//liqapp_log("thread schedparam=%i (current)",param.sched_priority);

	param.sched_priority = newprio;
	// setting the new scheduling param
	ret = pthread_attr_setschedparam(&tattr, &param);
	// with new priority specified

	ret = pthread_create(tid, &tattr, func, arg);


//	ret = pthread_create(tid, NULL, func, arg);
	return ret;
}

static void *liqtimer_workthread(void* workthread_data)
{

	liqcell *self = (liqcell *)workthread_data;

	//liqapp_sleep(100 + (rand() % 4000));
	//liqapp_sleep(100 + (rand() % 2000));
	//liqapp_sleep(10 + (rand() % 100));
	
	//liqcell_setdirty(self);
	
	
	
	while(1)
	{
        liqcell_hold(self);
		//liqapp_log("looping..");
		if(liqcell_getenabled(self))
		{
			int dt =  liqcell_propgeti(self,"timerinterval",0);
			if(dt<=0)dt=1;
            liqcell_release(self);
            
			liqapp_sleep(dt);
            
			//liqapp_log("ticking..");
            liqcell_hold(self);
			liqcell_handlerrun(self,"timertick",NULL);
            liqcell_release(self);
		}
		else
		{
            liqcell_release(self);
            
			liqapp_sleep(50);
			//liqcell_handlerrun(keyhit,"timertick",NULL);			
		}
	}
	
	return NULL;
}




liqcell *liqtimer_create()
{
	liqcell *self = liqcell_quickcreatewidget("liqtimer","liqtimer", 0,0);

	if(self)
	{
		
		// todo: should make sure we break out of duldrums if the user enables or disables us
		liqcell_setvisible(self,0);	// not visible in the render window ever
		liqcell_setenabled(self,0);
		liqcell_propseti(self,"timerinterval",100);




				pthread_t 		*tid = (pthread_t *)malloc(sizeof(pthread_t));
				if(!tid)
				{
					liqapp_log("liqtimer, cannot alloc thread");
					return 0;					
				}

				//pthread_t 		tid;
				int tres=thread_createwithpriority(tid,0,liqtimer_workthread,self);
				if(tres)
				{
					liqapp_log("liqtimer, thread create fail");
					free(tid);
					return 0;
				}
				liqcell_setdata(self,tid);
				
				
				//liqcell_propseti(self,"timerenabled",0);
	}

	return self;
}

#ifdef __cplusplus
}
#endif

