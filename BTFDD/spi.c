#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "spi.h"
#include "uart0.h"

uint8_t  spi_leds      = 0;
volatile uint32_t spi_sram_addr = 0;
volatile uint16_t spi_sram_crc;

void spi_master(uint8_t c) {

	if(c) {
		uart_putsP(PSTR("+MASTER\r\n"), 9);
		//��x�����ɂ��Ȃ��ƍėL�����ł��Ȃ��H
		SPCR   = 0x00; 
		DDRB  |= _BV(PB4)|_BV(PB5)|_BV(PB7);
		SPCR   = _BV(SPE) | _BV(MSTR); 
	}else{
		SPCR   = 0x00; 
		SPCR   = _BV(SPE);
	}
}


void spi_leds_set(unsigned char c) {

	spi_leds = c;
	SPDR     = c;
    while((SPSR & _BV(SPIF)) == 0);

	PORTD &= ~_BV(PD5);
	PORTD |= _BV(PD5);
}

//SRAM�ǂݏo������
void spi_sram_start_read(unsigned long start) {

	//CS������
	PORTD &= ~_BV(PD4);

	//READ���M
	SPDR     = 0x03;
    while((SPSR & _BV(SPIF)) == 0);

	//�A�h���X���M
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
}


//SRAM�����o������
void spi_sram_start_write(unsigned long start) {

	//CS������
	PORTD &= ~_BV(PD4);

	//WRITE���M
	SPDR     = 0x02;
    while((SPSR & _BV(SPIF)) == 0);

	//�A�h���X���M
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
}


//SRAM�ǂݏo������
void spi_sram_stop() {

	PORTD |= _BV(PD4);
}

//SRAM�����߂�
void spi_sram_rewind() {

	//0�o�C�g�ɖ߂�
	spi_sram_addr = 0;
	spi_sram_crc  = 0xFFFF;
}


//SRAM���W�X�^��������
void spi_sram_resister_write(unsigned char c) {

	//CS������
	PORTD &= ~_BV(PD4);

	//���W�X�^�����������M
	SPDR     = 0x01;
    while((SPSR & _BV(SPIF)) == 0);

	//���W�X�^�l�ݒ�
	SPDR     = c;
    while((SPSR & _BV(SPIF)) == 0);

	PORTD |= _BV(PD4);
}


//SRAM���W�X�^�ǂݍ���
unsigned char spi_sram_resister_read() {

	unsigned char c;

	//CS������
	PORTD &= ~_BV(PD4);

	//���W�X�^�����������M
	SPDR     = 0x05;
    while((SPSR & _BV(SPIF)) == 0);

	//�_�~�[���M
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);

	c=SPDR;

	PORTD |= _BV(PD4);

    return c;
}
