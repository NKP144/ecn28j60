/*
 * net.h
 *
 *  Created on: Sep 6, 2024
 *      Author: Konstantin
 */

#ifndef NET_7A894118535E655F_H_
#define NET_7A894118535E655F_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Symbol color (foreground)
#define COLOR_RESET      "\033[0m"
#define COLOR_BLACK      "\033[30m"
#define COLOR_RED        "\033[31m"
#define COLOR_GREEN      "\033[32m"
#define COLOR_YELLOW     "\033[33m"
#define COLOR_BLUE       "\033[34m"
#define COLOR_MAGENTA    "\033[35m"
#define COLOR_CYAN       "\033[36m"
#define COLOR_GRAY       "\033[37m"
#define COLOR_DARK_GRAY  "\033[90m"
#define COLOR_RED_I      "\033[91m"
#define COLOR_GREEN_I    "\033[92m"
#define COLOR_YELLOW_I   "\033[93m"
#define COLOR_BLUE_I     "\033[94m"
#define COLOR_MAGENTA_I  "\033[95m"
#define COLOR_CYAN_I     "\033[96m"
#define COLOR_WHITE      "\033[97m"
// Background Color
#define COLOR_BG_BLACK      "\033[40m"
#define COLOR_BG_RED        "\033[41m"
#define COLOR_BG_GREEN      "\033[42m"
#define COLOR_BG_YELLOW     "\033[43m"
#define COLOR_BG_BLUE       "\033[44m"
#define COLOR_BG_MAGENTA    "\033[45m"
#define COLOR_BG_CYAN       "\033[46m"
#define COLOR_BG_GRAY       "\033[47m"
#define COLOR_BG_DARK_GRAY  "\033[100m"
#define COLOR_BG_RED_I      "\033[101m"
#define COLOR_BG_GREEN_I    "\033[102m"
#define COLOR_BG_YELLOW_I   "\033[103m"
#define COLOR_BG_BLUE_I     "\033[104m"
#define COLOR_BG_MAGENTA_I  "\033[105m"
#define COLOR_BG_CYAN_I     "\033[106m"
#define COLOR_BG_WHITE      "\033[107m"

#define SWAP_U16( a ) ((((uint16_t)(a) & 0x00FF) << 8) | (((uint16_t)(a) & 0xFF00) >> 8))

#define ETH_ARP 		SWAP_U16(0x0806)
#define ETH_IP 			SWAP_U16(0x0800)
#define ETH_TYPE_1 	    SWAP_U16(1)
#define ARP_REQUEST 	SWAP_U16(1)
#define ARP_REPLY 		SWAP_U16(2)
#define IP_ADDR 		{192,168,1,197}

#define IP_ICMP 	1
#define IP_TCP 		6
#define IP_UDP 		17

#define ICMP_ECHO_REQ 		8
#define ICMP_ECHO_REPLY 	0

typedef struct eth_frame_s{
  uint8_t 	addr_dest[6];
  uint8_t	addr_src[6];
  uint16_t 	type;
  uint8_t 	data[];
} eth_frame_ptr_t;

typedef struct arp_msg_s{
  uint16_t 	net_tp;
  uint16_t 	proto_tp;
  uint8_t	macaddr_len;
  uint8_t 	ipaddr_len;
  uint16_t	op;
  uint8_t	macaddr_src[6];
  uint8_t	ipaddr_src[4];
  uint8_t	macaddr_dst[6];
  uint8_t	ipaddr_dst[4];
} arp_msg_ptr_t;

typedef struct ip_pkt_s{
	uint8_t 	verlen;			//версия протокола и длина заголовка
	uint8_t 	ts;				//тип севриса
	uint16_t 	len;			//длина
	uint16_t 	id;				//идентификатор пакета
	uint16_t 	fl_frg_of;		//флаги и смещение фрагмента
	uint8_t 	ttl;			//время жизни
	uint8_t 	prt;			//тип протокола
	uint16_t 	cs;				//контрольная сумма заголовка
	uint8_t 	ipaddr_src[4];	//IP-адрес отправителя
	uint8_t 	ipaddr_dst[4];	//IP-адрес получателя
	uint8_t 	data[];			//данные
} ip_pkt_ptr_t;

typedef struct icmp_pkt_s{
	uint8_t 	msg_tp;	//тип севриса
	uint8_t 	msg_cd;	//код сообщения
	uint16_t 	cs;		//контрольная сумма заголовка
	uint16_t 	id;		//идентификатор пакета
	uint16_t 	num;	//номер пакета
	uint8_t 	data[];	//данные
} icmp_pkt_ptr_t;

void net_init (void);
void net_pool(void);


#ifdef __cplusplus
}
#endif

#endif /* NET_H_ */
