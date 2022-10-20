#ifndef __ARP_UTIL_H__
#define __ARP_UTIL_H__

#include <netinet/if_ether.h>
#include <netinet/in.h>

struct arp_packet
{
	struct ether_header eth_hdr;
	struct ether_arp arp;
};

void print_usage();

void set_hard_type(struct ether_arp *packet, unsigned short int type);
void set_prot_type(struct ether_arp *packet, unsigned short int type);
void set_hard_size(struct ether_arp *packet, unsigned char size);
void set_prot_size(struct ether_arp *packet, unsigned char size);
void set_op_code(struct ether_arp *packet,unsigned short int code);

void set_sender_hardware_addr(struct ether_arp *packet, char *address);
void set_sender_protocol_addr(struct ether_arp *packet, char *address);
void set_target_hardware_addr(struct ether_arp *packet, char *address);
void set_target_protocol_addr(struct ether_arp *packet, char *address);

char* get_target_protocol_addr(struct ether_arp *packet); 
char* get_sender_protocol_addr(struct ether_arp *packet); 
char* get_sender_hardware_addr(struct ether_arp *packet); 
char* get_target_hardware_addr(struct ether_arp *packet); 

void filter_string(char *str,char remove);
uint8_t* convert(char *str);
void print_ip_addr(uint8_t *ip_addr);
#endif
