/* liqbase
 * Copyright (C) 2011 Gary Birkett
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
 * Header for tag cloud helper functions
 *
 */


// tag group management

// /home/user/.liqbase/tags


// liqcloud
//	all liqnode
// liqnode
//	key
//	all liqleaf
// liqleaf
//	key
//	filename



#ifndef liqtag_H
#define liqtag_H

#include <stdio.h>
#include <dirent.h>

#include "liqapp.h"

#ifdef __cplusplus
extern "C" {
#endif


//################################################################################
//################################################################################

// tag leaves hold that actual link members for the tag mentioned.
// a technical extension would be to discover the list of tags using this item in future.
// for this, it may be practical to maintain a working tree of actual files in the system
// linking to this node where appropriate


typedef struct liqtagleaf
{	
	unsigned int usagecount;
	struct liqtagleaf *linkprev;
	struct liqtagleaf *linknext;

	char *key;
	char *filename;
	//char *created;
	//char *creator;
} 
	liqtagleaf;

liqtagleaf * 	liqtagleaf_new();
liqtagleaf * 	liqtagleaf_hold(liqtagleaf *self);
void    		liqtagleaf_release(liqtagleaf *self);
void    		liqtagleaf_free(liqtagleaf *self);



//################################################################################
//################################################################################


//################################################################################
//################################################################################
// a tagitem holds a list of its contents
// a count of leavves and potential to range boundaries for time periods etc
// use of boundingbox for this would be fun, just like my old code.


typedef struct liqtagnode
{	
	unsigned int usagecount;
	struct liqtagnode *linkprev;
	struct liqtagnode *linknext;

	char *key;
	char *filename;
	//char *description;
	//char *created;
	//char *creator;
	
	int   leafcount;		// how many members are in this tagnode
	liqtagleaf *leaffirst;	// list of members, may be filenames,words,phrases,* that fits one per line in a text file.
	liqtagleaf *leaflast;	// list of members, may be filenames,words,phrases,* that fits one per line in a text file.
	
} 
	liqtagnode;

liqtagnode * 		liqtagnode_new();
liqtagnode * 		liqtagnode_hold(liqtagnode *self);
void    		liqtagnode_release(liqtagnode *self);
void    		liqtagnode_free(liqtagnode *self);

//################################################################################
//################################################################################



int liqtagnode_clear(liqtagnode *self);										// Clear the node
liqtagleaf *liqtagnode_findleaf(liqtagnode *self,const char *itemkey);						// check if itemkey is used in this node
liqtagleaf *liqtagnode_findorcreateleaf(liqtagnode *self,const char *leafkey, const char *leafdata);			// insert items into the node

//################################################################################
//################################################################################

// the tagcloud is opened on the whole folder.
// it should scan and create a whole bunch of member tagitems
// each tagitem holds a list of its contents


typedef struct liqtagcloud
{	
	unsigned int usagecount;
	struct liqtagcloud *linkprev;
	struct liqtagcloud *linknext;

	int   nodecount;		// how many members are in this tagcloud
	liqtagnode *nodefirst;		// list of members, may be filenames,words,phrases,* that fits one per line in a text file.
	liqtagnode *nodelast;		// list of members, may be filenames,words,phrases,* that fits one per line in a text file.

	
} 
	liqtagcloud;

liqtagcloud * 		liqtagcloud_new();
liqtagcloud * 		liqtagcloud_hold(liqtagcloud *self);
void    		liqtagcloud_release(liqtagcloud *self);
void    		liqtagcloud_free(liqtagcloud *self);


//################################################################################
//################################################################################


liqtagnode *liqtagcloud_findnode(liqtagcloud *self,const char *nodekey);							// check if itemkey is used in this cloud
int liqtagcloud_containsleaf(liqtagcloud *self,const char *leafkey);								// check if itemkey is used in this cloud
liqtagnode *liqtagcloud_findorcreatenode(liqtagcloud *self,const char *tagname);						// insert items into the node
int liqtagcloud_systemstart();

extern liqtagcloud *system_tagcloud;



void liqtag_quicksaveas( const char *tagname, const char *data_filename );					// helper routine for sketcheditor to save a new file as a specific tag
int tagnode_save(liqtagnode *self);


#ifdef __cplusplus
}
#endif

#endif


