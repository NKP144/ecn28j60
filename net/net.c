/*
 * net.c
 *
 *  Created on: Sep 6, 2024
 *      Author: Konstantin
 */


#include "net.h"
#include "enc28j60.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

uint8_t net_buf[ENC28J60_MAXFRAME];
uint8_t ipaddr[4]=IP_ADDR;
uint32_t packet_counter = 0;

void net_init (void)
{
	enc28j60_init();
}

uint16_t checksum(uint8_t *ptr, uint16_t len)
{
	uint32_t sum = 0;
	while(len > 1)
	{
		sum += (uint16_t)(((uint32_t)*ptr<<8)|*(ptr+1));
		ptr += 2;
		len -= 2;
	}

	if (len)
		sum += ((uint32_t)*ptr) << 8;

	while (sum>>16)
		sum = (uint16_t)sum + (sum>>16);

	return ~SWAP_U16((uint16_t)sum);
}

uint8_t arp_read(eth_frame_ptr_t *frame, uint16_t len)
{
	uint8_t res=0;
	arp_msg_ptr_t *msg=(void*)(frame->data);

	if(len >= sizeof(arp_msg_ptr_t))
	{
	  if((msg->net_tp == ETH_TYPE_1) && (msg->proto_tp == ETH_IP))
	  {
	    if ((msg->op == ARP_REQUEST) && (!memcmp(msg->ipaddr_dst, ipaddr, 4)))
	    {
	    	printf(COLOR_BLUE);
	    	printf("\trequest\r\n\tmac_src %02X:%02X:%02X:%02X:%02X:%02X\r\n",
	    	  msg->macaddr_src[0],msg->macaddr_src[1],msg->macaddr_src[2],msg->macaddr_src[3],msg->macaddr_src[4],msg->macaddr_src[5]);

	    	printf("\tip_src %d.%d.%d.%d\r\n",
	    	    msg->ipaddr_src[0],msg->ipaddr_src[1],msg->ipaddr_src[2],msg->ipaddr_src[3]);

	    	printf("\tmac_dst %02X:%02X:%02X:%02X:%02X:%02X\r\n",
	    	    msg->macaddr_dst[0],msg->macaddr_dst[1],msg->macaddr_dst[2],msg->macaddr_dst[3],msg->macaddr_dst[4],msg->macaddr_dst[5]);

	    	printf("\tip_dst %d.%d.%d.%d\r\n",
	    	    msg->ipaddr_dst[0],msg->ipaddr_dst[1],msg->ipaddr_dst[2],msg->ipaddr_dst[3]);
	    	printf(COLOR_RESET);
	    	res = 1;
	    }
	  }
	}
	return res;
}

int eth_send(eth_frame_ptr_t *frame, uint16_t len)
{
	int result = 0;

	memcpy(frame->addr_dest, frame->addr_src, 6);
	memcpy(frame->addr_src, macaddr, 6);

	result = enc28j60_packet_send((void*)frame, len + sizeof(eth_frame_ptr_t));

	return result;
}

int arp_send(eth_frame_ptr_t *frame)
{
	int result = 0;
	arp_msg_ptr_t *arp_msg = (void*)frame->data;

	arp_msg->op = ARP_REPLY;
	memcpy(arp_msg->macaddr_dst, arp_msg->macaddr_src, 6);
	memcpy(arp_msg->macaddr_src, macaddr, 6);
	memcpy(arp_msg->ipaddr_dst, arp_msg->ipaddr_src, 4);
	memcpy(arp_msg->ipaddr_src, ipaddr, 4);

	printf(COLOR_CYAN);
	printf("\tresponse\r\n\tmac_src %02X:%02X:%02X:%02X:%02X:%02X\r\n",
		arp_msg->macaddr_src[0], arp_msg->macaddr_src[1], arp_msg->macaddr_src[2], arp_msg->macaddr_src[3], arp_msg->macaddr_src[4], arp_msg->macaddr_src[5]);

	printf("\tip_src %d.%d.%d.%d\r\n",
		arp_msg->ipaddr_src[0], arp_msg->ipaddr_src[1], arp_msg->ipaddr_src[2], arp_msg->ipaddr_src[3]);

	printf("\tmac_dst %02X:%02X:%02X:%02X:%02X:%02X\r\n",
			arp_msg->macaddr_dst[0], arp_msg->macaddr_dst[1], arp_msg->macaddr_dst[2], arp_msg->macaddr_dst[3], arp_msg->macaddr_dst[4], arp_msg->macaddr_dst[5]);

	printf("\tip_dst %d.%d.%d.%d\r\n",
			arp_msg->ipaddr_dst[0], arp_msg->ipaddr_dst[1], arp_msg->ipaddr_dst[2], arp_msg->ipaddr_dst[3]);
	printf(COLOR_RESET);

	result = eth_send(frame, sizeof(arp_msg_ptr_t));

	return result;
}

int ip_send(eth_frame_ptr_t *frame, uint16_t len)
{
	int result = 0;
	ip_pkt_ptr_t *ip_pkt_ptr = (void*)frame->data;

	//Заполним заголовок пакета IP
	ip_pkt_ptr->len = SWAP_U16(len);
	ip_pkt_ptr->fl_frg_of = 0;
	ip_pkt_ptr->ttl = 128;
	ip_pkt_ptr->cs = 0;
	memcpy(ip_pkt_ptr->ipaddr_dst, ip_pkt_ptr->ipaddr_src, 4);
	memcpy(ip_pkt_ptr->ipaddr_src, ipaddr, 4);
	ip_pkt_ptr->cs = checksum((void*)ip_pkt_ptr, sizeof(ip_pkt_ptr_t));

	//отправим фрейм
	result = eth_send(frame, len);

	return result;
}

int icmp_read(eth_frame_ptr_t *frame, uint16_t len)
{
	int result = 0;

	ip_pkt_ptr_t *ip_pkt_ptr = (void*)frame->data;
	icmp_pkt_ptr_t *icmp_pkt_ptr = (void*)ip_pkt_ptr->data;

	//Отфильтруем пакет по длине и типу сообщения - эхо-запрос
	if ((len >= sizeof(icmp_pkt_ptr_t)) && (icmp_pkt_ptr->msg_tp == ICMP_ECHO_REQ))
	{
		printf(COLOR_MAGENTA_I);
		printf("\ticmp echo request\r\n");
		printf(COLOR_RESET);

		printf(COLOR_CYAN);
		printf("\ticmp echo reply\r\n");
		printf(COLOR_RESET);

		icmp_pkt_ptr->msg_tp = ICMP_ECHO_REPLY;
		icmp_pkt_ptr->cs = 0;
		icmp_pkt_ptr->cs = checksum((void*)icmp_pkt_ptr, len);
		ip_send(frame, len + sizeof(ip_pkt_ptr_t));
	}

	return result;
}



uint8_t ip_read (eth_frame_ptr_t *frame, uint16_t len)
{
	uint8_t res = 0;
	ip_pkt_ptr_t *ip_pkt_ptr = (void*)(frame->data);

	if((ip_pkt_ptr->verlen == 0x45)&&(!memcmp(ip_pkt_ptr->ipaddr_dst, ipaddr, 4)))
	{
		printf(COLOR_GREEN);
		printf("\tIP confirm\r\n");
		printf(COLOR_RESET);
        //
		//len = SWAP_U16(ip_pkt_ptr->len) - sizeof(ip_pkt_ptr_t);
		//printf("\r\nip_cs 0x%04X\r\n", ip_pkt_ptr->cs);
		//ip_pkt_ptr->cs=0;
		//printf("ip_cs 0x%04X\r\n", checksum((void*)ip_pkt_ptr, sizeof(ip_pkt_ptr_t)));
		len = SWAP_U16(ip_pkt_ptr->len) - sizeof(ip_pkt_ptr_t);

		if (ip_pkt_ptr->prt==IP_ICMP)
		{
			icmp_read(frame, len);
		}
		else if (ip_pkt_ptr->prt==IP_TCP)
		{

		}
		else if (ip_pkt_ptr->prt==IP_UDP)
		{

		}

	}

	return res;
}

void eth_read (eth_frame_ptr_t *frame, uint16_t len)
{
	char *protocol_type = NULL;
	if (len >= sizeof(eth_frame_ptr_t))
	{
	  packet_counter++;
	  if (frame->type == ETH_ARP)
		  protocol_type = "arp";
	  else
		  protocol_type = "ip";

	  printf("%lu;\t%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X;\t%d;\t%s\r\n",
	    packet_counter,
		frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
		frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],
		len,
		protocol_type);

	  if (frame->type == ETH_ARP)
	  {
		if (arp_read(frame, len-sizeof(eth_frame_ptr_t)))
		{
			printf(COLOR_GREEN);
			printf("\tIP confirm\r\n");
			printf(COLOR_RESET);

			arp_send(frame);
		}
	  }
	  else if (frame->type == ETH_IP)
	  {
		  ip_read(frame, len-sizeof(ip_pkt_ptr_t));
	  }
	}
}



void net_pool(void)
{
	int result = 0;

	uint32_t len = 0;
	eth_frame_ptr_t *frame = (void*)net_buf;

	do {
		len = 0;
		result = enc28j60_packet_receive(net_buf, sizeof(net_buf), &len);
		eth_read(frame, len);
	} while (len > 0);

}
