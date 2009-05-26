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
#include "ted_log.h"

void ted_log( const char *format, ... ){
	FILE *fp;
	va_list arg_ptr ;
	time_t rawtime;
  	struct tm * timeinfo;
	char logfile[0xFF]  = {0},
	     datetime[0xFF] = {0};
	
	time(&rawtime);
  	timeinfo = localtime(&rawtime);
	
	strftime( logfile, 0xFF, "%Y%m%d.log", timeinfo );
		
	if( (fp = fopen( logfile, "a+t" )) ){
 		strftime( datetime, 0xFF, "[%x %X] ", timeinfo );
		fprintf( fp, datetime );
		
		va_start( arg_ptr, format );
			vfprintf( fp, format, arg_ptr );
		va_end( arg_ptr ); 	
		fclose(fp);
	}
}
