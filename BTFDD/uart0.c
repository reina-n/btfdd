#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "global.h"
#include "crc16.h"
#include "shell.h"
#include "spi.h"
#include "shell.h"

volatile char uart_shell_buffer[80];
volatile uint8_t uart_shell_enable;
uint8_t uart_shell_length = 0;

volatile uint32_t spi_sram_addr;
volatile uint16_t spi_sram_crc;


//1文字送信
void uart_putchar(char c) {

    // Enable the USART Receive interrupt
	UCSR0B &= ~(1 << RXCIE0 );

    loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
    UDR0 = c;
    loop_until_bit_is_set(UCSR0A, TXC0); /* Wait until transmission ready. */

	UCSR0B |= (1 << RXCIE0 );
}

//PGMから送信
void uart_putsP(const char *data, uint8_t len) {
	uint8_t i;
	for (i = 0; i < len; i++) {
		uart_putchar(pgm_read_byte(data+i));
	};
}

//改行を送信
void uart_putnewline() {
		uart_putchar('\r');
		uart_putchar('\n');
}

//RAMから送信
void uart_puts(char *str) {
    int i = 0;
    while(str[i] != '\0') { // Loop through string, sending each character
        uart_putchar(str[i]);
        i++;
    }
}



//16進2桁を出力
void uart_itoh(unsigned char value)
{
	char scratch[2] = {'0', '0'};
	int offset = 1;
	uint8_t c;
	uint8_t accum;

	if(value != 0)
	{
		accum = value;
		while(accum) {
			c = accum % 16;
			if(c < 10) {
				c += '0';
			}else{
				c += 'A' - 10;
			}
			scratch[offset] = c;
			accum /= 16;
			offset--;
		}
		offset++;
	}

	uart_putchar(scratch[0]);		
	uart_putchar(scratch[1]);
}



//SRAM操作(クリティカル)
void uart_sram_write_hex(uint32_t size) {
}

//SRAM操作(クリティカル)
void uart_sram_write_raw(unsigned long size) {

	//読み取り済みバイト数
	unsigned long recieved = 0;
	unsigned char c;

	uart_putsP(PSTR("+SRAM WRITE BYTES "), 12);
    uart_itoh((size >> 16) & 0xFF);
    uart_itoh((size >>  8) & 0xFF);
    uart_itoh((size)       & 0xFF);
	uart_putnewline();

	spi_master(1);
	spi_sram_start_write(0UL);


	//割り込み禁止
	UCSR0B &=~_BV(RXCIE0);

	while(recieved < size){
	    loop_until_bit_is_set(UCSR0A, RXC0);
		c = UDR0;
		SPDR = c;
		//spi_sram_crc = crcByte(spi_sram_crc, c);
		//spi_sram_crc = _crc_ccitt_update(spi_sram_crc, c);
		spi_sram_crc = accumulate_crc16(spi_sram_crc, c);

	    while(!(SPSR & (1<<SPIF)));
		recieved++;
	}

	//割り込み許可
	UCSR0B |= (1 << RXCIE0 );

	spi_sram_stop();
	spi_master(0);

	uart_putsP(PSTR("+SRAM WRITE DONE "), 17);
    uart_itoh((unsigned char) (spi_sram_crc >> 8) & 0xFF);
    uart_itoh((unsigned char) (spi_sram_crc     ) & 0xFF);
	uart_putnewline();


}

//SRAM操作(クリティカル)
void uart_sram_read_hex(unsigned long size) {

	//残り読み取りバイト数
	unsigned long sent  = 0;

	//残り読み取りバイト数
	unsigned long total = size;


	//溢れた場合
	//if(rem > spi_sram_addr) {
	//	rem = spi_sram_addr;
	//}

	spi_master(1);
	spi_sram_start_read(0);

	while(total > sent) {

		//ダミー送信
	    SPDR = 0x00;
	    while(!(SPSR & (1<<SPIF)));
	    uart_itoh(SPDR);

//	    uart_itoh(sent);
		sent++;
	}

	spi_sram_stop();
	spi_master(0);
}



ISR (USART0_RX_vect)
{
    // Get data from the USART in register
    uart_shell_buffer[uart_shell_length] = UDR0;

    // End of line!
    if (uart_shell_buffer[uart_shell_length] == '\r' || uart_shell_buffer[uart_shell_length] == '\n') {

		uart_shell_buffer[uart_shell_length] = '\0'; 

		if(uart_shell_length > 0) {

	        // Reset to 0, ready to go again
	        uart_shell_length = 0;
	        uart_shell_enable = 2;
			return;
		}

    } else {
		if(uart_shell_length < 79) {
	        uart_shell_length++;
		}else{
			uart_putsP(PSTR("+UART OVERFLOW"), 14);
			uart_putnewline();
		}
    }
}
