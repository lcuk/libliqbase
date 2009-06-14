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
 * Header for a horrible sketch filename decoder
 *
 */

#ifndef LIQSKETCHPAGEFILENAME_H
#define LIQSKETCHPAGEFILENAME_H


// horrid core function, my apologies

struct pagefilename
{
	// From: /home/user/MyDocs/_apg/liq.lcuk.20080525_021309.page.base

	char filepath[255+1];		// /home/user/MyDocs/_apg
	//char filename[255+1];		// liq.lcuk.20080525_021309.page.base
	char fileuser[15+1];		// lcuk
	char filedate[15+1];		// 20080525_021309
	char fileclass[20+1];		// page
	char filetitle[20+1];		// base
};

int 		  pagefilename_breakapart(struct pagefilename *self,char *filename);
char 		  *pagefilename_rebuild(struct pagefilename *self,char *bufferresultfilename,int buffermax);


#endif
