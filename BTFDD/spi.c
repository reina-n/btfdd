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
		//一度無効にしないと再有効化できない？
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

//SRAM読み出し準備
void spi_sram_start_read(unsigned long start) {

	//CS下げる
	PORTD &= ~_BV(PD4);

	//READ送信
	SPDR     = 0x03;
    while((SPSR & _BV(SPIF)) == 0);

	//アドレス送信
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
}


//SRAM書き出し準備
void spi_sram_start_write(unsigned long start) {

	//CS下げる
	PORTD &= ~_BV(PD4);

	//WRITE送信
	SPDR     = 0x02;
    while((SPSR & _BV(SPIF)) == 0);

	//アドレス送信
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);
}


//SRAM読み出し準備
void spi_sram_stop() {

	PORTD |= _BV(PD4);
}

//SRAM巻き戻し
void spi_sram_rewind() {

	//0バイトに戻す
	spi_sram_addr = 0;
	spi_sram_crc  = 0xFFFF;
}


//SRAMレジスタ書き込み
void spi_sram_resister_write(unsigned char c) {

	//CS下げる
	PORTD &= ~_BV(PD4);

	//レジスタ書き換え送信
	SPDR     = 0x01;
    while((SPSR & _BV(SPIF)) == 0);

	//レジスタ値設定
	SPDR     = c;
    while((SPSR & _BV(SPIF)) == 0);

	PORTD |= _BV(PD4);
}


//SRAMレジスタ読み込み
unsigned char spi_sram_resister_read() {

	unsigned char c;

	//CS下げる
	PORTD &= ~_BV(PD4);

	//レジスタ書き換え送信
	SPDR     = 0x05;
    while((SPSR & _BV(SPIF)) == 0);

	//ダミー送信
	SPDR     = 0x00;
    while((SPSR & _BV(SPIF)) == 0);

	c=SPDR;

	PORTD |= _BV(PD4);

    return c;
}
