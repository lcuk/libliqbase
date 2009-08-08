// this file is part of liqbase by Gary Birkett
		
#include <liqbase/liqbase.h>
#include <liqbase/liqcell.h>
#include <liqbase/liqcell_prop.h>
#include <liqbase/liqcell_easyrun.h>
#include <liqbase/liqcell_easyhandler.h>
		
		
//#####################################################################
//#####################################################################
//##################################################################### liqmultitouch :: by gary birkett
//#####################################################################
//#####################################################################

// prepare the multitouch fields of the liqcellmouseeventargs

int liqcell_easyrun_mouseeventargs_multitouchprepare(liqcell *self, liqcellmouseeventargs *args,liqcell *context);



liqcell *liqmultitouch_f1=NULL;		// holder for standard default 1st finger
liqcell *liqmultitouch_f2=NULL;		// holder for the midpoint between 2 fingers
liqcell *liqmultitouch_f3=NULL;		// holder for the active 2nd finger, set to VISIBLE if in use


static void liqmultitouch_initrectfromliqpointstream(liqcell *self,liqpoint *mostrecentpoint,int maxcount,liqpoint *hardstopifencountered)
{
	liqpoint *p = mostrecentpoint;
	liqpoint *q=NULL;
	int tcnt=0;
	int tx=0;
	int ty=0;
	int tw=0;
	int th=0;
	while((p) && (p!=hardstopifencountered) && (maxcount>0))
	{
		tcnt++;
		tx+=p->x;
		ty+=p->y;
		if(q)
		{
			// can get bearings
			tw+=q->x-p->x;
			th+=q->y-p->y;
		}
		q=p;
		p=p->linkprev;
		maxcount--;
	}
	if(tcnt)
	{
		tx/=tcnt;
		ty/=tcnt;
		tw/=tcnt;
		th/=tcnt;
		
		//tw+=80;
		//th+=48;
		
		liqcell_setrect(self,  tx-tw/2, ty-th/2,   tw,th );
		//self->x=tx-tw/2;
		//self->y=ty-th/2;
		//self->w=tw;
		//self->h=th;
		//liqtile_update_boundfrompos(self);
		//self->visible=1;
		
		if(liqcell_getw(self)>10 || liqcell_geth(self)>10)
		{
			liqcell_setvisible(self,0);
		}
		else
		{
			liqcell_adjustpos(self,-40,-24);
			liqcell_adjustsize(self,80,48);
			liqcell_setvisible(self,1);
		}
	}
	else
	{
		//self->visible=0;
		liqcell_setvisible(self,0);
	}
}
#define samplecount 10



/**	
 * liqcell_easyrun_mouseeventargs_multitouchprepare prepare the multitouch fields for use :)
 */	
int liqcell_easyrun_mouseeventargs_multitouchprepare(liqcell *self, liqcellmouseeventargs *args,liqcell *context)
{
		liqcell *par = self;//liqcell_getlinkparent(self);
		liqcell *body = self;
		//liqapp_log("# liqcell kinetic body xy(%i,%i) wh(%i,%i) :: %s   ",body->x,body->y,  body->w,body->h,  body->name);
		//liqapp_log("# liqcell kinetic par  xy(%i,%i) wh(%i,%i) :: %s   ",par->x,par->y,  par->w,par->h,  par->name);
			
					
			//	if((args->msy-args->oy) >= (par->h*0.8))
				{
					// 20090724_034021 lcuk :  i'd like to try something here..
					// 20090724_034233 lcuk : lets assume that the user has pressed for a time in the lower quadrant of a list
					// 20090724_034253 lcuk : which is why we are here now
					
					if(!liqmultitouch_f1) liqmultitouch_f1 = liqcell_quickcreatevis("f1",NULL,0,0,0,0);
					if(!liqmultitouch_f2) liqmultitouch_f2 = liqcell_quickcreatevis("f2",NULL,0,0,0,0);
					if(!liqmultitouch_f3) liqmultitouch_f3 = liqcell_quickcreatevis("f3",NULL,0,0,0,0);

					//liqcell *userf1 = liqcell_child_lookup(self, "userf1");
					//liqcell *userf2 = liqcell_child_lookup(self, "userf2");
					//liqcell *userf3 = liqcell_child_lookup(self, "userf3");
					liqcell *userf1 = liqmultitouch_f1;
					liqcell *userf2 = liqmultitouch_f2;
					liqcell *userf3 = liqmultitouch_f3;
					if(userf1 && userf2 && userf3)
					{
						if( (args->mcnt == 1) )
						{
							// starting, so lets make sure these are not visible..
										liqcell_setvisible(userf1,0);
										liqcell_setvisible(userf2,0);
										liqcell_setvisible(userf3,0);
										
										liqcell_settag(userf1, NULL );
										liqcell_settag(userf2, NULL );
										liqcell_settag(userf3, NULL );

										liqmultitouch_initrectfromliqpointstream(userf1 ,args->stroke->pointlast,         1,NULL);
										liqmultitouch_initrectfromliqpointstream(userf2,NULL,                             1,NULL);
										liqmultitouch_initrectfromliqpointstream(userf3,NULL,                             1,NULL);


						}
						else
						{
							
							float ml = sqrt((float)(args->mdx*args->mdx+args->mdy*args->mdy));
							
							if( (args->mez > 0) )
							{
								// continuing
								if(liqcell_gettag(userf1)==NULL)
								{
									// we are in normal single finger mode
									// depending on how far this step takes us we might split into a multitouch
									if(ml>35)
									{
										// bsg.hybrid.jump();
										
										// set the starting point, so we know where to jump from
										liqcell_settag(userf1, args->stroke->pointlast->linkprev );
										liqmultitouch_initrectfromliqpointstream(userf1 ,args->stroke->pointlast->linkprev,         1,NULL);
										
										// now, setup for userf2
	
										liqmultitouch_initrectfromliqpointstream(userf2 ,args->stroke->pointlast,         samplecount,liqcell_gettag(userf1));
										// make sure we double the offset 									
										//liqcell_adjustpos(userf2, (liqcell_getcx(userf2)-liqcell_getcx(userf1))/2, (liqcell_getcy(userf2)-liqcell_getcy(userf1))/2  );
										
										
										liqcell_setrect( userf3,    liqcell_getx(userf2)+((liqcell_getcx(userf2)-liqcell_getcx(userf1))),
																	liqcell_gety(userf2)+((liqcell_getcy(userf2)-liqcell_getcy(userf1))),
																	liqcell_getw(userf2),
																	liqcell_geth(userf2)
																	);
										liqcell_setvisible(userf3,liqcell_getvisible(userf2));

									}
									else
									{
										// carry on being single touch..

										liqmultitouch_initrectfromliqpointstream(userf1 ,args->stroke->pointlast,         1,NULL);
										liqmultitouch_initrectfromliqpointstream(userf2,NULL,                             1,NULL);
										liqcell_setrect( userf3,    liqcell_getx(userf2)+((liqcell_getcx(userf2)-liqcell_getcx(userf1))),
																	liqcell_gety(userf2)+((liqcell_getcy(userf2)-liqcell_getcy(userf1))),
																	liqcell_getw(userf2),
																	liqcell_geth(userf2)
																	);
										liqcell_setvisible(userf3,liqcell_getvisible(userf2));

									}
								}
								else
								{									
									// we already have a multitouch in progress
									
									// do not alter userf1 now, it is fixed and locked
	
	
										liqmultitouch_initrectfromliqpointstream(userf2 ,args->stroke->pointlast,         samplecount,liqcell_gettag(userf1));
										
										// make sure we double the offset 														
										//liqcell_adjustpos(userf2, (liqcell_getcx(userf2)-liqcell_getcx(userf1))/2, (liqcell_getcy(userf2)-liqcell_getcy(userf1))/2  );
										liqcell_setrect( userf3,    liqcell_getx(userf2)+((liqcell_getcx(userf2)-liqcell_getcx(userf1))),
																	liqcell_gety(userf2)+((liqcell_getcy(userf2)-liqcell_getcy(userf1))),
																	liqcell_getw(userf2),
																	liqcell_geth(userf2)
																	);
										liqcell_setvisible(userf3,liqcell_getvisible(userf2));
									
								}
							}
							else
							{
								// finishing
								// lets make sure we tell our source that we completed

										liqcell_settag(userf1, NULL );
										liqcell_settag(userf2, NULL );
										liqcell_settag(userf3, NULL );

										liqcell_setvisible(userf1,0);
										liqcell_setvisible(userf2,0);
										liqcell_setvisible(userf3,0);
							}
						}
					}
					args->multiok = liqcell_getvisible(userf3);
					args->multisx = liqcell_getcx(userf1);
					args->multisy = liqcell_getcy(userf1);
					args->multix = liqcell_getcx(userf3);
					args->multiy = liqcell_getcy(userf3);
					args->multiw = liqcell_getw(userf3);
					args->multih = liqcell_geth(userf3);
					

				}
				

			
	return 0;
}
		
