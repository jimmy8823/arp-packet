#include "arp.h"

#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <linux/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//You can fill the following functions or add other functions if needed. If not, you needn't write anything in them.  
void set_hard_type(struct ether_arp *packet, unsigned short int type)
{
	packet->arp_hrd = htons(type);
}
void set_prot_type(struct ether_arp *packet, unsigned short int type)
{
	packet->arp_pro = htons(type);
}
void set_hard_size(struct ether_arp *packet, unsigned char size)
{
	packet->arp_hln = size;
}
void set_prot_size(struct ether_arp *packet, unsigned char size)
{
	packet->arp_pln = size;
}
void set_op_code(struct ether_arp *packet, short int code)
{
	packet->arp_op = htons(code);
}
void set_sender_hardware_addr(struct ether_arp *packet, char *address)
{
	memcpy(packet->arp_sha , address, ETH_ALEN);
}
void set_sender_protocol_addr(struct ether_arp *packet, struct in_addr address)
{
	//struct in_addr myip;
	//inet_pton(AF_INET,address,myip);
	memcpy(packet->arp_spa , &address ,4);
}
void set_target_hardware_addr(struct ether_arp *packet, char *address)
{
	memcpy(packet->arp_tha , address, ETH_ALEN);
}
void set_target_protocol_addr(struct ether_arp *packet, struct in_addr address)
{
	//struct in_addr dst_ip;
	//inet_pton(AF_INET,address,dst_ip);
	memcpy(packet->arp_tpa , &address, 4);
}

char* get_target_protocol_addr(struct ether_arp *packet)
{
	char *tmp = malloc(16);
	sprintf(tmp,"%d.%d.%d.%d",
		packet->arp_tpa[0],
		packet->arp_tpa[1],
		packet->arp_tpa[2],
		packet->arp_tpa[3]);
	return tmp;
	// if you use malloc, remember to free it.
}
char* get_sender_protocol_addr(struct ether_arp *packet)
{
	char *tmp = malloc(16);
	sprintf(tmp,"%d.%d.%d.%d",
		packet->arp_spa[0],
		packet->arp_spa[1],
		packet->arp_spa[2],
		packet->arp_spa[3]);
	return tmp;
	// if you use malloc, remember to free it.
}
char* get_sender_hardware_addr(struct ether_arp *packet)
{
	char *tmp = malloc(18);
	sprintf(tmp,"%x.%x.%x.%x.%x.%x",
		packet->arp_sha[0],
		packet->arp_sha[1],
		packet->arp_sha[2],
		packet->arp_sha[3],
		packet->arp_sha[4],
		packet->arp_sha[5]);
	return tmp;
	// if you use malloc, remember to free it.
}
char* get_target_hardware_addr(struct ether_arp *packet)
{
	char *tmp = malloc(18);
	sprintf(tmp,"%x.%x.%x.%x.%x.%x",
		packet->arp_tha[0],
		packet->arp_tha[1],
		packet->arp_tha[2],
		packet->arp_tha[3],
		packet->arp_tha[4],
		packet->arp_tha[5]);
	return tmp;
	// if you use malloc, remember to free it.
}

void print_usage()
{
	printf("./arp -l -a\n");
	printf("./arp -l <filter_ip_address>\n");
	printf("./arp -l <query_ip_address>\n");
	printf("./arp <fake_mac_address> <target_ip_address>\n");
}

void filter_string(char *str ,char remove)
{
	int j=0;
	for(int i=0;str[i]!='\0';i++){
		if(str[i]!=remove){
			str[j++]=str[i];
		}
		str[j]='\0';
	}
}

uint8_t* convert(char *str)
{
	static uint8_t ip_addr[4];
	int tmp=0,digit=1,seg=3;
	for(int i=strlen(str)-1;i>=0;i--){
		if(str[i]!='.'){
			tmp+= ((str[i]-'0')*digit);
			digit*=10;
			if(digit>1000)break;
		}else{
			ip_addr[seg] = tmp;
			digit= 1;
			tmp=0;
			seg--;
		}
	}
	ip_addr[seg] = tmp;
	//printf("%d.%d.%d.%d\n",ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
	return ip_addr;
}
void print_ip_addr(uint8_t *ip_addr){
	for(int i=0;i<4;i++){
		if(i==3)
			printf("%u",ip_addr[i]);
		else
			printf("%u.",ip_addr[i]);
	}

}
