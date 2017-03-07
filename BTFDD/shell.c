#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "global.h"
#include "uart0.h"
#include "fdc.h"
#include "shell.h"


volatile char uart_shell_buffer[80];
volatile uint8_t uart_shell_enable;


char uart_shell_command[7];
uint8_t uart_shell_argv[10];
char uart_shell_argc;

void shell_exec() {

	char *tp;
	int i;

	//�������ߖ�p
	const char *str_invalid = PSTR("+INVALID ");
	const char *str_delim   = PSTR(" ");

	//+�Ŏn�܂�s�̓R�����g�ɂ��Ȃ��Ǝ�M�����
	if(uart_shell_buffer[0] == '+') {
		return;
	}


	//�V�F�������J�n
	uart_putchar('+');

	//�q�����CONNECTED����Ԃ̂ŁA�C���M�����[����
	if(!strcmp_P((const char *) uart_shell_buffer, PSTR("CONNECTED"))){

		uart_puts((char *) uart_shell_buffer);
		uart_putnewline();

		uart_putsP(PSTR("+WELCOME"), 8);
		uart_putnewline();
		return;
	}

	//�R�}���h��6����
	tp = strtok_P((char *) uart_shell_buffer, str_delim);
	if(tp == NULL || strlen(tp) != 6) {

		uart_puts((char *) uart_shell_buffer);
		uart_putnewline();

		uart_putsP(str_invalid, 9);
		uart_putsP(PSTR("SYNTAX"), 6);
		uart_putnewline();
		return;
	}
	strcpy(uart_shell_command, tp);
	uart_shell_argc = 0;


	//�p�����[�^��o�^
	for(i=0; i<10; i++){
		tp = strtok_P( NULL, str_delim);
		if(tp != NULL) {
			uart_shell_argc++;

			if(strlen(tp) == 4){
				//4������2�o�C�g�̂Ă�16�i
				uart_shell_argv[i] = htoi(tp+2);
			}
			else if(strlen(tp) < 4){
				//3�����ȉ���10�i
				uart_shell_argv[i] = atoi(tp);
			}
			else{
				//�G���[�ŏI��
				uart_puts((char *) uart_shell_buffer);
				uart_putnewline();
				uart_putsP(str_invalid, 9);
				uart_putsP(PSTR("ARGV"), 4);
				uart_putnewline();
				return;
			}
		}else{
			//�R�}���h�I��
			break;
		}
	}

	//���������R�}���h�𑗐M
	uart_puts((char *) uart_shell_command);
	uart_putchar(' ');
	for(i=0; i<uart_shell_argc; i++){
		uart_itoh(uart_shell_argv[i]);
		uart_putchar(' ');
	}
	uart_putnewline();	


	//���Z�b�g����
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FATRST"))){
		fdc_pcatreset();
		return;
	}


	//CMD 0F
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FCMD0F"))){
		fdc_exec_0F(uart_shell_argv[0], uart_shell_argv[1]);
		return;
	}


	//DO��������
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FWRDOR"))){
		fdc_write_do(uart_shell_argv[0]);
		return;
	}

	//CR��������
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FWRCRR"))){
		fdc_write_cr(uart_shell_argv[0]);
		return;
	}

	//CMD 0A
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FCMD0A"))){
		fdc_exec_0A(uart_shell_argv[0]);
		return;
	}

	//HIGH DENSITY
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FHIDEN"))){
		fdc_set_highden(uart_shell_argv[0]);
		return;
	}

	//CHRN�ݒ�
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FSCHRN"))){
		fdc_set_chrn(uart_shell_argv[0], uart_shell_argv[1], uart_shell_argv[2], uart_shell_argv[3]);
		return;
	}


	//return����
	uart_putsP(str_invalid, 9);
	uart_putsP(PSTR("COMMAND"), 7);
	uart_putnewline();
}
