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
 * basic sketch
 *
 */





#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "liqapp.h"
#include "liqfont.h"
#include "liqcanvas.h"
#include "liqsketch.h"

#ifdef __cplusplus
extern "C" {
#endif

//#include "liqdoc.h"

//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#include <pthread.h>
//#include <sched.h>
	// crashing liqflow via osc when removing a node, this is bad
	// its a threading issue tho
	// one process is recurings each stroke, the other is using it
	//

//static pthread_mutex_t sketch_op_lock = PTHREAD_MUTEX_INITIALIZER;
	//pthread_mutex_lock(&sketch_op_lock);
	//pthread_mutex_unlock(&sketch_op_lock);

liqpoint *liqpoint_new()
{
	liqpoint *self = (liqpoint *)calloc(sizeof(liqpoint),1);
	if(self==NULL) {  liqapp_errorandfail(-1, "liqpoint new failed" ); return NULL; }
	// NULL everything
	//memset((char *)self,0,sizeof(liqpoint));
	self->usagecount=1;

	
	return self;
}

liqpoint * liqpoint_hold(liqpoint *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}

void liqpoint_release(liqpoint *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqpoint_free(self);
}

void liqpoint_free(liqpoint *self)
{
	//liqapp_log("liqpoint free hmmm");
	
	
	free(self);
}




//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################











inline void liqpoint_copy(liqpoint *self,liqpoint *s) 
{
	// inline memcpy would work for this, but it will do as is
	self->x=s->x;  self->y=s->y;  self->z=s->z; self->t=s->t;
}

liqpoint * liqpoint_clone(liqpoint *s) 
{
	liqpoint *self = liqpoint_new();
	liqpoint_copy(self,s);
	return self;
}

inline void liqpoint_getdiff(liqpoint *self,liqpoint *s,liqpoint *e) 
{
	self->x=e->x-s->x;  self->y=e->y-s->y;  self->z=e->z-s->z; self->t=e->t-s->t;
}
inline int liqpoint_issame(liqpoint *self,liqpoint *s) 
{
	return (self->x==s->x && self->y==s->y && self->z && self->z==s->z);
}

//##################################################################

void liqpointrange_start(liqpointrange *self,liqpoint *p)
{
	self->xl = p->x;
	self->xr = p->x;
	self->yt = p->y;
	self->yb = p->y;
	self->zf = p->z;
	self->zb = p->z;
}
void liqpointrange_extendrubberband(liqpointrange *self,liqpoint *p)
{
	if(self->xl > p->x)self->xl = p->x;
	if(self->xr < p->x)self->xr = p->x;
	if(self->yt > p->y)self->yt = p->y;
	if(self->yb < p->y)self->yb = p->y;
	if(self->zf < p->z)self->zf = p->z;
	if(self->zb > p->z)self->zb = p->z;
}
int liqpointrange_isconnected(liqpointrange *self,liqpointrange *b)
{
	
	if(self->xl > b->xr)return 0;
	if(self->xr < b->xl)return 0;
	if(self->yt > b->yb)return 0;
	if(self->yb < b->yt)return 0;
	return 1;
}



void          liqpointrange_start_xyz(liqpointrange *self,int px,int py,int pz)
{
	self->xl = px;
	self->xr = px;
	self->yt = py;
	self->yb = py;
	self->zf = pz;
	self->zb = pz;
}
void          liqpointrange_extendrubberband_xyz(liqpointrange *self,int px,int py,int pz)
{
	if(self->xl > px)self->xl = px;
	if(self->xr < px)self->xr = px;
	if(self->yt > py)self->yt = py;
	if(self->yb < py)self->yb = py;
	if(self->zf < pz)self->zf = pz;
	if(self->zb > pz)self->zb = pz;
}




//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################



liqstroke *liqstroke_new()
{
	liqstroke *self = (liqstroke *)calloc(sizeof(liqstroke),1);
	if(self==NULL) {  liqapp_errorandfail(-1, "liqstroke new failed" ); return NULL; }
	// NULL everything
	//memset((char *)self,0,sizeof(liqstroke));
	self->usagecount=1;

	self->pen_thick=1;
	
	return self;
}

liqstroke * liqstroke_hold(liqstroke *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}

void liqstroke_release(liqstroke *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqstroke_free(self);
}

void liqstroke_free(liqstroke *self)
{
	
	/*
	 
		liqapp_log("liqstroke free_test");
		
	*/
	liqstroke_clear(self);
	free(self);
}




//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################











void liqstroke_clear(liqstroke *self)
{
	self->pointcount=0;
	// we throw away the points we added
	while(self->pointfirst)
	{
		liqpoint *p = self->pointfirst;
		self->pointfirst = p->linknext;
		liqpoint_release(p);
	}
	self->pointlast=NULL;
	if(self->quadchain)
	{
		free(self->quadchain);
		self->quadchain=NULL;
	}
}


liqstroke *liqstroke_clone(liqstroke *s) 
{
	liqstroke *self = liqstroke_new();
	// first, a shallow copy..
	// memcpy isnt right here, we dont want to do a binary clone on all the links etc
	//memcpy((char *)self,(char *)s,sizeof( liqstroke ));
	// only do the specific fields
	self->pen_y=s->pen_y;
	self->pen_u=s->pen_u;
	self->pen_v=s->pen_v;
	self->pen_thick=s->pen_thick;
	liqpoint *p=s->pointfirst;
	while(p)
	{
		liqstroke_appendpoint(self,liqpoint_clone(p));
		p=p->linknext;
	}
	
	return self;
}




int           liqstroke_totallength(liqstroke *self)
{
	liqpoint *p = self->pointfirst;
	liqpoint *q = NULL;
	int m=0;
	while(p)
	{
		q=p->linknext;
		if(!q)break;
		
		int dx=q->x-p->x;
		int dy=q->y-p->y;
		int dm=sqrt(dx*dx+dy*dy);
		m+=dm;
		
		p=q;
	}
	return m;
}


void liqstroke_appendpoint(liqstroke *self,liqpoint *p)
{
	//liqpoint *p = liqpoint_alloc();
	//p->x=s->x; p->y=s->y;  p->z=s->z;  p->t = s->t;
	if(self->pointcount==0)
	{
		p->linknext = NULL;
		p->linkprev = NULL;
		self->pointfirst = p;
		self->pointlast = p;
		self->pointcount = 1;
		liqpointrange_start( &self->boundingbox, p );		
	}
	else
	{
		p->linkprev = self->pointlast;	
		p->linknext = NULL;	
		self->pointlast->linknext = p;
		self->pointlast = p;
		self->pointcount++;
		liqpointrange_extendrubberband(&self->boundingbox, p);		
	}
}
void liqstroke_start(liqstroke *self,int px,int py, int pz,unsigned long pt)
{
	liqpoint *p = liqpoint_new();	
	p->x=px; p->y=py;  p->z=pz; p->t=pt;
	liqstroke_appendpoint(self,p);
}

//double atan2( double y, double x );

void liqstroke_extend(liqstroke *self,int px,int py, int pz,unsigned long pt)
{
	switch(self->strokekind)
	{
		case 0:	// normal stroke
			{
				liqpoint *p = liqpoint_new();	
				p->x=px; p->y=py;  p->z=pz;	 p->t=pt;
				liqstroke_appendpoint(self,p);
			}
			break;

		case 1: // p2p line
			
			if(self->pointcount<2)
			{
				liqpoint *p = liqpoint_new();	
				p->x=px; p->y=py;  p->z=pz;	 p->t=pt;
				liqstroke_appendpoint(self,p);
			}
			else
			{
				liqpoint *p = self->pointlast;	
				p->x=px; p->y=py;  p->z=pz; p->t=pt;
				liqpointrange_start( &self->boundingbox, self->pointfirst );
				liqpointrange_extendrubberband( &self->boundingbox, self->pointlast );
			}
			break;
		
		case 2: // box
			
			if(self->pointcount<2)
			{
				liqpoint *p = liqpoint_new();	
				p->x=px; p->y=py;  p->z=pz;	 p->t=pt;
				liqstroke_appendpoint(self,p);
			}
			else
			{
				liqpoint *p = self->pointlast;	
				p->x=px; p->y=py;  p->z=pz; p->t=pt;
				liqpointrange_start( &self->boundingbox, self->pointfirst );
				liqpointrange_extendrubberband( &self->boundingbox, self->pointlast );
			}
			break;
		
		case 3: // filled
			if(self->pointcount<2)
			{
				liqpoint *p = liqpoint_new();	
				p->x=px; p->y=py;  p->z=pz;	 p->t=pt;
				liqstroke_appendpoint(self,p);
			}
			else
			{
				liqpoint *p = self->pointlast;	
				p->x=px; p->y=py;  p->z=pz; p->t=pt;
				liqpointrange_start( &self->boundingbox, self->pointfirst );
				liqpointrange_extendrubberband( &self->boundingbox, self->pointlast );
			}
			break;

		case 4: // stamp
			if(self->pointcount<2)
			{
				liqpoint *p = liqpoint_new();	
				p->x=px; p->y=py;  p->z=pz;	 p->t=pt;
				liqstroke_appendpoint(self,p);
			}
			else
			{
				liqpoint *p = self->pointlast;	
				p->x=px; p->y=py;  p->z=pz; p->t=pt;
				liqpointrange_start( &self->boundingbox, self->pointfirst );
				liqpointrange_extendrubberband( &self->boundingbox, self->pointlast );
			}
			break;
	}
}


int          liqstroke_hittest(liqstroke *self,int px,int py)
{
	// depending upon what kind of stroke this is.
	// we react differently.
	// maybe using the stroke for this purpose is wrong
	// i should be using a primative which can use strokes as input.
	
	return 0;
}


void          liqstroke_ensurepositive(liqstroke *self)
{
	if(self->strokekind>1 && self->pointcount>1)
	{
		liqpoint *p = self->pointfirst;	
		liqpoint *q = self->pointlast;
		if(p->x>q->x){int t=p->x; p->x=q->x; q->x=t; }
		if(p->y>q->y){int t=p->y; p->y=q->y; q->y=t; }
		
		
	}
}



int liqstroke_isconnected(liqstroke *self,liqstroke *o)
{
	//
	if( !liqpointrange_isconnected(&self->boundingbox,&o->boundingbox))
	{
		return 0;
	}
	// we MIGHT be connected
	
	liqpoint *p=self->pointfirst;
	while(p)
	{
		liqpoint *q=o->pointfirst;
		while(q)
		{
			int dx=p->x-q->x;
			int dx2=(dx*dx);


			int dy=p->y-q->y;
			int dy2=dy*dy;
			if(dx2+dy2<64)
			{
				if(sqrt(dx2+dy2)<8)
				{
					// ! we are connected
					return 1;
				}
			}

						
			q=q->linknext;
		}
		
		p=p->linknext;
	}
	// no, we arent
	return 0;
	
	
}


char *          liqstroke_quadchainbuild(liqstroke *self)
{

	// depending upon what kind of stroke this is.
	// we react differently.
	// maybe using the stroke for this purpose is wrong
	// i should be using a primative which can use strokes as input.
	
	// i should be taking the entire stroke inside a unit SQUARE, not as I currently do and stretch a rectangle
	
	if(self->pointcount==0)
		return NULL;
	if(self->quadchain)
		return self->quadchain;
	
	int box = self->boundingbox.xl;
	int boy = self->boundingbox.yt;
	
	int bmx = self->boundingbox.xr-box;
	int bmy = self->boundingbox.yb-boy;

	int bcx = box+bmx/2;
	int bcy = boy+bmy/2;
	
	
	int bfac = (bmx > bmy) ? bmx : bmy;
	
	
	box = bcx-bfac/2;
	boy = bcy-bfac/2;
	
	bfac++;


	if(bmx==0 || bmy==0)
		return NULL;

	
	char quadchain[33];
	int quadused=0;
	int quadcurr=-1;
	
	


	
	liqpoint *p = self->pointfirst;
	while(p)
	{	
		// normalize to quadrant 0,1,2,3,4
		//int nx = 2 + ((p->x - bcx) * 5 / bfac);
		//int ny = 2 + ((p->y - bcy) * 5 / bfac);
		//int nq = ny*5+nx;
		
		// normalize to quadrant 0,1,2
		//int nx = 1 + ((p->x - bcx) * 3 / bfac);
		//int ny = 1 + ((p->y - bcy) * 3 / bfac);
		//int nq = ny*3+nx;		
	
		// normalize to quadrant 0,1,2
		int nx = ((p->x - box) * 3 / bfac);		// 3 steps
		int ny = ((p->y - boy) * 3 / bfac);		// 3 steps
		int nq = ny*3+nx;	
		
		if(quadcurr==-1)
		{
				//liqapp_log("%i,%i,%i",nx,ny,nq);

			quadcurr = nq;
		}
		else
		{
			if(nq!=quadcurr)
			{
				//liqapp_log("%i,%i,%i",nx,ny,nq);
				
				// stroke has changed to a different quadrant
				if(quadused<32)
					quadchain[quadused++] = 97 + quadcurr;
				else
					break;
				quadcurr = nq;
			}
		}		
		p=p->linknext;
	}
	if(quadused<32)
		quadchain[quadused++] = 97+quadcurr;
		
	quadchain[quadused++] = 0;
	self->quadchain = strdup(quadchain);
	return self->quadchain;
}


//##################################################################



//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################


liqsketch 	* liqsketch_newfromfile(char *filename)
{
	liqsketch *self = liqsketch_new();
	if(self==NULL) {  liqapp_errorandfail(-1, "liqsketch new failed" ); return NULL; }
	
	if(0 != liqsketch_fileload(self,filename))
	{
		liqapp_log("liqsketch_newfromfile error loading '%s'",filename);
		liqsketch_free(self);
		return NULL;
	}
	return self;
}



//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################



liqsketch *liqsketch_new()
{
	liqsketch *self = (liqsketch *)calloc(sizeof(liqsketch),1);
	if(self==NULL) {  liqapp_errorandfail(-1, "liqsketch new failed" ); return NULL; }
	// NULL everything
	//memset((char *)self,0,sizeof(liqsketch));
	self->usagecount=1;
	
	self->pixelwidth=800;//=canvas.pixelwidth;
	self->pixelheight=480;//canvas.pixelheight;

	self->dpix=225;// *canvas.scalew;
	self->dpiy=225;// *canvas.scaleh;
	
	return self;
}

liqsketch * liqsketch_hold(liqsketch *self)
{
	// use this to hold onto an object which someone else created
	if(self)self->usagecount++;
	return self;
}

void liqsketch_release(liqsketch *self)
{
	// use this when you are finished with an object
	if(!self) return;
	self->usagecount--;
	if(!self->usagecount) liqsketch_free(self);
}

void liqsketch_free(liqsketch *self)
{
	liqapp_log("liqsketch free");
	liqsketch_clear(self);
	free(self);
}




//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################
//#########################################################################

void liqsketch_clear(liqsketch *self)
{
	// we throw away the points we added
	self->strokecount=0;
	while(self->strokefirst)
	{
		liqstroke *s = self->strokefirst;
		self->strokefirst = s->linknext;
		liqstroke_free(s);
	}

	if(self->title){ free(self->title); self->title=NULL; }
	if(self->filename) { free(self->filename); self->filename=NULL; }


	if(self->backgroundfilename) { free(self->backgroundfilename); self->backgroundfilename=NULL; }
	if(self->backgroundimage) { liqimage_release(self->backgroundimage); self->backgroundimage=NULL; }
	if(self->backgroundsketch) { liqsketch_release(self->backgroundsketch); self->backgroundsketch=NULL; }

	self->backgroundstyle=0;
	self->backgroundcoloryuv= 0;//vcolor_YUV(0,128,128);



	self->strokelast=NULL;
}




void liqsketch_coordchange_scr_to_page(liqsketch *self,int scrx,int scry,int scrw,int scrh, int scrdpix,int scrdpiy,int *rx,int *ry)
{
//	*rx = scrx * (scrdpix * scrw) / (self->pixelwidth  * self->dpix);
//	*ry = scry * (scrdpiy * scrh) / (self->pixelheight * self->dpiy);

	*rx = scrx * (self->pixelwidth  * self->dpix) / (scrdpix * scrw);
	*ry = scry * (self->pixelheight * self->dpiy) / (scrdpiy * scrh);

}


void liqsketch_titlechange(liqsketch *self,char *title)
{
	if(self->title){ free(self->title); self->title=NULL; }
	if(!title || *title==0) return;
	self->title = strdup(title);
}

// '''''''''''''''''''''''''''''''''''''''''''''''''''''''''

void          liqsketch_strokeremove(liqsketch *self,liqstroke *s)
{
	//
	if(!s)return;
	
	//pthread_mutex_lock(&sketch_op_lock);
	//
	
	//if(! (s->linkpage==self) ) return;
	liqstroke *l=s->linkprev;
	liqstroke *r=s->linknext;
	
	if(self->strokefirst==s)
	{
		// remove the front most item
		self->strokefirst=r;
		s->linknext=NULL;

	}

	if(self->strokelast==s)
	{
		// remove the front most item
		self->strokelast=l;
		s->linkprev=NULL;
	}
	if(l)l->linknext=r;
	if(r)r->linkprev=l;
	
	s->linkprev=NULL;
	s->linknext=NULL;
	s->linkpage=NULL;
	
	//pthread_mutex_unlock(&sketch_op_lock);
	
	//liqstroke_free(s);
	liqstroke_release(s);
	
	
	self->strokecount--;
	
}



void liqsketch_strokeinsert(liqsketch *self,liqstroke *s)
{
	liqsketch_strokeupdate(self,s);

	//liqstroke *s = liqstroke_alloc();
	s->linkprev = self->strokelast;
	if(!self->strokefirst) self->strokefirst = s;
	if( self->strokelast)  self->strokelast->linknext = s;
	self->strokelast = s;
	self->strokecount++;
	//return s;
}
void liqsketch_islandswap(liqsketch *self,int islandnumberfrom,int islandnumberto)
{
		liqstroke *o = self->strokefirst;
		while(o)
		{
			if(o->islandnumber==islandnumberfrom) o->islandnumber=islandnumberto;
			o=o->linknext;
		}
}

void liqsketch_islandclear(liqsketch *self)
{
		liqstroke *o = self->strokefirst;
		while(o)
		{
			o->islandnumber=0;
			o=o->linknext;
		}
		self->islandcount=0;
}

void liqsketch_islandcalcone(liqsketch *self,liqstroke *s)
{	
			// we need to check out the others
		liqstroke *o = self->strokefirst;
		while(o)
		{
			if(o!=s)
			{
				// check
				if(liqstroke_isconnected(s,o))
				{
					//	liqapp_log("conn");
					// yes, they are connected to us
					if(s->islandnumber==0)
					{
						// obtain the island ref from our neighbour
						if(o->islandnumber==0)
						{
							s->islandnumber = (self->islandcount++);
							o->islandnumber = s->islandnumber;
						}
						else
						{
							s->islandnumber=o->islandnumber;
						}
					}
					else
					{
						// islands merging, keep the lowest
						if(s->islandnumber < o->islandnumber)
							liqsketch_islandswap(self,o->islandnumber,s->islandnumber);
						else
							liqsketch_islandswap(self,s->islandnumber,o->islandnumber);

					}
				}
			}
			o=o->linknext;
		}
		if(s->islandnumber==0)
		{
			// make one now :)
			s->islandnumber = (self->islandcount++);
		}
		
}

void liqsketch_islandcalcall(liqsketch *self)
{

	liqsketch_islandclear(self);
		// we need to check out the others
	liqstroke *o = self->strokefirst;
	while(o)
	{
		liqsketch_islandcalcone(self,o);
		o=o->linknext;
	}
		
}


void liqsketch_strokeupdate(liqsketch *self,liqstroke *s)
{
liqpoint p1,p2;
	// the stroke has been extended
	// this may be handled by a callback from the stroke itself
	// not sure yet
	// however its kicked in we must update our bounding box
	p1.x = s->boundingbox.xl;
	p1.y = s->boundingbox.yt;
	p1.z = s->boundingbox.zf;
	p2.x = s->boundingbox.xr;
	p2.y = s->boundingbox.yb;
	p2.z = s->boundingbox.zb;
	if(self->strokecount==0)
	{
		liqpointrange_start(&self->boundingbox, &p1);
		liqpointrange_extendrubberband(&self->boundingbox, &p2);
	}
	else
	{
		liqpointrange_extendrubberband(&self->boundingbox, &p1);
		liqpointrange_extendrubberband(&self->boundingbox, &p2);
	}
	
	// if its island is 0 then we must scan for it :)
	//liqsketch_islandcalcone(self,s);
}


void          liqsketch_boundwholearea(liqsketch *self)
{
	// make the page boundary be the full boundary
liqpoint p1,p2;
	p1.x = 0;
	p1.y = 0;
	p1.z = self->boundingbox.zf;
	p2.x = self->pixelwidth;
	p2.y = self->pixelheight;
	p2.z = self->boundingbox.zb;

	liqpointrange_extendrubberband(&self->boundingbox, &p1);
	liqpointrange_extendrubberband(&self->boundingbox, &p2);

}






int liqsketch_filesave(liqsketch *self,char *filename)
{
	liqapp_log("filesave, saving to '%s'",filename);

	if(self->filename) { free(self->filename); self->filename=NULL; }
	self->filename = strdup(filename);

	FILE *fd;
	//int   ri;
	fd = fopen(filename, "w");
	if(fd==NULL){ liqapp_log("filesave, cannot open '%s' for writing",filename); return -1; }
	// actual file data

	liqapp_log("filesave, writing head");

	fprintf(fd,									"page:%i,%i,%i,%i\n",
																									self->pixelwidth,
																									self->pixelheight,
																									self->dpix,
																									self->dpiy
																									);
	liqapp_log("filesave, writing strokes");
	liqstroke *stroke=self->strokefirst;
	while(stroke)
	{
		
		fprintf(fd,								"\tstroke:%i,%i,%i,%i\n",
																									stroke->pen_y,
																									stroke->pen_u,
																									stroke->pen_v,
																									stroke->strokekind
																									);
		liqpoint *point;

		point = stroke->pointfirst;
		while(point)
		{
			fprintf(fd,							"\t\tpoint:%lu,%i,%i,%i\n",
																									point->t,
																									point->x,
																									point->y,
																									point->z
																									);
			point=point->linknext;
		}

		stroke=stroke->linknext;
	}
	// 20090422_194645 lcuk : todo append a save event id: datestamp,user,machine
	// 20090422_194637 lcuk : todo save the extra tokens

	liqapp_log("filesave, closing");			
	fclose(fd);
	liqapp_log("filesave, finished");
	//if(ri<0){ liqapp_log("filesave, cannot write to '%s'",filename); return -2; }
	return 0;
}

int liqsketch_fileload(liqsketch *self,char *filename)
{
	return liqsketch_fileload_memstream(self,filename,NULL,0);
}

int liqsketch_fileload_memstream(liqsketch *self,char *filename,char *srcdata, int srcsize)
{



	//liqapp_log("liqsketch_fileload '%s'",filename);
	char *indat;
	//int err=0;
	liqsketch_clear(self);
	
	

	//liqapp_log("liqsketch_fileload 2 '%s'",filename);
	

	if(self->filename) { free(self->filename); self->filename=NULL; }

	liqstroke *stroke=NULL;
	//liqpoint *point=NULL;





	int linenum=1;	



	FILE *fn=NULL;
	
	if(!srcdata)
	{
		self->filename = strdup(filename);
	
		fn=fopen(filename,"r");
		if(!fn)
		{
			liqapp_log("liqsketch_fileload could not open '%s'",filename);
			return -1;
		}
		
	}
	int srcpos=0;

	
	

	char lineraw[512];
	int linemax=511;
	//char *line=NULL;
	while(  (fn && !feof(fn)) || ( (!fn) && (srcpos<srcsize) && (srcdata[srcpos]) ) )
	{
		char * rc;
		
		if(fn)
		{
		
			rc=fgets(lineraw,linemax, (FILE*) fn);
			if(!rc)break;
		}
		else
		{
			char *ss = &srcdata[srcpos];
			char *pp = strchr(ss,'\n');
			char *tt = pp;
			if(!tt)tt=&srcdata[srcsize-1];
			int cnt=(tt-ss);
			if(cnt>512) cnt=512;
			
			if(tt)
			{
				srcpos += cnt;
				strncpy(lineraw,ss,cnt);
				lineraw[cnt]=0;
				lineraw[sizeof(lineraw)]=0;
				//if(pp)srcpos++;
				while( srcdata[srcpos]==10 || srcdata[srcpos]==13 )srcpos++;
			}
			//liqapp_log("mem read: '%s', cnt=%i,sp=%i",lineraw,cnt,srcpos);
		}


		if(linenum==1)
		{
			
						
			if(strncmp(lineraw,"page:",5) != 0)
			{
				// invalid header
				if(fn)fclose(fn);
				{ return liqapp_warnandcontinue(-1,"liqsketch_fileload invalid file header"); }						
		
			}				
						
		}
		
		indat=lineraw;
		
		
		// proof of concept
		// load in the points first
		// should be MUCH faster
		
		
		int indentlevel=0;
		while(*indat==9)
		{
			indentlevel++;
			indat++;
		}
		int isdone=0;		// use this to save some time loading (skips the other scanf's after matching one)

		
		{
			unsigned long pt=0;
			int px=0;
			int py=0;
			int pz=0;
			int res = sscanf(indat,"point: %lu, %i, %i, %i",&pt,&px,&py,&pz);
			if(res==4)
			{
				//liqapp_log("%4i ++point ++ %i '%s' == %lu,%i,%i,%i",linenum,res,indat,pt,px,py,pz);

				if(stroke==NULL) 
				{
					//doc_close(&doc);
					if(fn)fclose(fn);
					{ return liqapp_warnandcontinue(-1,"liqsketch_fileload point without stroke"); }
				}
				if(stroke->pointcount==0)
				{
					liqstroke_start(stroke,px,py,pz,pt);
					liqsketch_strokeinsert(self,stroke);
				}
				else
				{
					liqstroke_extend(stroke,px,py,pz,pt);
					liqsketch_strokeupdate(self,stroke);
				}
				//stroke->pointlast->t = pt;
				isdone=1;

			}
			else
			{
				//liqapp_log("%4i --point -- %i '%s' == %i,%i,%i,%i",linenum,res,indat,pt,px,py,pz);
			}		
		
		}

		if(!isdone)
		{
			int  peny=0;
			int  penu=0;
			int  penv=0;
			int  strokekind=0;
			int  res = sscanf(indat,"stroke: %i, %i, %i, %i",&peny,&penu,&penv,&strokekind);
			if(res==3){ strokekind=0; res=4; }	// todo: make all this import cleaner, this is a fudge
			if(res==4)
			{
				
				// 20090504_180359 lcuk : since I am starting a new stroke, I can assume if I already have one I would like
				// 20090504_180402 lcuk : to run liqstroke_quadchainbuild on it
				
				if(stroke)
				{
					liqstroke_quadchainbuild(stroke);
				}
				
				
				//liqapp_log("%4i ++stroke++ %i '%s' == %i,%i,%i kind=%i",linenum,res,indat,peny,penu,penv,strokekind);

				//if(stroke==NULL) 
				//	{ return liqapp_warnandcontinue(-1,"liqsketch_fileload point without stroke"); }
				stroke = liqstroke_new();
				stroke->pen_y=peny;
				stroke->pen_u=penu;
				stroke->pen_v=penv;
				stroke->strokekind=strokekind;
				if(strokekind==4)
				{
					stroke->mediapage = self;
				}
				isdone=1;
			}
		}
		
		
		if(!isdone)
		{
			int  pagew=0;
			int  pageh=0;
			int  pagedpix=0;
			int  pagedpiy=0;
			int res = sscanf(indat,"page: %i, %i, %i, %i",&pagew,&pageh,&pagedpix,&pagedpiy);
			if(res==4)
			{
				//liqapp_log("%4i ++page  ++ %i '%s' == %i,%i,%i,%i",linenum,res,indat,pagew,pageh,pagedpix,pagedpiy);
				self->pixelwidth =pagew;
				self->pixelheight=pageh;
				self->dpix=pagedpix;
				self->dpiy=pagedpiy;
				isdone=1;
			}
		}		

		
		// 20090422_194605 lcuk : todo load in the extratokens
		
		linenum++;
	}
	//doc_close(&doc);
	
	if(fn)fclose(fn);

				// 20090504_180439 lcuk : again, if i have a stroke and i reach the end of file then calc it
				// 20090504_180454 lcuk : this is entirely to see how much speedup/slowdown i have
				
				// 20090720_191331 lcuk : test remove it to see how quick we can make this :)
				//if(stroke)
				//{
				//	liqstroke_quadchainbuild(stroke);
				//}	
	

	//pagefilename_test(filename);

	return self->strokecount>0 ? 0 : -1;
}





















void liqsketch_setbackgroundimage(liqsketch *self,liqimage *image)
{
	if(self->backgroundimage)
	{
		self->backgroundimage=NULL;
	}
	if(image)
	{
		self->backgroundstyle=2;
		self->backgroundimage      = liqimage_hold(image);
	}
}


//


void liqsketch_setbackgroundsketch(liqsketch *self,liqsketch *sketch)
{
	if(self->backgroundsketch)
	{
		liqsketch_release(self->backgroundsketch);
		self->backgroundsketch=NULL;
	}
	if(sketch)
	{
		self->backgroundstyle=3;
		self->backgroundsketch      = liqsketch_hold(sketch);
	}
}





liqsketch *liqsketch_getbackgroundsketch(liqsketch *self)
{
	return self->backgroundsketch;
}


liqimage *liqsketch_getbackgroundimage(liqsketch *self)
{
	return self->backgroundimage;
}

#ifdef __cplusplus
}
#endif

