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
#include <pcap.h>
#include <libnet.h>
#include <pthread.h>
#include <netinet/ether.h>

#include <libnotify/notify.h>

using std::vector;
using std::string;

#define TED_EVENT_CONNECTION 0x00
#define TED_EVENT_NEW_HOST   0x01
#define TED_EVENT_HOST_DOWN  0x02

typedef vector<string> ted_linelist_t;

typedef struct {
	unsigned short ar_hrd; 
	unsigned short ar_pro;
	unsigned char  ar_hln;
	unsigned char  ar_pln;
	unsigned short ar_op;
    unsigned char  ar_sha[ETH_ALEN];
    unsigned char  ar_spa[4];
    unsigned char  ar_tha[ETH_ALEN];
    unsigned char  ar_tpa[4];
}
ted_arp_header_t;

typedef struct _endpoint_t {
	struct in_addr ip;
	unsigned char  hw[ETH_ALEN];

	_endpoint_t( unsigned char *address, unsigned char *mac ){
		memcpy( &this->ip.s_addr, address, 4 );
		memcpy( this->hw, mac, ETH_ALEN );	
	}
	
	char *address(){
		return inet_ntoa(this->ip);	
	}
	
	char *hardware(){
		char shw[0xFF] = {0};
		
		sprintf( shw,
				 "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
				 this->hw[0],
				 this->hw[1],
				 this->hw[2],
				 this->hw[3],
				 this->hw[4],
				 this->hw[5] );
				 
		return strdup(shw);	
	}
	
	char *resolve(){
		struct hostent *host = gethostbyaddr( &this->ip, 4, AF_INET );
		return (host->h_name ? strdup(host->h_name) : this->address());
	}
}	
ted_endpoint_t;

typedef vector<ted_endpoint_t *> ted_endpoint_list_t;

typedef struct {
	char datetime[0xFF];
	char protocol[0xFF];
	char source[0xFF];
	char port[0xFF];
}
ted_connection_t;

typedef struct {
	char          	   *device;
	bpf_u_int32    		network;
	bpf_u_int32    		netmask;
	unsigned short 		datalink;
	unsigned short 		head_shift;   
	pcap_t              *pd;   
	ted_endpoint_list_t endpoints;
	unsigned int        arp_delay;

	unsigned short verbose;
	unsigned short notify;
	unsigned int   notification_time;
	char           syslog[0xFF];
	unsigned int   poll_delay;
	
	/* events data */
	ted_connection_t *connection;
	ted_endpoint_t   *endpoint;
}
ted_context_t;



void ted_die( const char *format, ... );
void ted_init( ted_context_t *ted );

void ted_event_notification( unsigned short event, void *args );

#endif


