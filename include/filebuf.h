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
 * header for filebuffer routines
 *
 */



#ifndef FILEBUF_H
#define FILEBUF_H

#ifdef __cplusplus
extern "C" {
#endif



//##########################################################################	
//##########################################################################	filebuf class.  quick and dirty way to load a named file
//##########################################################################	

struct 					filebuf
{
	char 				*filename;		// original filename of the document
	int					filelength;		// actual filesize
	char				*filedata;		// malloced memory
};

//##########################################################################	

int 					filebuf_open(struct filebuf *self,char *filename);

//##########################################################################	

int						filebuf_close(struct filebuf *self);

#ifdef __cplusplus
}
#endif



#endif
