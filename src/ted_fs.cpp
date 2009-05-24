/***************************************************************************
 *   TED - Network Event Daemon	                                           *
 *   by evilsocket - evilsocket@gmail.com - http://www.evilsocket.net      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "ted_fs.h"

unsigned short ted_isfile( char *path ){
	FILE *fp = NULL;
	
	if( (fp = fopen( path, "r" )) ){
		fclose(fp);
		return 1;
	}
	
	return 0;	
}

unsigned long ted_fsize( char *path ){
	unsigned long size = 0;
	FILE *fp           = NULL;
	
	if( (fp = fopen( path, "rt" )) ){
		fseek( fp, 0, SEEK_END );
		size = ftell(fp);
		fclose(fp);
	}
	
	return size;	
}

unsigned int ted_last_lines( char *path, unsigned long offset, ted_linelist_t *lines ){
	FILE *fp          = NULL;
	char  line[0xFFF] = {0};
	
	lines->clear();
	if( (fp = fopen( path, "rt" )) ){
		fseek( fp, offset, SEEK_SET );
		while( fgets( line, 0xFFF, fp ) ){
			line[strlen(line) - 1] = 0x00;
			lines->push_back(line);
			memset( line, 0x00, 0xFFF );
		}
		fclose(fp);
		
		return lines->size();
	}
	
	return 0;
}

