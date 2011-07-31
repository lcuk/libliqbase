
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include "liqcell.h"
#include "liqcell_easyrun.h"
#include "liqcell_easyhandler.h"
#include "liqbase.h"


#ifdef __cplusplus
extern "C" {
#endif

	int 	liqcell_easyhandler_content_zoom_click(liqcell *self, liqcellclickeventargs *args,liqcell *context)
    {
        liqapp_log("liqcell_easyhandler_content_zoom_click '%s'",self->name);

		args->newdialogtoopen =liqcell_hold(  liqcell_getcontent( self ) );
        return 1;
       
    }


//#####################################################################
//#####################################################################
//##################################################################### kinetic mouse handler :: by gary birkett
//#####################################################################
//#####################################################################


// assign this as the mouse handler for your object and it will perform kinetic bursts :)




	int 	liqcell_easyhandler_kinetic_mouse(liqcell *self, liqcellmouseeventargs *args,liqcell *context)
	{

		liqcell *par = liqcell_getlinkparent(self);
		liqcell *body = self;
		//liqapp_log("# liqcell kinetic mous xy(%i,%i) wh(%i,%i)   ",args->mex,args->mey,args->mex-args->msx,args->mey-args->msy);


		
		//liqapp_log("# liqcell kinetic body xy(%i,%i) wh(%i,%i) :: %s   ",body->x,body->y,  body->w,body->h,  body->name);
		//liqapp_log("# liqcell kinetic par  xy(%i,%i) wh(%i,%i) :: %s   ",par->x,par->y,  par->w,par->h,  par->name);
		if(body && par && par->h)
		{
			if( (args->mcnt == 1) )
			{
				// make sure we stop kinetic scrolling when we get a mouse event
				liqcell_setkinetic(body, 0,0 );
			}
			// 20090514_004718 lcuk : do i add an auto scrollbar mechanism onto this object?
			// 20090514_004749 lcuk : only visible whilst actually moving
			// 20090514_004904 lcuk : and it also accounts for automatic near invisible "normal" scrolling by default
			/*liqcell *knob = liqcell_child_lookupnameclass(body,"kinetic_knob","kinetic_knob");
			if(!knob)
			{
				knob = liqcell_quickcreatevis("kinetic_knob","kinetic_knob",  par->w-10,0,10,par->h );
				liqcell_propsets( knob, "backcolor", "rgb(30,90,30)" );
				liqcell_setvisible( knob, 0 );
				// 20090514_010118 lcuk : problem though is ensuring its removed again after a mouse action completes
				// 20090514_010136 lcuk : 
			}*/
			
			//#####################################
			// 20090614_181606 lcuk : adding "direct addressing mode" which is the scrollbar at right hand side mechanism
			// entering this mode if the mouse if on the right hand side 20% of parent
			// for testing leave as direct addressing of Y
			
			// 20100110_150000 lcuk : removed for now..
			if(0 &&  (args->stroke->pointlast->t -  args->stroke->pointfirst->t) > 250 )  //liqstroke_totallength(args->stroke) > 25 )
			{
				
				if( liqstroke_totallength(args->stroke) > 20 )

				
				if(   (args->msx-args->ox) >= (par->w*0.8) )
				{			
					if(   (args->mex-args->ox) >= (par->w*0.8) )
					{
						//float my = args->mey;
						float my = args->mey;//-args->oy;
						int ah = (self->h-par->h);
						// self->y    p->0    p->h       self->h-self->y
						// -300       0       480        1700
						
						float mj = my * ((float)ah) / ((float)par->h);
						if(mj<0)mj=0;
						if(mj>ah)mj=ah;
						
						liqcell_setpos(body,self->x,-mj);
						return 1;
					}
				}
				
					
				if((args->msy-args->oy) >= (par->h*0.8))
				{
					// 20090724_034021 lcuk :  i'd like to try something here..
					// 20090724_034233 lcuk : lets assume that the user has pressed for a time in the lower quadrant of a list
					// 20090724_034253 lcuk : which is why we are here now
					// 20090726_011904 lcuk : this test will simply use "multitouch_test_range" on the PARENT as an indicator of how many available columns the MT will fall into
					// 20090726_011929 lcuk : depending on which column, the list will be arranged using extrapolated dimensions
					// 20090726_012034 lcuk : this is very "testing" phase, and will eventually have an associated event to deal with MT
					int multitouch_range = liqcell_propgeti(par,"multitouch_test_range",0);
					if(args->multiok && multitouch_range>0)
					{
						// we are in a multitouch block!
						// using the HORIZONTAL position, change the scale of the items in the list..
						
						float sectorx = (float)args->multix / (float)par->w;
						
						if(sectorx<0)sectorx=0;
						if(sectorx>1)sectorx=1;
						
						int gridx = 1+(sectorx * 5);
						
						
						
						
						
						liqcell *c=liqcell_getlinkchild_visual(self);
						while(c)
						{
							// liqcell_setsize(c,args->multix,c->h);
							liqcell_setsize(c,par->w / gridx,par->h/gridx);
							
							c=liqcell_getlinknext_visual(c);
						}
						liqcell_setsize(self,par->w,self->h);
						liqcell_child_arrange_autoflow(self);
						liqcell_setrect(self,0,0,par->w,self->h);
						//liqcell_setpos(self,0,0);
						return 1;
					}

				}

			}
			//#####################################
			
			
			
			
			
			
			


			int mdx=0;
			int mdy=0;
			if(body->w>par->w || body->x!=0)mdx=args->mdx;
			if(body->h>par->h || body->y!=0)mdy=args->mdy;

			liqcell_adjustpos(body,mdx,mdy);
			if(body->w>par->w || body->x!=0)
			{
				//if(body->x>0) body->x=0;
				if(body->x>0) liqcell_setpos(body,0,body->y);
				int bb=body->x+body->w;

				//if(body->w>par->w && bb < par->w) body->x=par->w-body->w;
				if(body->w>par->w && bb < par->w) liqcell_setpos(body,par->w-body->w,body->y);

			}
			
			if(body->h>par->h || body->y!=0)
			{
				//if(body->y>0) body->y=0;
				if(body->y>0) liqcell_setpos( body, body->x,0);
				int bb=body->y+body->h;

				//if(body->h>par->h && bb < par->h) body->y=par->h-body->h;
				if(body->h>par->h && bb < par->h) liqcell_setpos(body,body->x,par->h-body->h);
			}

			//liqapp_log("kinetic mouse d %i,%i    ez=%i    %i,%i",args->mdx,args->mdy,args->mez    ,args->stroke->pointlast->x,args->stroke->pointlast->y  );



			if( (args->mez == 0) )
			{
				// x11 lets me know motion in a different event to mouseup
				// so i always have a stroke that finishes moving and then indicates release
				// so i have to examine the stroke from the end
				// if the last point and its neighbour are identical i can try one further back
				liqstroke *stroke = args->stroke;
				liqpoint *p1=NULL;
				liqpoint *p2=NULL;
				
						p1 = stroke->pointlast;
				if(p1)	p2 = p1->linkprev;
				
				
				if(p1 && p2 && p1->x==p2->x && p1->y==p2->y)
				{
					// now replease p2 with the previous one again
						p2 = p2->linkprev;
				}
				if(p1 && p2 && p1->x==p2->x && p1->y==p2->y)
				{
					// now replease p2 with the previous one again
						p2 = p2->linkprev;
				}
				
				if(p1 && p2)
				{
					// now finally obtain the delta
					mdx=0;
					mdy=0;
					if(body->w>par->w || body->x!=0) mdx = p1->x-p2->x;
					if(body->h>par->h || body->y!=0) mdy = p1->y-p2->y;
					liqcell_setkinetic(body, mdx, mdy );
				}
			}

		}


		return 1;
	}

#ifdef __cplusplus
}
#endif

