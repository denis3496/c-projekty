/*
 * main.c
 *
 *  Created on: 4 sie 2018
 *      Author: Denis
 */
/*
 * main.c
 *
 *  Created on: 30 lip 2018
 *      Author: Szef
 */
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>


#define DIP1 (1<<PD0)
#define DIP2 (1<<PD1)
#define DIP3 (1<<PD2)
#define DIP4 (1<<PD3)
//dip switche
#define BTS (1<<PD7)
#define CEWKA (1<<PD6)
#define KONTROLKA (1<<PD7)
#define TEST (1<<PB2)
#define BUZZ (1<<PB0)
#define T_TEST (1<<PB2)
#define T_POZIOM (1<<PB1)
//wszystkie io- wejscia zanegowac!!!
#define BTN_DEBOUCE 20 //ms

ISR(INT0_vect) { //TYLKO NA PINIE PD2!!!

    uint16_t timer = 0;
    while(bit_is_set(PIND, PD2)) { // button hold down- WYKRYWA STAN NISKI- GDY GUZIK ZWIERA DO MASY
        timer++; // count how long button is pressed
        _delay_ms(1);
    }
    if(timer > BTN_DEBOUCE) { // software debouncing button
        if(timer < 5000UL) {//unsigned long
            //single click
        	  PORTD &= ~(1<<PD5); //Turns OFF LED
        	      _delay_ms(500); //3 second delay
        	      PORTD |= (1<<PD5); //Turns ON LED

        } else {
            //button hold

        					PORTD &= ~(1<<PD5); //Turns OFF LED
        	        	      _delay_ms(3000); //3 second delay
        	        	      PORTD |= (1<<PD5); //Turns ON LED- LEDY PODLACZONE DO VCC- SWIECI CALY CZAS
        	        	      _delay_ms(3000); //3 second delay
        	        	      PORTD &= ~(1<<PD5); //Turns OFF LED
        	        	      _delay_ms(3000); //3 second delay
        	        	      PORTD |= (1<<PD5);
        }
    }
    timer=0;
}

void io_init()
{
	  DDRD = 0b11000000;
	  //WEJ NA DIP Z REZYSTORAMI PULL DOWN, PD4-WEJ, PD5- WEJ KONTROLKA,PD6-WYJ NA CEWKE, PD7-WY NA BTS
	  DDRB = 0b00111001;
	  // PB0- WYJ NA BUZZ, PB1(T_POZIOM), PB2(T_TEST)- WEJSCIA Z TRANSOPTORA, PB3,PB4,PB5-MAGISTRALA SPI

}

void initInterrupt1(void)
{
	//TCCR0 |=(1<<WGM00)|(1<<WGM01)|(1<<COM00)|(1<<COM01)|(1<<CS00);
	 //   OCR0 = 0;
}

void initInterrupt0(void) {
    GIMSK |= (1 << INT0); // general interrupt mask register -> enable interrupt on INT0
    MCUCR &= ~(1 << ISC00); // trigger interrupt on falling edge
    MCUCR |= (1 << ISC01); // trigger interrupt on falling edge
    sei(); //  set interrupt enable bit
}

uint8_t time_switch()
{
	unsigned char io = PIND;
	uint8_t tmp;

	if((io & DIP1)==0) tmp=10; // GDY ZALACZONY POSZCZEGOLNY SWITCH
	if((io & DIP2)==0) tmp=20;
	if((io & DIP3)==0) tmp=30;
	if((io & DIP4)==0) tmp=40;

	return tmp;
}


int main(void)
{
	uint8_t TIME1, TIME2;
	io_init();
	initInterrupt1();//pwm
	initInterrupt0();//test guzik
	 TIME1= time_switch();
	 TIME2= test_butt();
asdasdasasd
	 while(1);

	 return 0;
}




