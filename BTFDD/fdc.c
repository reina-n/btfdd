#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "global.h"
#include "uart0.h"
#include "timer1.h"
#include "fdc.h"



//�ǂݍ��݃e���|�����͂�����g��
volatile unsigned char fdc_data;

//�}�X�^�[���W�X�^�l
volatile unsigned char fdc_mr;

//��ԑJ�ڃ��W�X�^�l
volatile unsigned char fdc_st0[4];

//CHRN�w��o�b�t�@
volatile unsigned char fdc_send_chrn[4];

//���[�h���C�g�n3�o�C�g�X�e�[�^�X�󂯎��p
volatile unsigned char fdc_result_strg[3];

//���[�h���C�g�n4�o�C�g�X�e�[�^�X�󂯎��p
volatile unsigned char fdc_result_chrn[4];




//���s���R�}���h
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

	//CS������ A0�グ��
    PORTC = FDC_LDCR|FDC_LDOR|FDC_RD|FDC_WR|FDC_DACK|FDC_A0;

	//�����͏o��
	data_out();
	PORTA = c;
    _delay_us(1);

	//WR������
    PORTC &= ~(FDC_WR);
    _delay_us(0.2);

	//WR�グ��
    PORTC |= FDC_WR;
    _delay_us(0.2);
}


unsigned char fdc_read(){

	unsigned char c;

	//CS������ A0�グ��
    PORTC = FDC_LDCR|FDC_LDOR|FDC_RD|FDC_WR|FDC_DACK|FDC_A0;

	//�����͓���
	data_in();

	//RD������
    PORTC &= ~(FDC_RD);
    _delay_us(0.2);

    c = PINA;

	//RD�グ��
    PORTC |= FDC_RD;
    _delay_us(0.2);

	return c;
}


void fdc_hardreset(){

	int i;

	//FDC Outputs
    DDRC  = 0xFF;

	//���_���v���A�b�v
    PORTC = FDC_LDCR|FDC_LDOR|FDC_RD|FDC_WR|FDC_CS|FDC_DACK;

    //RESET
    PORTD = _BV(PD7);
    _delay_us(1);
    PORTD &= ~_BV(PD7);
    _delay_us(100);

	//ST0���Z�b�g
	for(i=0;i<=3;i++){
		fdc_st0[i] = 0xFF;
	}

	//BASE���[�h��
	fdc_read_mr();

	//������������҂�
	for(;;){
		if(((fdc_st0[0]|fdc_st0[1]|fdc_st0[2]|fdc_st0[3]) & 0x04) == 0x00) break;
	}

	uart_putsP(PSTR("+BASE"), 5);
	uart_putnewline();
}


void fdc_pcatreset(){

	//���Z�b�g���Ă�����0x04���������ނ�PCAT���[�h
	fdc_write_do(0x00);
	fdc_write_do(0x0C);

	//������������҂�
	for(;;){
		if(((fdc_st0[0]|fdc_st0[1]|fdc_st0[2]|fdc_st0[3]) & 0x04) == 0x00) break;
	}

	//DMA���[�h�ɐ؂�ւ���
	fdc_exec_03(0x33, 0x02 < 1);

	uart_putsP(PSTR("+PCAT"), 5);
	uart_putnewline();
}


unsigned char fdc_read_mr(){

	//�����͓���
	data_in();

	//CS������
    PORTC = FDC_LDCR|FDC_LDOR|FDC_RD|FDC_WR|FDC_DACK;

	//RD������
    PORTC &= ~(FDC_RD);
    _delay_us(1);

    fdc_mr = PINA;

	//RD�グ��
    PORTC |= FDC_RD|FDC_CS;
    return fdc_mr;
}


//�f�W�^���A�E�g���W�X�^
void fdc_write_do(unsigned char c){

	int i;
	//�\�t�g���Z�b�g
	if(c & 0x04){
		//ST0���Z�b�g
		for(i=0;i<=3;i++){
			fdc_st0[i] = 0xFF;
		}
	}

	uart_putsP(PSTR("+OR WRITE "), 10);
	uart_itoh(c);
	uart_putnewline();

	//�����͏o��
	data_out();
	PORTA = c;
    _delay_us(1);

	//WR LDOR�����ɉ�����
    PORTC = FDC_LDCR|FDC_RD|FDC_CS|FDC_DACK;
    _delay_us(1);

	//WR�グ��
    PORTC |= FDC_WR|FDC_LDOR;
}



//�R���g���[�����W�X�^
void fdc_write_cr(unsigned char c){

	uart_putsP(PSTR("+DR WRITE "), 10);
	uart_itoh(c);
	uart_putnewline();

	//�����͏o��
	data_out();
	PORTA = c;
    _delay_us(0.2);

	//WR LDCR�����ɉ�����
    PORTC = FDC_LDOR|FDC_RD|FDC_CS|FDC_DACK;
    _delay_us(0.2);

	//WR�グ��
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

	//CS�グ��
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

	//�����͓���
	data_in();

	//A0�グ��RD������
    PORTC &= ~(FDC_CS);
    PORTC |= FDC_A0;
	_delay_us(1);
    PORTC &= ~(FDC_RD);
	_delay_us(1);

	//�ǂݍ���
    fdc_data = PINA;

	//RD�グ��
    PORTC |= FDC_RD;

	//CS�グ��
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

	//CS�グ��
    PORTC |= FDC_CS;

	//������҂�
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

	//CS�グ��
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

	//�����͓���
	data_in();

	//��������
    for(;;) {
		fdc_read_mr();
		if((fdc_mr & 0xF8) == 0xD0) break;
		_delay_us(100);
	}

	//A0�グ��RD������
    PORTC &= ~(FDC_CS);
    PORTC |= FDC_A0;
	_delay_us(1);
    PORTC &= ~(FDC_RD);
	_delay_us(1);

	//�ǂݍ���
    fdc_data = PINA;

	//RD�グ��
    PORTC |= FDC_RD;


	//0x80��1�o�C�g�A����2�o�C�g
	if(fdc_data != 0x80) {

	    for(;;) {
			fdc_read_mr();
			if(fdc_mr == 0xD0) break;
			_delay_us(100);
		}

		//A0�グ��RD������
	    PORTC &= ~(FDC_CS);
	    PORTC |= FDC_A0;
		_delay_us(1);
   		PORTC &= ~(FDC_RD);
		_delay_us(1);

		//RD�グ��
	    PORTC |= FDC_RD;

		//�X�e�[�^�X�ۑ�
        fdc_st0[fdc_data & 0x03] = fdc_data;

	}

	//CS�グ��
    PORTC |= FDC_CS;

	uart_putsP(PSTR("+SENSE "), 7);
	uart_itoh(fdc_data);
	uart_putnewline();

	return fdc_data;
}



//READ DATA
void fdc_exec_06(unsigned char c1, unsigned char c2){

	//�f�[�^�����v�Z
	unsigned long int len;

 	fdc_command = 0x06;




    //MT�̓T�|�[�g���Ȃ�
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

		//���Z�b�g����
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



//INT���荞��
ISR(INT0_vect)
{

	int i;
    uart_putsP(PSTR("+INT0 "), 6);

	//���荞�ݎ�ʎ擾
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

//DMA���荞��
ISR(INT1_vect)
{
    uart_putsP(PSTR("+INT1\r\n"), 7);
}

