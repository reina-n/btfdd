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

	//メモリ節約用
	const char *str_invalid = PSTR("+INVALID ");
	const char *str_delim   = PSTR(" ");

	//+で始まる行はコメントにしないと受信が飛ぶ
	if(uart_shell_buffer[0] == '+') {
		return;
	}


	//シェル応答開始
	uart_putchar('+');

	//繋がるとCONNECTEDが飛ぶので、イレギュラー処理
	if(!strcmp_P((const char *) uart_shell_buffer, PSTR("CONNECTED"))){

		uart_puts((char *) uart_shell_buffer);
		uart_putnewline();

		uart_putsP(PSTR("+WELCOME"), 8);
		uart_putnewline();
		return;
	}

	//コマンドは6文字
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


	//パラメータを登録
	for(i=0; i<10; i++){
		tp = strtok_P( NULL, str_delim);
		if(tp != NULL) {
			uart_shell_argc++;

			if(strlen(tp) == 4){
				//4文字は2バイト捨てて16進
				uart_shell_argv[i] = htoi(tp+2);
			}
			else if(strlen(tp) < 4){
				//3文字以下は10進
				uart_shell_argv[i] = atoi(tp);
			}
			else{
				//エラーで終了
				uart_puts((char *) uart_shell_buffer);
				uart_putnewline();
				uart_putsP(str_invalid, 9);
				uart_putsP(PSTR("ARGV"), 4);
				uart_putnewline();
				return;
			}
		}else{
			//コマンド終了
			break;
		}
	}

	//清書したコマンドを送信
	uart_puts((char *) uart_shell_command);
	uart_putchar(' ');
	for(i=0; i<uart_shell_argc; i++){
		uart_itoh(uart_shell_argv[i]);
		uart_putchar(' ');
	}
	uart_putnewline();	


	//リセット命令
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FATRST"))){
		fdc_pcatreset();
		return;
	}


	//CMD 0F
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FCMD0F"))){
		fdc_exec_0F(uart_shell_argv[0], uart_shell_argv[1]);
		return;
	}


	//DO書き込み
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FWRDOR"))){
		fdc_write_do(uart_shell_argv[0]);
		return;
	}

	//CR書き込み
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

	//CHRN設定
	if(!strcmp_P((const char *) uart_shell_command, PSTR("FSCHRN"))){
		fdc_set_chrn(uart_shell_argv[0], uart_shell_argv[1], uart_shell_argv[2], uart_shell_argv[3]);
		return;
	}


	//return無し
	uart_putsP(str_invalid, 9);
	uart_putsP(PSTR("COMMAND"), 7);
	uart_putnewline();
}
