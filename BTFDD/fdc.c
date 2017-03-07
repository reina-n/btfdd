#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "global.h"
#include "uart0.h"
#include "timer1.h"
#include "fdc.h"



//読み込みテンポラリはこれを使う
volatile unsigned char fdc_data;

//マスターレジスタ値
volatile unsigned char fdc_mr;

//状態遷移レジスタ値
volatile unsigned char fdc_st0[4];

//CHRN指定バッファ
volatile unsigned char fdc_send_chrn[4];

//リードライト系3バイトステータス受け取り用
volatile unsigned char fdc_result_strg[3];

//リードライト系4バイトステータス受け取り用
volatile unsigned char fdc_result_chrn[4];




//実行中コマンド
volatile unsigned char fdc_command;


void fdc_loop_until_rqm(){
    for(;;) {
		fdc_read_mr();
		if((fdc_mr & 0xC0) == 0x80) break;
		_delay_us(100);
	}
}


void fdc_loop_until_dio(){
    for(;;) {
		fdc_read_mr();
		if((fdc_mr & 0xD0) == 0xD0) break;
		_delay_us(100);
	}
}


void fdc_write(unsigned char c){

	//CS下げる A0上げる
    PORTC = FDC_LDCR|FDC_LDOR|FDC_RD|FDC_WR|FDC_DACK|FDC_A0;

	//方向は出力
	data_out();
	PORTA = c;
    _delay_us(1);

	//WR下げる
    PORTC &= ~(FDC_WR);
    _delay_us(0.2);

	//WR上げる
    PORTC |= FDC_WR;
    _delay_us(0.2);
}


unsigned char fdc_read(){

	unsigned char c;

	//CS下げる A0上げる
    PORTC = FDC_LDCR|FDC_LDOR|FDC_RD|FDC_WR|FDC_DACK|FDC_A0;

	//方向は入力
	data_in();

	//RD下げる
    PORTC &= ~(FDC_RD);
    _delay_us(0.2);

    c = PINA;

	//RD上げる
    PORTC |= FDC_RD;
    _delay_us(0.2);

	return c;
}


void fdc_hardreset(){

	int i;

	//FDC Outputs
    DDRC  = 0xFF;

	//負論理プルアップ
    PORTC = FDC_LDCR|FDC_LDOR|FDC_RD|FDC_WR|FDC_CS|FDC_DACK;

    //RESET
    PORTD = _BV(PD7);
    _delay_us(1);
    PORTD &= ~_BV(PD7);
    _delay_us(100);

	//ST0リセット
	for(i=0;i<=3;i++){
		fdc_st0[i] = 0xFF;
	}

	//BASEモードに
	fdc_read_mr();

	//初期化完了を待つ
	for(;;){
		if(((fdc_st0[0]|fdc_st0[1]|fdc_st0[2]|fdc_st0[3]) & 0x04) == 0x00) break;
	}

	uart_putsP(PSTR("+BASE"), 5);
	uart_putnewline();
}


void fdc_pcatreset(){

	//リセットしてすぐに0x04を書き込むとPCATモード
	fdc_write_do(0x00);
	fdc_write_do(0x0C);

	//初期化完了を待つ
	for(;;){
		if(((fdc_st0[0]|fdc_st0[1]|fdc_st0[2]|fdc_st0[3]) & 0x04) == 0x00) break;
	}

	//DMAモードに切り替える
	fdc_exec_03(0x33, 0x02 < 1);

	uart_putsP(PSTR("+PCAT"), 5);
	uart_putnewline();
}


unsigned char fdc_read_mr(){

	//方向は入力
	data_in();

	//CS下げる
    PORTC = FDC_LDCR|FDC_LDOR|FDC_RD|FDC_WR|FDC_DACK;

	//RD下げる
    PORTC &= ~(FDC_RD);
    _delay_us(1);

    fdc_mr = PINA;

	//RD上げる
    PORTC |= FDC_RD|FDC_CS;
    return fdc_mr;
}


//デジタルアウトレジスタ
void fdc_write_do(unsigned char c){

	int i;
	//ソフトリセット
	if(c & 0x04){
		//ST0リセット
		for(i=0;i<=3;i++){
			fdc_st0[i] = 0xFF;
		}
	}

	uart_putsP(PSTR("+OR WRITE "), 10);
	uart_itoh(c);
	uart_putnewline();

	//方向は出力
	data_out();
	PORTA = c;
    _delay_us(1);

	//WR LDOR同時に下げる
    PORTC = FDC_LDCR|FDC_RD|FDC_CS|FDC_DACK;
    _delay_us(1);

	//WR上げる
    PORTC |= FDC_WR|FDC_LDOR;
}



//コントロールレジスタ
void fdc_write_cr(unsigned char c){

	uart_putsP(PSTR("+DR WRITE "), 10);
	uart_itoh(c);
	uart_putnewline();

	//方向は出力
	data_out();
	PORTA = c;
    _delay_us(0.2);

	//WR LDCR同時に下げる
    PORTC = FDC_LDOR|FDC_RD|FDC_CS|FDC_DACK;
    _delay_us(0.2);

	//WR上げる
    PORTC |= FDC_WR|FDC_LDCR;

}





//specify
void fdc_exec_03(unsigned char c1, unsigned char c2){

	fdc_command = 0x03;

	fdc_loop_until_rqm();
	fdc_write(0x03);

	fdc_loop_until_rqm();
	fdc_write(c1);

	fdc_loop_until_rqm();
	fdc_write(c2);

	//CS上げる
    PORTC |= FDC_CS;
    _delay_us(0.5);
}



//DEVICE
unsigned char fdc_exec_04(unsigned char c){

	fdc_loop_until_rqm();
	fdc_write(0x04);

	fdc_loop_until_rqm();
	fdc_write(c);

    for(;;) {
		fdc_read_mr();
		if(fdc_mr == 0xD0) break;
		_delay_us(100);
	}

	//方向は入力
	data_in();

	//A0上げるRD下げる
    PORTC &= ~(FDC_CS);
    PORTC |= FDC_A0;
	_delay_us(1);
    PORTC &= ~(FDC_RD);
	_delay_us(1);

	//読み込み
    fdc_data = PINA;

	//RD上げる
    PORTC |= FDC_RD;

	//CS上げる
    PORTC |= FDC_CS;

	uart_putsP(PSTR("+DEVICE "), 8);
	uart_itoh(fdc_data);
	uart_putnewline();

	return fdc_data;
}




//SEEK
void fdc_exec_0F(unsigned char c1, unsigned char c2){

	fdc_command = 0x0F;

	fdc_loop_until_rqm();
	fdc_write(0x0F);

	fdc_loop_until_rqm();
	fdc_write(c1);

	fdc_loop_until_rqm();
	fdc_write(c2);

	//CS上げる
    PORTC |= FDC_CS;

	//完了を待つ
	for(;;){
		if(fdc_command == 0x00) break;
	}
}





//RECALIBARTE
void fdc_exec_07(unsigned char c){

	fdc_command = 0x07;

	fdc_loop_until_rqm();
	fdc_write(0x07);

	fdc_loop_until_rqm();
	fdc_write(c);

	//CS上げる
    PORTC |= FDC_CS;
}




//SENSE
unsigned char fdc_exec_08(){

	fdc_command = 0x07;

    for(;;) {
		fdc_read_mr();
		if((fdc_mr & 0xF8) == 0x80) break;
		_delay_us(100);
	}

	fdc_write(0x08);

	//方向は入力
	data_in();

	//準備完了
    for(;;) {
		fdc_read_mr();
		if((fdc_mr & 0xF8) == 0xD0) break;
		_delay_us(100);
	}

	//A0上げるRD下げる
    PORTC &= ~(FDC_CS);
    PORTC |= FDC_A0;
	_delay_us(1);
    PORTC &= ~(FDC_RD);
	_delay_us(1);

	//読み込み
    fdc_data = PINA;

	//RD上げる
    PORTC |= FDC_RD;


	//0x80は1バイト、他は2バイト
	if(fdc_data != 0x80) {

	    for(;;) {
			fdc_read_mr();
			if(fdc_mr == 0xD0) break;
			_delay_us(100);
		}

		//A0上げるRD下げる
	    PORTC &= ~(FDC_CS);
	    PORTC |= FDC_A0;
		_delay_us(1);
   		PORTC &= ~(FDC_RD);
		_delay_us(1);

		//RD上げる
	    PORTC |= FDC_RD;

		//ステータス保存
        fdc_st0[fdc_data & 0x03] = fdc_data;

	}

	//CS上げる
    PORTC |= FDC_CS;

	uart_putsP(PSTR("+SENSE "), 7);
	uart_itoh(fdc_data);
	uart_putnewline();

	return fdc_data;
}



//READ DATA
void fdc_exec_06(unsigned char c1, unsigned char c2){

	//データ長を計算
	unsigned long int len;

 	fdc_command = 0x06;




    //MTはサポートしない
	fdc_loop_until_rqm();
	fdc_write(0x06 | (c1 & 0x60));

 	fdc_loop_until_rqm();
	fdc_write(c1 & 0x07);




}




//READ ID
void fdc_exec_0A(unsigned char c){

 	fdc_command = 0x0A;

	fdc_loop_until_rqm();
	fdc_write(0x0A | (c & 0x40));

 	fdc_loop_until_rqm();
	fdc_write(c & 0x07);

	timer_start();
	for(;;){
		if(fdc_command == 0x00 || timer_count >= timer_timeout) break;
	}
	timer_stop();

    uart_putsP(PSTR("+READID "), 8);
	if(fdc_command == 0x00) {
		uart_putsP(PSTR("SUCCEEDED"), 9);
		uart_putnewline();
	}else{
		uart_putsP(PSTR("FAILED FORCE-RESET"), 18);
		uart_putnewline();

		//リセットする
		fdc_pcatreset();
	}
}


void fdc_set_highden(unsigned char c){

    uart_putsP(PSTR("+DENSITY "), 9);
	if(c) {
		//360rpm
    	PORTB |= _BV(PB3);
	    uart_putsP(PSTR("HIGH"), 4);
		
	}else{
		//300rpm
    	PORTB &= ~_BV(PB3);
	    uart_putsP(PSTR("LOW"), 3);
	}
	uart_putnewline();
}


void fdc_set_chrn(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4){

	int i;
	fdc_send_chrn[0] = c1;
	fdc_send_chrn[1] = c2;
	fdc_send_chrn[2] = c3;
	fdc_send_chrn[3] = c4;
    uart_putsP(PSTR("+CHRN"), 5);
	for(i=0; i<4; i++){
		uart_putchar(' ');
		uart_itoh(fdc_send_chrn[i]);		
	}
	uart_putnewline();
}



//INT割り込み
ISR(INT0_vect)
{

	int i;
    uart_putsP(PSTR("+INT0 "), 6);

	//割り込み種別取得
	fdc_mr = fdc_read_mr();

    if((fdc_mr & 0xC0) == 0x80) {
	    uart_putsP(PSTR("SENSE"), 5);
		uart_putnewline();

		for(;;) {
			fdc_exec_08();
			if(fdc_data == 0x80) break;
		}
	} else {
	    uart_putsP(PSTR("RESULT"), 6);
		uart_putnewline();
	    uart_putsP(PSTR("RESULT"), 6);

		for(i = 0; i<=2; i++) {
			fdc_loop_until_dio();
			fdc_result_strg[i] = fdc_read();
			uart_putchar(' '); 
			uart_itoh(fdc_result_strg[i]);
		}

		for(i = 0; i<=3; i++) {
			fdc_loop_until_dio();
			fdc_result_chrn[i] = fdc_read();
			uart_putchar(' '); 
			uart_itoh(fdc_result_chrn[i]);
		}
		uart_putnewline();
	}

    uart_putsP(PSTR("+INT0 DONE"), 10);
	uart_putnewline();

	fdc_command  = 0x00;
}

//DMA割り込み
ISR(INT1_vect)
{
    uart_putsP(PSTR("+INT1\r\n"), 7);
}

