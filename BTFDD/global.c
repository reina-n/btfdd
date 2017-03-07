#include <avr/io.h>
#include <avr/pgmspace.h>
#include "global.h"

void data_in() {
	//データIOポート
    DDRA  = 0x00;
    PORTA = 0x00;
}

void data_out() {
	//データIOポート
    DDRA  = 0xFF;
}

char base64chars[] PROGMEM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//strtol関数が動かないので代替
int htoi (const char *s) {
    int n;
    
    for(n=0;*s;s++){
        if ( *s >= '0' && *s <= '9' ) {
            n = 16 * n + (*s - '0');
        }
        else if ( *s >= 'a' && *s <= 'f' ) {
            n = 16 * n + ((*s - 'a') + 10);
        }
        else if ( *s >= 'A' && *s <= 'F' ) {
            n = 16 * n + ((*s - 'A') + 10);
        }
    }
    return n;
}

unsigned int crcByte(unsigned int crc, unsigned char b) {
  crc = (unsigned char)(crc >> 8) | (crc << 8);
  crc ^= b;
  crc ^= (unsigned char)(crc & 0xff) >> 4;
  crc ^= crc << 12;
  crc ^= (crc & 0xff) << 5;
  return crc;
}
