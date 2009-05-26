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
#include "ted_arpmon.h"

extern void ted_log( const char *format, ... );

int ted_is_known_endpoint( ted_context_t *ted, unsigned char *ip, unsigned char *mac ){
	int i;
	for( i = 0; i < ted->endpoints.size(); i++ ){
		// TODO: check for mac address changes
		if( memcmp( &ted->endpoints[i]->ip, ip, 4 ) == 0 && memcmp( &ted->endpoints[i]->hw, mac, ETH_ALEN ) == 0 ){
			return 1;	
		}
	}	
	return 0;
}

void *ted_arpmon_recv_thread( void *arg ){
	ted_context_t *ted = (ted_context_t *)arg;
	struct bpf_program filter;
	unsigned char *packet;
	struct  pcap_pkthdr header;
	ted_arp_header_t *arp;

	if( pcap_compile( ted->pd, &filter, "arp", 0, ted->network ) < 0 ){
		ted_die( "Error compiling 'arp' pcap filter .\n" );
	}
	pcap_setfilter( ted->pd, &filter );
	
	ted_log( "Listening for new hosts on this network ...\n" );

	while( (packet = (unsigned char *)pcap_next( ted->pd, &header )) ){
		packet     += ted->head_shift;
		header.len -= ted->head_shift;
		arp         = (ted_arp_header_t *)packet;
		
		if( ntohs(arp->ar_op) == ARPOP_REPLY ){
			// new machine detected on the network ?
			if( ted_is_known_endpoint( ted, arp->ar_spa, arp->ar_sha ) == 0 ){
				ted_endpoint_t *endpoint = new ted_endpoint_t( arp->ar_spa, arp->ar_sha );
				ted->endpoints.push_back(endpoint);
				ted->endpoint = endpoint;
				ted_event_notification( TED_EVENT_NEW_HOST, ted );
			}
		}
	}

}


void *ted_arpmon_thread( void *arg ){
	ted_context_t *ted = (ted_context_t *)arg;
	struct libnet_ether_addr *if_mac;
	struct in_addr            if_ip;
	libnet_t                 *nd;
	char                      n_error[LIBNET_ERRBUF_SIZE];
	char                      p_error[PCAP_ERRBUF_SIZE];
	unsigned int			  nhosts,
	        				  i,
							  ip;
	libnet_ptag_t net = LIBNET_PTAG_INITIALIZER,
	              arp = LIBNET_PTAG_INITIALIZER;
	unsigned char unknown_hw[ETH_ALEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		   		  bcast_hw[ETH_ALEN]   = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	pthread_t tid;
		
	if( (nd = libnet_init( LIBNET_LINK, ted->device, n_error )) == NULL ){
		ted_die( "Error initializing libnet layer (%s) .\n", n_error );
	}

	if( (if_mac = libnet_get_hwaddr( nd )) == NULL ){
		ted_die( "Error retrieving %s MAC address .\n", ted->device );
	}

	if( (if_ip.s_addr = libnet_get_ipaddr4( nd )) < 0 ){
		ted_die( "Error retrieving %s bound ip address .\n", ted->device );
	}

	ted->pd = pcap_open_live( ted->device, 65535, 1, 1000, p_error );

	if( (ted->datalink = pcap_datalink(ted->pd)) < 0 ){
        ted_die( "Error retrieving %s data link layer (%s) .\n", ted->device, p_error );
	}
	
    switch( ted->datalink )
    {
        case DLT_RAW        : ted->head_shift = 0;  break;
        case DLT_PPP        :
        case DLT_LOOP       :
        case DLT_NULL       : ted->head_shift = 4;  break;
        case DLT_PPP_ETHER  : ted->head_shift = 8;  break;
        case DLT_EN10MB     :
        case DLT_EN3MB      : ted->head_shift = 14; break;
        case DLT_LINUX_SLL  :
        case DLT_SLIP       : ted->head_shift = 16; break;
        case DLT_SLIP_BSDOS :
        case DLT_PPP_BSDOS  :
        case DLT_IEEE802_11 : ted->head_shift = 24; break; 
        case DLT_PFLOG      : ted->head_shift = 48; break;
        
        default             : ted_die( "Device datalink not supported .\n" );
    }

	pcap_lookupnet( ted->device, &ted->network, &ted->netmask, p_error );
	
	if( pthread_create( &tid, NULL, ted_arpmon_recv_thread, (void *)ted ) != 0 ){
		ted_die( "Could not create alive hosts reciever thread .\n" );
	}
	
	nhosts = ntohl(~ted->netmask);
	
	while( 1 ){
		for( i = 1; i <= nhosts; i++ ){
			ip = (if_ip.s_addr & ted->netmask) | htonl(i);
			// don't send arp request to ourself :P
			if( ip != if_ip.s_addr ){
				arp = libnet_build_arp( ARPHRD_ETHER,
										ETHERTYPE_IP,
										ETH_ALEN,
										4,
										ARPOP_REQUEST,
										// sender
										if_mac->ether_addr_octet,
										(unsigned char *)&if_ip,
										// reciever
										unknown_hw,
										(unsigned char *)&ip,
										NULL,
										0,
										nd,
										arp );

				net = libnet_build_ethernet( bcast_hw,
											 if_mac->ether_addr_octet,
											 ETHERTYPE_ARP, 
											 NULL, 
											 0, 
											 nd, 
											 net );

				if( libnet_write(nd) < 0 ){
					ted_die( "Could not send ARP request .\n" );
				}
			}
		}
	   			
		usleep( ted->arp_delay );
	}
}

void ted_start_arpmon( ted_context_t *ted ){
	pthread_t tid;
	
	if( pthread_create( &tid, NULL, ted_arpmon_thread, (void *)ted ) != 0 ){
		ted_die( "Could not create alive hosts monitoring thread .\n" );
	}
}

