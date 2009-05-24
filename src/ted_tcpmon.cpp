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
#include "ted_tcpmon.h"

extern unsigned long  ted_fsize( char *path );
extern unsigned int   ted_last_lines( char *path, unsigned long offset, ted_linelist_t *lines );

int ted_is_connection( const char *line ){
	regex_t crex;
	int     ret = 0;
		
	if( !regcomp( &crex, ".+kernel:.+Inbound IN=.* OUT=.* MAC=.* SRC=.+ DST=.*", REG_EXTENDED|REG_ICASE|REG_NOSUB|REG_NEWLINE ) ){
		regmatch_t match[1];
		ret = ( regexec( &crex, line, 1, match, 0) == 0 ? 1 : 0 );
		regfree(&crex);
	}

	return ret;
}

void ted_get_between( const char *buffer, char *a, char *b, char *dest ){
	char *p = (a ? strstr( buffer, a ) + strlen(a) : (char *)buffer);
	
	strncpy( dest, p, strstr( p, b ) - p );	
}

void ted_parse_connection( const char *line, ted_connection_t *c ){
	memset( c, 0x00, sizeof(ted_connection_t) );
	
	strncpy( c->datetime, line, 15 );
	ted_get_between( line, "PROTO=", " ", c->protocol );
	ted_get_between( line, "SRC=",   " ", c->source );
	ted_get_between( line, "DPT=",   " ", c->port );
}

void *ted_tcpmon_thread( void *arg ){
	ted_context_t *ted = (ted_context_t *)arg;
	unsigned long  last_known_size,
				   size;
	int            i;
	ted_linelist_t lines;
	
	if( ted->verbose ){
		printf( "* Connection monitoring thread started ...\n" );	
	}
	
	last_known_size = ted_fsize( ted->syslog );
	
	while( 1 ){
		if( (size = ted_fsize(ted->syslog)) > last_known_size ){
			ted_last_lines( ted->syslog, last_known_size, &lines );
			for( i = 0; i < lines.size(); i++ ){
				if( ted_is_connection( lines[i].c_str() ) ){
					ted_connection_t connection;
					ted_parse_connection( lines[i].c_str(), &connection );
					ted_event_notification( TED_EVENT_CONNECTION, &connection );
				}	
			}
		}
		
		last_known_size = size;
		
		usleep( ted->poll_delay );	
	}	
}

void ted_start_tcpmon( ted_context_t *ted ){
	pthread_t tid;
	
	if( pthread_create( &tid, NULL, ted_tcpmon_thread, (void *)ted ) != 0 ){
		ted_die( "Could not create connection monitoring thread .\n" );
	}
}
