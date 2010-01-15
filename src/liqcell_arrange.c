/**
 * @file	liqcell_arrange.c
 * @author  Gary Birkett
 * @brief	This arrange module contains functions for arranging clusters 
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


#define ABS(X) ((X)<0 ? -(X) : (X))
#define SGN(X) ((X)<0 ? -(1) : (1))


//##################################################################
//################################################################## dimension base
//##################################################################
// dimension isnt really a class, its just a span of Start..End and the handling of it
//#####################################################################
static inline void dimension_forceinbound(register int *s,register int *m,register int l,register int r)
{

	register int e = *s+*m;

	register int d=*m;//*e-*s;
	register int w=r-l;
	register int isneg=0;
	if(d<0  ){register int t=*s;*s=e;e=t;d=-d;isneg=1;}
	if(d>w  ){e-=d-w;        }
	if(*s<l ){e+=l-*s;*s=l;  }
	if(e>=r){*s-=e-(r);e=r;  }
	if(isneg){register int t=*s;*s=e;e=t;d=-d;}
	*m=e-*s;

/*
	register int d=*e-*s;
	register int w=r-l;
	register int isneg=0;
	if(d<0  ){register int t=*s;*s=*e;*e=t;d=-d;isneg=1;}
	if(d>w  ){*e-=d-w;        }
	if(*s<l ){*e+=l-*s;*s=l;  }
	if(*e>=r){*s-=*e-(r);*e=r;  }
	if(isneg){register int t=*s;*s=*e;*e=t;d=-d;}
*/
}

//#####################################################################
static inline int dimension_gapcalc(register int cs,register int ce,register int ds,register int de,int *gapres)
{
	// single dimension quick gap calc
	if(ce < ds)  {*gapres = (ds-ce)/2; return -2; }		//  CCC    DDDDDD
	if(ce == ds) {*gapres = 0;        return -1; }		//  CCCDDDDDD
	if(de < cs)  {*gapres = (cs-de)/2; return 2;  }		//  DDDDDD    CCC
	if(de == cs) {*gapres = 0;        return 1;  }		//  DDDDDDCCC
	// they are overlapping
	*gapres=0;
	return 0;
}

//#####################################################################
static inline int dimension_overlapcalc(register int cs,register int ce,register int ds,register int de,int *overlapres)
{
	// single dimension quick overlap calc
	if(ce < ds) return -2;		//  CCC    DDDDDD
	if(ce == ds) return -1;		//  CCCDDDDDD
	if(de < cs) return 2;		//  DDDDDD    CCC
	if(de == cs) return 1;		//  DDDDDDCCC
	if(cs <= ds)
	{
		// C starts before D starts
		if( ce <= de )
		{
			// a partial overlap exists
			*overlapres = (ds-ce);
			return 0;
		}
		// D is smaller than us
		// we are wholey engulfing D
		// GET IT OUT OF ME!
		// we take the shortest route out.
		if( (ce-de) < (ds-cs) )
		{
			// D is closer to the right hand side of C
			*overlapres = (ds-ce);
			return 0;
		}
		else
		{
			// D is closer to the left hand side of C
			// this should be negative
			*overlapres = (cs-de);
			return 0;
		}
	}
	else
	{
		// D starts before C starts
		if( de <= ce )
		{
			// a partial overlap exists
			*overlapres = -(cs-de);
			return 0;
		}
		// D is smaller than us
		// we are wholey engulfing D
		// GET IT OUT OF ME!
		// we take the shortest route out.
		if( (de-ce) < (cs-ds) )
		{
			// D is closer to the right hand side of C
			*overlapres = -(cs-de);
			return 0;
		}
		else
		{
			// D is closer to the left hand side of C
			// this should be negative
			*overlapres = -(ds-ce);
			return 0;
		}
	}
	return 0;
}

//##################################################################
//##################################################################
//##################################################################
//##################################################################
//##################################################################


void liqcell_forceinboundparent(liqcell *self)
{
	liqcell *par=self->linkparent;
	if(par)
	{
		//dimension_forceinbound(&self->sx,&self->ex,par->sx,par->ex);
		//dimension_forceinbound(&self->sy,&self->ey,par->sy,par->ey);
		dimension_forceinbound(&self->x,&self->w,0,par->w);
		dimension_forceinbound(&self->y,&self->h,0,par->h);
	}
}




int liqcell_child_arrange_autoflow(liqcell *self)			// flowing document text
{
	liqcell *c;

	int availw=liqcell_getw(self);
	//int availh=liqcell_geth(self);


	int x=0;
	int y=0;

	int allmaxw=0;
	int rowmaxh=0;

	//################################################
	//int answercount=0;

	//################################################
	//liqapp_log("liqcell_child_arrange_nooverlap preparing");
	c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{

			int w=liqcell_getw(c);
			int h=liqcell_geth(c);

			if(x+w>availw && x>0)
			{
				// cannot fit (we ignore the first so we get at least 1 on the flow)
				x=0;
				y+=rowmaxh;
				rowmaxh=0;
			}

			if(x+w>allmaxw) allmaxw = x+w;		// make sure we store the widest total width
			if(h>rowmaxh) rowmaxh = h;			//

			liqcell_setpos(c, x,y);
			x+=w;
		}
		c=liqcell_getlinknext(c);
	}

	liqcell_setsize(self, allmaxw, y+rowmaxh);

	return 0;

}



int liqcell_child_arrange_nooverlap(liqcell *self,liqcell *currentselection)
{

	// oo way o.O :)
	liqcell *c;

	//################################################
	int answercount=0;

	//################################################
	//liqapp_log("liqcell_child_arrange_nooverlap preparing");
	c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{
			c->overlapx=0;
			c->overlapy=0;
			answercount++;		//
		}
		c=liqcell_getlinknext(c);
	}

	if(answercount==0)
	{
		liqapp_log("liqcell_child_arrange_nooverlap nothing to weigh, leaving as is");
		return 0;
	}

	//################################################
	//liqapp_log("liqcell_child_arrange_nooverlap calculating");
	c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{


			liqcell *d = liqcell_getlinknext(c);
			while(d)
			{
				// find the overlap

				if(d->visible)
				{
					// we calculate the overlap between c&d
					int olw=0;
					int olh=0;
					if( dimension_overlapcalc(c->x,c->x+c->w,   d->x,d->x+d->w,   &olw) == 0
					&&	dimension_overlapcalc(c->y,c->y+c->h,   d->y,d->y+d->h,   &olh) == 0
					   )
					{
						if(olw && ABS(olw) < ABS(olh))
						{
							olh=0;
						}
						if(olh && ABS(olh) < ABS(olw))
						{
							olw=0;
						}

					//	if(olw==0) olw -= SGN(olh)*10;
					//	if(olh==0) olh -= SGN(olw)*10;


					//	olw*=1.5;
					//	olh*=1.5;
					/*
						float fracw = ( d->w / (c->w+d->w) ) ;
						float frach = ( d->h / (c->h+d->h) ) ;
						c->overlapx+=(float)olw*fracw;
						c->overlapy+=(float)olh*frach;
						d->overlapx-=(float)olw*(1-fracw);
						d->overlapy-=(float)olh*(1-frach);
					*/

						// we know what the force is going to be and in which direction :)
						//app_log("c %i, d %i",c->ident,d->ident);
						olw/=2;
						olh/=2;
						c->overlapx+=olw;
						c->overlapy+=olh;
						d->overlapx-=olw;
						d->overlapy-=olh;
					}
				}

				d = liqcell_getlinknext(d);
			}
			// parent boundary adjustment
			if(c->x<0)c->overlapx-=c->x;
			if(c->y<0)c->overlapy-=c->y;
			if(c->x>=self->w)c->overlapx-=c->x-self->w;
			if(c->y>=self->h)c->overlapy-=c->y-self->h;
		}
		c=liqcell_getlinknext(c);
	}


	int tilew =  liqcell_propgeti(self,"liqcell_child_arrange_nooverlap_minimumw",0);
	int tileh =  liqcell_propgeti(self,"liqcell_child_arrange_nooverlap_minimumh",0);

	//liqapp_log("tile %i,%i",tilew,tileh);

	c=liqcell_getlinkchild(self);
	while(c)
	{
		if( liqcell_getvisible(c))
		{
			if( (c->selected==0) && (c!=currentselection) )
			{
				if(c->overlapx || c->overlapy)
				{
					c->x+=c->overlapx;
					c->y+=c->overlapy;
					liqcell_setdirty(c,1); 
				}

				liqcell_forceinboundparent(c);

			}

			if(tilew && tileh)
			{
				if(c->selected || c==currentselection)
				{
					if((c->overlapx==0) && (c->overlapy==0))
					{
						// are free to make it a bit bigger
						if( c->w < (self->w/2) ) {c->x-=2; c->w += 4;  liqcell_setdirty(c,1); }
						if( c->h < (self->h/2) ) {c->y-=2; c->h += 4;  liqcell_setdirty(c,1); }
						if( (c->selected==0) && (c!=currentselection) )
						{
							liqcell_forceinboundparent(c);
						}
					}
				}
				else
				{
					if((c->overlapx==0) && (c->overlapy==0))
					{
						// do nothing
					}
					else
					{
						// are free to make it a bit smaller
						// need to however find the smallest size
						
					if(currentselection)
					{
						// can only make it smaller whilst pressing :)
						if( (c->w-1) > (tilew) ) {c->x++; c->w-=2;  liqcell_setdirty(c,1); }
						if( (c->h-1) > (tileh) ) {c->y++; c->h-=2;  liqcell_setdirty(c,1); }
					}
						
						
						if( (c->selected==0) && (c!=currentselection) )
						{
							liqcell_forceinboundparent(c);
						}
					}
				}

				//liqcell_forceinboundparent(c);

			}

		}
		c=liqcell_getlinknext(c);
	}

	return 0;

}


// an arrangement function for a set of cells




/**
 * Set the position of the child liqcell's in order to form a grid-like
 * formation.
 * 
 * @param self The parent liqcell
 * @param viscolcount Column count
 * @param visrowcount Row count
 * @return int finished 1 if at target, 0 if still in flight, -1 error
 * 
 */
int liqcell_child_arrange_makegrid_internal(liqcell *self,int viscolcount,int visrowcount,int flymode)
{

	// oo way o.O :)
	liqcell *c;
	
	
	if(viscolcount<=0)
	{
		liqapp_log("liqcell_child_arrange_makegrid invalid colcount");
		return -1;
		
	}
	
	if(visrowcount<=0)
	{
		liqapp_log("liqcell_child_arrange_makegrid invalid rowcount");
		return -1;
	}	
	
	//################################################
	int answercount=0;

	//################################################
	//liqapp_log("liqcell_child_arrange_nooverlap preparing");
	c=liqcell_getlinkchild_visible(self);
	while(c)
	{

		answercount++;		//
		c=liqcell_getlinknext_visible(c);
	}

	if(answercount==0)
	{
		liqapp_log("liqcell_child_arrange_makegrid nothing to weigh, leaving as is");
		return 1;
	}
	
	// 20090521_220229 lcuk : allow graceful reduction from optimal "busy" visibility
	
	int usegracefulreduction = liqcell_propgeti(self,"arrange_usegracefulreduction",1);
	
	
retry:
	if((answercount < (visrowcount * viscolcount)) && usegracefulreduction)
	{
		//
		if(viscolcount > visrowcount)
		{
			if( answercount <= ((viscolcount-1) * visrowcount) )
			{
				viscolcount--;
				goto retry;
			}
		}
		else
		{
			if( answercount <= ((viscolcount) * (visrowcount-1)) )
			{
				visrowcount--;
				goto retry;
			}
		}
	}
	
	
	
	
	
	//int isfloat = liqcell_propgeti(  self, "tilefloating", 0 );

	int ccols=viscolcount;
	int crows=visrowcount;

	int tilew =  liqcell_getw(self) / ccols;
	int tileh =  liqcell_geth(self) / crows;

	int borderw = tilew * 0.05;
	int borderh = tileh * 0.05;
	//tilew-=borderw;
	//tileh-=borderh;
	//borderw/=2;
	//borderh/=2;
	
	if(answercount==1)
	{
		borderw=0;
		borderh=0;
	}
	
	
	


	liqcell_propseti(self,"easytilew",tilew);
	liqcell_propseti(self,"easytileh",tileh);
	
	//liqcell_propseti(self,"easytileflyisfinished",1);

	//################################################
	//int rr = 585000;
	//liqapp_log("liqcell_child_arrange_easytile pushing %i, rr=%i",sizeof(int),rr);

	int row=0;		//row
	int col=0;		//col
	int xx=0;
	int yy=0;
	
	int maxw=0;
	int maxh=0;
	
	int isfinished=1;

	c=liqcell_getlinkchild_visible(self);
	int remain = answercount;
	while(c)
	{
		
		//liqapp_log("aaaa %i,%i,%i",answercount,remain,ccols);
	/*	
		if(remain < ccols && answercount>1 )
		{
			liqapp_log("aaab %i,%i,%i",answercount,remain,ccols);
			// centralize final row
			tilew = liqcell_getw(self) / (remain);
			borderw = tilew * 0.05;
			// just set this high enough to be out of the way
			remain=999999;
		}
		remain--;
	*/
		// work it!
		xx=col*tilew;
		yy=row*tileh;
		
		// 20090521_233205 lcuk : idea: i want to achieve this effect incrementally
		// i only want to move a fraction of the way towards target and alter size by a fraction
		
		if(flymode)
		{
			if( 0!= liqcell_movetowardsrect(c, xx+borderw/2,yy+borderh/2  ,   (tilew-borderw),(tileh-borderh) , 0.4 ) )
			{
				// we are not yet at target, we must
				liqcell_setdirty(c,1);
				
				//liqcell_propseti(self,"easytileflyisfinished",0);
				isfinished=0;
				
			}
		}
		else
		{
		

			liqcell_setpos(c,  xx+borderw/2,yy+borderh/2);
			liqcell_setsize(c, (tilew-borderw),(tileh-borderh));

		}
		// 20090814_190720 lcuk : replace this, its missing the rhs
		//if(c->x+c->w > maxw)maxw=c->x+c->w;
		//if(c->y+c->h > maxh)maxh=c->y+c->h;
		
		
		if(xx+tilew > maxw)maxw=xx+tilew;
		if(yy+tileh > maxh)maxh=yy+tileh;

		
		
		//liqapp_log("makegrid '%s' xy(%i,%i)   xxyy(%i,%i)",self->name,xx,yy,c->x,c->y);

		col++;
		if(col>=ccols){col=0;row++;}


		c=liqcell_getlinknext_visible(c);
	}

	liqcell_setsize( self, maxw,maxh);//tilew * ccols , yy + tileh );

	//liqapp_log("liqcell_child_arrange_makegrid done");
	return isfinished;
}


int liqcell_child_arrange_makegrid_fly(liqcell *self,int viscolcount,int visrowcount)
{
	return liqcell_child_arrange_makegrid_internal(self,viscolcount,visrowcount,1);
}


/**
 * Set the position of the child liqcell's in order to form a grid-like
 * formation.
 * 
 * @param self The parent liqcell
 * @param viscolcount Column count
 * @param visrowcount Row count
 * @return int Success or Failure
 * 
 */
int liqcell_child_arrange_makegrid(liqcell *self,int viscolcount,int visrowcount)
{
	return liqcell_child_arrange_makegrid_internal(self,viscolcount,visrowcount,0);
}







/**
 * Determine and set the number of columns and rows. Based on the number of
 * children liqcells the parent has. This also set's childrens size and position
 * in grid formation.
 * @see liqcell_child_arrange_makegrid()
 * 
 * @param self The parent cell whose children are to be arranged
 * @return int Success or Failure
 * 
 */
int liqcell_child_arrange_easytile(liqcell *self)
{

	// oo way o.O :)
	liqcell *c;

	//################################################
	int answercount=0;

	//################################################ Count the number of visible children
	liqapp_log("liqcell_child_arrange_easytile scanning");
	c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{
			// work it!
			answercount++;		//
		}
		c=liqcell_getlinknext(c);
	}

	if(answercount==0)
	{
		liqapp_log("liqcell_child_arrange_easytile nothing to weigh, leaving as is");
		return 0;

	}

	//################################################ Determine the number of rows/cols
	liqapp_log("liqcell_child_arrange_easytile weighing %i items",answercount);
	int ccols=1;
	// 20090624_010635 lcuk : changed from <3 to <5 to allow for more columns by default on busy ui's
	if(answercount<=12)
	{
		while(ccols<3 && ccols<answercount)ccols++;
	}
	else
	{
		if(answercount<=25)
			while(ccols<5 && ccols<answercount)ccols++;
		else
			while(ccols<7 && ccols<answercount)ccols++;
		
	}
	int crows=answercount/ccols;
	while(ccols*crows<answercount)crows++;

	switch(answercount)
	{
		case 1:
			ccols=1;
			crows=1;
			break;

		case 2:
			ccols=2;
			crows=1;
			break;
		case 3:
			ccols=2;
			crows=2;
			break;
		case 4:
			ccols=2;
			crows=2;
			break;
	}


	liqcell_child_arrange_makegrid(self,ccols,crows);

	liqapp_log("liqcell_child_arrange_easytile done %i x %i grid created",ccols,crows);
	return 0;
}


/**
 * Arrange parent's child cells into a simple row
 * @param self The liqcell to arrange the children of
 * @return int Success or Failure
 */
int liqcell_child_arrange_easyrow(liqcell *self)
{
	liqcell *c;

	//int availw=liqcell_getw(self);
	//int availh=liqcell_geth(self);


	int x=0;
	int y=0;

	int allmaxw=0;
	int rowmaxh=0;

	//################################################
	//int answercount=0;

	//################################################
	//liqapp_log("liqcell_child_arrange_nooverlap preparing");
	c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{

			int w=liqcell_getw(c);
			int h=liqcell_geth(c);
			// easyrow never ends..
			//if(x+w>availw && x>0)
			//{
			//	// cannot fit (we ignore the first so we get at least 1 on the flow)
			//	x=0;
			//	y+=rowmaxh;
			//	rowmaxh=0;
			//}

			if(x+w>allmaxw) allmaxw = x+w;		// make sure we store the widest total width
			if(h>rowmaxh) rowmaxh = h;			//

			liqcell_setpos(c, x,y);
			x+=w;
		}
		c=liqcell_getlinknext(c);
	}
	liqcell_setsize(self, allmaxw, rowmaxh);
	return 0;
}

/**
 * Arrange parent's child cells into a simple column
 * @param self The liqcell to arrange the children of
 * @return int Success or Failure
 */
int liqcell_child_arrange_easycol(liqcell *self)
{
	liqcell *c;

	//int availw=liqcell_getw(self);
	//int availh=liqcell_geth(self);


	int x=0;
	int y=0;

	int rowmaxw=0;
	int allmaxh=0;

	//################################################
	//int answercount=0;

	//################################################
	//liqapp_log("liqcell_child_arrange_nooverlap preparing");
	c=liqcell_getlinkchild(self);
	while(c)
	{
		if(liqcell_getvisible(c))
		{

			int w=liqcell_getw(c);
			int h=liqcell_geth(c);
			// easycol almost always resets..
			//if(x>0)
			//{
			//	// cannot fit (we ignore the first so we get at least 1 on the flow)
			//	x=0;
			//	y+=rowmaxh;
			//	rowmaxh=0;
			//}

			if(w>rowmaxw) rowmaxw = w;		// make sure we store the widest total width
			if(y+h>allmaxh) allmaxh = y+h;			//

			liqcell_setpos(c, x,y);
			y+=h;
		}
		c=liqcell_getlinknext(c);
	}
	liqcell_setsize(self, rowmaxw, allmaxh);
	return 0;
}
