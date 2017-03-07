#include <avr/io.h>
#include <avr/interrupt.h>
#include "global.h"
#include "timer1.h"


volatile uint8_t timer_count;
volatile uint8_t timer_timeout;

ISR(TIMER1_CAPT_vect)
{
    timer_count++;
}

void timer_stop(){

	//���䃌�W�X�^A
	TCCR1A = 0x00;
	//���䃌�W�X�^B
	TCCR1B = _BV(WGM13) | _BV(WGM12);

	TIMSK1 = 0x00;
}

void timer_start(){

	//���䃌�W�X�^A
	TCCR1A = 0x00;
	//���䃌�W�X�^B
	TCCR1B = _BV(WGM13) | _BV(WGM12) |_BV(CS12) |_BV(CS10);
  
	//�ő�l 1�b
	ICR1 = 18000;

	TIMSK1 = _BV(ICIE1);

	timer_count = 0;
}
