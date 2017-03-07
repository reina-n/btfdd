#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "global.h"
#include "spi.h"
#include "uart0.h"
#include "timer1.h"
#include "shell.h"
#include "fdc.h"



volatile unsigned char fdc_mr;

//状態遷移レジスタ値
volatile unsigned char fdc_st0[4];

//実行中コマンド
volatile unsigned char fdc_command;

//タイマー1Hz
volatile uint8_t timer_count;

//タイマー1Hz
volatile uint8_t timer_timeout;

//シェルモード
volatile uint8_t uart_shell_enable;

int main(){


    //JTAG停止
    MCUCR |= 0x80;
    MCUCR |= 0x80;


    // Turn on USART hardware (RX, TX)
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */

    // 8 bit char sizes
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */

    // Enable the USART Receive interrupt
	UCSR0B |= (1 << RXCIE0 );

    //倍速禁止
	UCSR0A &= ~(_BV(U2X0));

    //115200bps
    UBRR0H = 0x00;
    UBRR0L = 0x13;

    //立ち上がり割り込み
    EICRA = _BV(ISC00)|_BV(ISC01)|_BV(ISC10)|_BV(ISC11);
    EIMSK |= _BV(INT1)|_BV(INT0);


	SPCR = _BV(SPE);       /* SPI許可, SLAVE, fosc / 4 */
	SPSR = 0x00;

	//主CPU INT6
    DDRB  = 0xFF;
    PORTB = 0x00;

	//半分だけ出力
    DDRD  = 0xF0;
	//主CPU INT1以外上げる
	PORTD = _BV(PD4)|_BV(PD5)|_BV(PD7);

	timer_timeout = 5;

	sei();



	spi_master(1);
	spi_leds_set(0xFF);
	spi_master(0);


	//リセットする
	fdc_hardreset();
	fdc_pcatreset();

	//モーターON
	fdc_write_do(0x1C);

	//ドライブ状態取得
	fdc_exec_04(0x00);


	//ヘッド位置を0に戻す
	fdc_exec_07(0x00);
	for(;;){
		if(fdc_command == 0x00) break;
	}
	uart_putsP(PSTR("+SPECIFY\r\n"), 10);

	//ヘッド移動
	//fdc_exec_0F(0x00, 0x04);
	//uart_putsP(PSTR("+SEEK\r\n"), 7);
	//fdc_write_cr(0x02);
	//fdc_exec_0A(0x44);


	//モーター停止
	fdc_write_do(4+8);


    //SRAMモード設定
	spi_master(1);
	spi_leds_set(0x08);
	spi_sram_resister_write(0x40);
	uart_putsP(PSTR("+SRAM REG "), 10);
	uart_itoh(spi_sram_resister_read());
	uart_putnewline();
	spi_master(0);

	spi_sram_rewind();
	uart_sram_write_raw(0x08UL);
	uart_sram_read_hex(0x10UL);

	uart_shell_enable = 1;


	for(;;){
		if(uart_shell_enable == 2) {
			shell_exec();
			uart_shell_enable = 1;
		}
		if(uart_shell_enable == 3) {
			uart_shell_enable = 1;
		}

	}

}


