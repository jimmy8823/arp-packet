#include "arp.h"

#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <linux/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//You can fill the following functions or add other functions if needed. If not, you needn't write anything in them.  
void set_hard_type(struct ether_arp *packet, unsigned short int type)
{
	packet->arp_hrd = type;
}
void set_prot_type(struct ether_arp *packet, unsigned short int type)
{
	packet->arp_pro = type;
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
	packet->arp_op = code;
}

void set_sender_hardware_addr(struct ether_arp *packet, char *address)
{
	memcpy(packet->arp_sha , address, ETH_ALEN);
}
void set_sender_protocol_addr(struct ether_arp *packet, char *address)
{
	memcpy(packet->arp_spa , address, ETH_ALEN);
}
void set_target_hardware_addr(struct ether_arp *packet, char *address)
{
	memcpy(packet->arp_tha , address, ETH_ALEN);
}
void set_target_protocol_addr(struct ether_arp *packet, char *address)
{
	memcpy(packet->arp_tpa , address, ETH_ALEN);
}

char* get_target_protocol_addr(struct ether_arp *packet)
{

	// if you use malloc, remember to free it.
}
char* get_sender_protocol_addr(struct ether_arp *packet)
{
	// if you use malloc, remember to free it.
}
char* get_sender_hardware_addr(struct ether_arp *packet)
{
	// if you use malloc, remember to free it.
}
char* get_target_hardware_addr(struct ether_arp *packet)
{
	// if you use malloc, remember to free it.
}

void print_usage()
{
	printf("./arp -l -a\n");
	printf("./arp -l <filter_ip_address>\n");
	printf("./arp -l <query_ip_address>\n");
	printf("./arp <fake_mac_address> <target_ip_address>\n");
}
