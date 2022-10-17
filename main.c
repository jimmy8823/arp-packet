#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include "arp.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define Ether_Hdr_Len sizeof(struct ether_header)
#define Ether_Arp_Len sizeof(struct ether_arp)
#define Packet_Len ETH_FRAME_LEN + Ether_Arp_Len
#define IP_ADDR_LEN 4
/* 
 * Change "enp2s0f5" to your device name (e.g. "eth0"), when you test your hoework.
 * If you don't know your device name, you can use "ifconfig" command on Linux.
 * You have to use "enp2s0f5" when you ready to upload your homework.
 */
#define DEVICE_NAME "enp2s0f5"

/*
 * You have to open two socket to handle this program.
 * One for input , the other for output.
 */

int main(int argc, char* argv[])
{
	int sockfd_recv = 0, sockfd_send = 0;
	int op = 0,listen=0,send=0,all=0;
	char *query_target;
	char *fake_mac_addr = NULL;
	char *target_ip_addr = NULL;
	struct sockaddr_ll sa;
	struct ifreq req;
	//struct in_addr myip;
	while(1)
	{
		op = getopt(argc,argv,"q:l:h::a");
		if(op == -1)break;
		switch(op){
			case 'q': //send arp request
				query_target = optarg;
				send = 1;
				printf("opening a send socket\n");
				break;
			case 'l': //listen packets
				listen = 1;
				printf("opening a recv socket\n");
				break;
			case 'h':
				if(strcmp("elp" , optarg)==0){
					print_usage();
				}
				break;
			case 'a':
				all=1;
				break;
			case '?':
				send = 1;
				if(fake_mac_addr==NULL){
					fake_mac_addr = optarg;
				}else{
					target_ip_addr = optarg;
				}
				printf("opening a send socket\n");
				break;
			
		}
	}
	
	if(listen == 1){
		char buffer[Packet_Len];
		struct sockaddr from;
		struct ether_header *header;
		struct ether_arp *arp_p;
		// Open a recv socket in data-link layer.
		if((sockfd_recv = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
		{
			perror("open recv socket error");
			exit(1);
		}
		/*
		 * Use recvfrom function to get packet.
		 * recvfrom( ... )
		 */
		while(1){
			memset(buffer,'\0',Packet_Len);
			recvfrom(sockfd_recv,buffer,Packet_Len,0,&from,sizeof(from)); //get packet and store in buffer
			header = buffer; //point to ethernet header
			arp_p = buffer + Ether_Hdr_Len; //point to arp header
			printf("Ethernet type : 0x%u \n",header->ether_type);
			/*if(header->ether_type==0x0806){// 0806 is represent ARP 
				if(all==1){
					
				}
			}*/
		}

	}
		
	if(send == 1){
		// Open a send socket in data-link layer.
		if((sockfd_send = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
		{
			perror("open send socket error");
			exit(sockfd_send);
		}
		
		/*
		* Use ioctl function binds the send socket and the Network Interface Card.
	`	 * ioctl( ... )
		*/
		
		

		
		// Fill the parameters of the sa.



		
		/*
		* use sendto function with sa variable to send your packet out
		* sendto( ... )
		*/
		
		
	}
	
	


	return 0;
}

