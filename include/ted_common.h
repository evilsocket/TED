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
#ifndef _TED_COMMON_H_
#	define _TED_COMMON_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <regex.h>
#include <vector>
#include <string>
#include <libnotify/notify.h>

using std::vector;
using std::string;

#define TED_EVENT_CONNECTION 0x00
#define TED_EVENT_NEW_HOST   0x01
#define TED_EVENT_HOST_DOWN  0x02

typedef vector<string> ted_linelist_t;

typedef struct {
	unsigned short verbose;
	char           syslog[0xFF];
	unsigned int   poll_delay;
}
ted_context_t;

typedef struct {
	char datetime[0xFF];
	char protocol[0xFF];
	char source[0xFF];
	char port[0xFF];
}
ted_connection_t;

void ted_die( const char *format, ... );
void ted_init( ted_context_t *ted );

void ted_event_notification( unsigned short event, void *args );

#endif


