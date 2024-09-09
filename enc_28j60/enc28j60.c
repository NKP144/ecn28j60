/*
 * enc28j60.c
 *
 *  Created on: Sep 6, 2024
 *      Author: Konstantin
 */

#include "enc28j60.h"

static uint8_t enc28j60_BankNum;
static int enc28j60_gNextPacketPtr;

uint8_t macaddr[6] = MAC_ADDR;

void enc28j60_init(void)
{
	int result = 0;
	enc28j60_reset();
	result = enc28j60_soft_reset();

	// Init buffers
	result = enc28j60_write_reg(ERXST,RXSTART_INIT);
	result = enc28j60_write_reg(ERXRDPT,RXSTART_INIT);
	result = enc28j60_write_reg(ERXND,RXSTOP_INIT);
	result = enc28j60_write_reg(ETXST,TXSTART_INIT);
	result = enc28j60_write_reg(ETXND,TXSTOP_INIT);

	// Init channel level
	result = enc28j60_write_reg_byte(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	result = enc28j60_write_reg_byte(MACON2, 0x00);
	result = enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	result = enc28j60_write_reg(MAIPG, 0x0C12);
	result = enc28j60_write_reg_byte(MABBIPG, 0x12);		//промежуток между фреймами
	result = enc28j60_write_reg(MAMXFL, MAX_FRAMELEN);		//максимальный размер фрейма
	result = enc28j60_write_reg_byte(MAADR5, macaddr[0]);	//Set MAC addres
	result = enc28j60_write_reg_byte(MAADR4, macaddr[1]);
	result = enc28j60_write_reg_byte(MAADR3, macaddr[2]);
	result = enc28j60_write_reg_byte(MAADR2, macaddr[3]);
	result = enc28j60_write_reg_byte(MAADR1, macaddr[4]);
	result = enc28j60_write_reg_byte(MAADR0, macaddr[5]);

	//Init pfy level
	result = enc28j60_write_Phy(PHCON2, PHCON2_HDLDIS);  		//отключаем loopback
	result = enc28j60_write_Phy(PHLCON, PHLCON_LACFG2|		 	//светодиоды
							   PHLCON_LBCFG2|
							   PHLCON_LBCFG1|
							   PHLCON_LBCFG0|
							   PHLCON_LFRQ0|
							   PHLCON_STRCH);

	result = enc28j60_set_bank (ECON1);
	result = enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
	result = enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);			//разрешаем приём пакетов

	enc28j60_gNextPacketPtr = 0;
}

int enc28j60_writeOp(uint8_t op, uint8_t addres, uint8_t data)
{
	int result = 0;
	enc28j60_cs_low();

	uint8_t send_data = op | (addres & ADDR_MASK);
	result = enc28j60_write_data (&send_data, 1);
	result = enc28j60_write_data (&data, 1);

	enc28j60_cs_hi();

	return result;
}

int enc28j60_readOp(uint8_t op, uint8_t addres, uint8_t *data)
{
	int result = 0;
	enc28j60_cs_low();

	uint8_t send_data = op | (addres & ADDR_MASK);
	result = enc28j60_write_data (&send_data, 1);
	if (addres & 0x80) // skip first byte if MAC or MII reg
		result = enc28j60_read_data (data, 1);
	result = enc28j60_read_data (data, 1);

	enc28j60_cs_hi();

	return result;
}

int enc28j60_soft_reset (void)
{
	int result = 0;
	uint8_t reg_data = 0;
	result = enc28j60_writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	HAL_Delay(2);
	while(!(reg_data & ESTAT_CLKRDY))
	{
		result = enc28j60_readOp(ENC28J60_READ_CTRL_REG, ESTAT, &reg_data);
	}

	return result;
}

int enc28j60_read_buf (uint8_t *data, uint32_t len)
{
	int result = 0;
	uint8_t send_data = ENC28J60_READ_BUF_MEM;

	enc28j60_cs_low();
	result = enc28j60_write_data (&send_data, 1);
	result = enc28j60_read_data (data, len);

	enc28j60_cs_hi();

	return result;
}

int enc28j60_write_buf (uint8_t *data, uint16_t len)
{
	int result = 0;
	uint8_t send_data = ENC28J60_WRITE_BUF_MEM;

	enc28j60_cs_low();
	result = enc28j60_write_data (&send_data, 1);
	result = enc28j60_write_data (data, len);
	enc28j60_cs_hi();

	return result;
}

int enc28j60_set_bank (uint8_t addr)
{
	int result = 0;

	if ((addr&BANK_MASK) != enc28j60_BankNum)
	{
		result = enc28j60_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1|ECON1_BSEL0);
		enc28j60_BankNum = addr&BANK_MASK;
		result = enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, enc28j60_BankNum>>5);
	}
	return result;
}

int enc28j60_write_reg_byte(uint8_t addr, uint8_t data)
{
	int result = 0;
	result = enc28j60_set_bank(addr);
	result = enc28j60_writeOp(ENC28J60_WRITE_CTRL_REG, addr, data);

	return result;
}

int enc28j60_read_reg_byte(uint8_t addr, uint8_t *data)
{
	int result = 0;
	result = enc28j60_set_bank(addr);
	result = enc28j60_readOp(ENC28J60_READ_CTRL_REG, addr, data);

	return result;
}

int enc28j60_write_reg(uint8_t addr, uint16_t data)
{
	int result = 0;

	result = enc28j60_write_reg_byte(addr, data&0xFF);
	result = enc28j60_write_reg_byte(addr+1, data>>8);

	return result;
}

int enc28j60_enable_bradcast(void)
{
	int result = 0;
	uint8_t reg_data = 0;

	result = enc28j60_read_reg_byte(MISTAT, &reg_data);
	result = enc28j60_write_reg_byte(ERXFCON, reg_data| ERXFCON_BCEN);

	return result;
}

int enc28j60_write_Phy(uint8_t addres, uint16_t data)
{
	int result = 0;
	uint8_t reg_data = 0;

	result = enc28j60_write_reg_byte(MIREGADR, addres);
	result = enc28j60_write_reg(MIWR, data);

	do {
		result = enc28j60_read_reg_byte(MISTAT, &reg_data);
	} while((reg_data & MISTAT_BUSY));

	return result;
}

int enc28j60_packet_receive(uint8_t *buf, uint16_t buflen, uint32_t *receive_len)
{
	int result = 0;
	uint8_t receive_pct_count = 0;
	header_t pkt_header = {0};
	uint32_t pkt_len = 0;

	result = enc28j60_read_reg_byte(EPKTCNT, &receive_pct_count);

	if (receive_pct_count != 0)
	{
		result = enc28j60_write_reg(ERDPT, enc28j60_gNextPacketPtr);
		result = enc28j60_read_buf((uint8_t*)&pkt_header, sizeof(pkt_header));
		enc28j60_gNextPacketPtr = pkt_header.nextPacket;

		pkt_len = pkt_header.byteCount - 4;  //remove CRC

		if (pkt_len > buflen)
			pkt_len = buflen;

		if ((pkt_header.status & 0x80) == 0)
			pkt_len = 0;
		else
			result = enc28j60_read_buf(buf, pkt_len);

		buf[pkt_len] = 0;

		*receive_len = pkt_len;

		// Init packet pointer to next packet addr
		if((enc28j60_gNextPacketPtr - 1) > RXSTOP_INIT)
			result = enc28j60_write_reg(ERXRDPT, RXSTOP_INIT);
		else
			result = enc28j60_write_reg(ERXRDPT, (enc28j60_gNextPacketPtr-1));

		result = enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC); // Decrement packet counter
	}

	return result;
}

int enc28j60_packet_send(uint8_t *buf, uint16_t buflen)
{
	int result = 0;
	uint8_t econ1_data = 0;
	uint8_t eir_data = 0;

	// Check TX ready
	result = enc28j60_readOp(ENC28J60_READ_CTRL_REG, ECON1, &econ1_data);
	while(econ1_data & ECON1_TXRTS)
	{
		result = enc28j60_read_reg_byte(EIR, &eir_data);
		// Clear INT if need
		if (eir_data & EIR_TXERIF)
		{
			result = enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1,ECON1_TXRST);
			result = enc28j60_writeOp(ENC28J60_BIT_FIELD_CLR, ECON1,ECON1_TXRST);
		}
		result = enc28j60_readOp(ENC28J60_READ_CTRL_REG, ECON1, &econ1_data);
	}

	// Write and send packet
	result = enc28j60_write_reg(EWRPT, TXSTART_INIT);
	result = enc28j60_write_reg(ETXND, TXSTART_INIT+buflen);
	result = enc28j60_write_buf((uint8_t*)"x00", 1);
	result = enc28j60_write_buf(buf, buflen);
	result = enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

	return result;
}


