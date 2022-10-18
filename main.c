#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
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
#define DEVICE_NAME "enp0s3"

/*
 * You have to open two socket to handle this program.
 * One for input , the other for output.
 */

int main(int argc, char* argv[])
{
	int sockfd_recv = 0, sockfd_send = 0;
	int op = 0,listen=0,send=0,all=0;
	uint8_t *query_target;
	uint8_t *filter_ip_addr;
	char *fake_mac_addr = NULL;
	char *target_ip_addr = NULL;
	char *dst_ip;
	unsigned char src_mac[ETH_ALEN];
	unsigned char dst_mac[ETH_ALEN];
	unsigned char broadcast_mac[ETH_ALEN]={0xff,0xff,0xff,0xff,0xff,0xff};
	struct sockaddr_ll sa;
	struct ifreq req;
	struct in_addr myip;
	while(1)
	{
		op = getopt(argc,argv,"q:l:h::");
		if(op == -1)break;
		switch(op){
			case 'q': //send arp request
				query_target = convert(optarg);
				send = 1;
				printf("opening a send socket query_target:");
				print_ip_addr(query_target);
				printf("\n");
				break;
			case 'l': //listen packets
				listen = 1;
				printf("opening a recv socket optarg =%s\n",optarg);
				if(strcmp(optarg,"-a")==0){
					//printf("listen all");
					all = 1;
				}else{
					//printf("filter_ip_addr :");
					filter_ip_addr = convert(optarg);
					print_ip_addr(filter_ip_addr);
					printf("\n");
					//strcpy(filter_ip_addr,optarg);
				}
				break;
			case 'h':
				if(strcmp("elp" , optarg)==0){
					print_usage();
				}
				break;
			case '?':
				send = 1;
				if(fake_mac_addr==NULL){
					//fake_mac_addr = malloc(6);
					strcpy(fake_mac_addr,optarg);
				}else if(target_ip_addr==NULL){
					target_ip_addr = malloc(4);
					strcpy(target_ip_addr,optarg);
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
			arp_p = buffer + Ether_Hdr_Len; //point to arp
			//printf("Ethernet type : 0x%x  a = %d\n",ntohs(header->ether_type),all);
			if(ntohs(header->ether_type)==0x806){// 0806 is represent ARP 
				for(int i=0;i<IP_ADDR_LEN;i++){
					printf("%u.",arp_p->arp_tpa[i]);
				}
				if(all==1){ // capture all arp packet
					printf("Get ARP packet - who has ");
					print_ip_addr(arp_p->arp_tpa);
					printf("          Tell ");
					print_ip_addr(arp_p->arp_spa);
					printf("\n");
				}else{ // capture specific IP arp packet
					int src_hit = 1,dst_hit = 1;
					for(int j=0;j<IP_ADDR_LEN;j++){
						if(filter_ip_addr[j]!=arp_p->arp_spa[j]){
							src_hit = 0;
						}
						if(filter_ip_addr[j]!=arp_p->arp_tpa[j]){
							dst_hit = 0;
						}
					}
					if(src_hit==1||dst_hit==1)
					{
						printf("Get ARP packet - who has ");
						print_ip_addr(arp_p->arp_tpa);
						printf("          Tell ");
						print_ip_addr(arp_p->arp_spa);
						printf("\n");
					}
				}
			}
		}
		close(sockfd_recv);
	}
		
	if(send == 1){
		struct ether_header *header;
		struct ether_arp *arp_p;
		char packet[Ether_Arp_Len];
		// Open a send socket in data-link layer.
		if((sockfd_send = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
		{
			perror("open send socket error\n");
			exit(sockfd_send);
		}
		/*
		* Use ioctl function binds the send socket and the Network Interface Card.
	`	 * ioctl( ... )
		*/
		memset(req,0,sizeof(req));
		memcpy(req.ifr_name, DEVICE_NAME,strlen(DEVICE_NAME));
		if(ioctl(sockfd_send,SIOGIFINDEX,&req) < 0){//get interface index and store in req
			perror("get ifindex failed\n");
		}
		if(ioctl(sockfd_send,SIOCGIFADDR,&req) < 0){//get interface ip and store in req
			perror("get ip failed\n");
		}
		if(ioctl(sockfd_send,SIOCGIFHWADDR,&req) < 0){//get interface MAC and store in req
			perror("get MAC failed\n");
		}

		// Fill the parameters of the sa.
		sa.sll_family = PF_PACKET;
		sa.sll_ifindex = req.ifr_ifindex;
		myip = ((struct sockaddr_in *)&(req.ifr_addr))->sin_addr;
		char *tmp = inet_ntoa(myip);
		printf("%s\n",tmp);
		memcpy(src_mac,req.ifr_hwaddr.sa_data,ETH_ALEN);
		header = (struct  ether_header *)packet;
		memcpy(header->ether_shost,src_mac,ETH_ALEN);
		memcpy(header->ether_dhost,broadcast_mac,ETH_ALEN);
		header->ether_type = htons(ETHERTYPE_ARP);
		
		arp_p = (struct ether_arp *)malloc(Ether_Arp_Len);
		set_hard_type(arp_p,ARPHRD_ETHER);
		set_hard_size(arp_p,ETH_ALEN);
		set_op_code(arp_p,ARPOP_REQUEST);
		set_prot_size(arp_p,IP_ADDR_LEN);
		set_prot_type(arp_p,ETHERTYPE_IP);
		set_sender_hardware_addr(arp_p,src_mac);
		//set_sender_protocol_addr(arp_p,myip);
		//set_target_protocol_addr(arp_p,dst_ip);
		set_target_hardware_addr(arp_p,broadcast_mac);
		//memcpy(packet + ETHER_HDR_LEN,arp_p,Ether_Arp_Len);
		/*
		* use sendto function with sa variable to send your packet out
		* sendto( ... )
		*/
		/*int result = sendto(sockfd_send,packet,Packet_Len,0,&sa,sizeof(sa));
		if(result > 0){
			printf("send ARP success\n");
		}*/
		close(sockfd_send);
	}
	
	return 0;
}

