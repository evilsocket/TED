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
#include "ted_common.h"

extern unsigned short ted_isfile( char *path );
extern void ted_log( const char *format, ... );

void ted_die( const char *format, ... ){
	va_list arg_ptr ;

	printf( "[ERROR] " );
	
	va_start( arg_ptr, format );
		vfprintf( stdout, format, arg_ptr );
	va_end( arg_ptr ); 

    exit(1);
}

void ted_get_syslog( char *syslog ){
	if( ted_isfile( "/var/log/messages" ) ){
		strcpy( syslog, "/var/log/messages" );
	}
	else if( ted_isfile( "/var/log/kernel" ) ){
		strcpy( syslog, "/var/log/kernel" );
	}
	else{
		ted_die( "Could not find any system log file .\n" );	
	}
}

void ted_init( ted_context_t *ted ){
	ted_get_syslog( ted->syslog );
	ted->device     = strdup("eth0");
	ted->poll_delay = 500 * 1000;	
	ted->arp_delay  = 5000 * 1000;
	ted->verbose    = 1;
	ted->notify     = 1;
	ted->notification_time = 5000;
	ted->connection = NULL;
}

void ted_event_notification( unsigned short event, void *args ){
	ted_context_t      *ted = (ted_context_t *)args;
	NotifyNotification *n = NULL;  
	char                message[0xFF] = {0};
	
	if( ted->notify ){
    	notify_init("TED");
	}

	switch( event ){
		case TED_EVENT_CONNECTION :
			// TODO: Check last connection event and skip if equal to prevent DOS attacks
			sprintf( message,
					"New connection from %s to port %s .", 
					 ted->connection->source,
					 ted->connection->port );
			
			ted_log( "%s\n", message );
			
			if( ted->verbose ){
				printf( "%s\n", message );
			}
			
			if( ted->notify ){
				n = notify_notification_new( "Network Event", 
											 message,
											 // TODO: Add an icon 
											 NULL, 
											 NULL );
											 
				notify_notification_set_timeout( n, ted->notification_time );
				notify_notification_show( n, NULL );
			}
		break;
		
		case TED_EVENT_NEW_HOST :
			sprintf( message,
					"New host detected : %s [%s] .", 
					 ted->endpoint->address(),
					 ted->endpoint->hardware() );
			
			ted_log( "%s\n", message );
			
			if( ted->verbose ){
				printf( "%s\n", message );
			}
			
			if( ted->notify ){
				n = notify_notification_new( "Network Event", 
											 message,
											 // TODO: Add an icon 
											 NULL, 
											 NULL );
											 
				notify_notification_set_timeout( n, ted->notification_time );
				notify_notification_show( n, NULL );
			}
		break;
	}
	
	if( ted->notify && n ){
		g_object_unref(G_OBJECT(n));	
	}
}
