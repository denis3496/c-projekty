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
#include <avr/pgmspace.h>
#include <string.h>
#include "lcd44780.h"

#define DIP1 (1<<PC0) // zmienic na PD w docelowym
#define DIP2 (1<<PC1)
#define DIP3 (1<<PC2)
#define DIP4 (1<<PC3)
//dip switche
#define BTS (1<<PD7)
#define CEWKA (1<<PD6)
#define KONTROLKA (1<<PD7)
#define TEST (1<<PB2)
#define BUZZ (1<<PB0)
#define T_TEST (1<<PB2)
#define T_POZIOM (1<<PB1)
//wszystkie io- wejscia zanegowac!!!
//--------------------------------------
#define IDLE 0
#define ACTIVE 50 // odpowiada wypelnieniu PWM
volatile unsigned char MOTOR_STAN;
//-----------------------------------------
#define czas_pracy 10 //ile sek krecic, kreci 10 sek i wylacza
volatile uint16_t motor_wait;//co ile wlaczac silnik
volatile uint16_t sek_count;// volatile mowi ze zmienna nalezy do wielu watkow dlatego nie mozna jej optymalizowac
//------------------------------------------
void io_init()
{
	//  DDRD = 0b11000000;
	  //WEJ NA DIP Z REZYSTORAMI PULL DOWN, PD4-WEJ, PD5- WEJ KONTROLKA,PD6-WYJ NA CEWKE, PD7-WY NA BTS
	//tymczasowo:
	  DDRD = 0b11111011; //0- PD2 GUZIK
	  DDRD &= ~(1<<PD2);//Makes firs pin of PORTD as Input
	  PORTD |= (1<<PD2);

	  DDRB = 0b00111001;
	  // PB0- WYJ NA BUZZ, PB1(T_POZIOM), PB2(T_TEST)- WEJSCIA Z TRANSOPTORA, PB3(OC2-pwm),PB4,PB5-MAGISTRALA SPI

	  DDRC = 0b11110000; // DIPY
	  PORTC = 0b00001111; // pull up

}

void initInterrupt2(void) {// TIMER1 INIT
	TCCR1A |= 0;
	TCCR1B |= (1 << WGM12)|(1<<CS12) | (1<<CS10); // Ÿród³em CLK, preskaler 64, Mode = CTC,
	// enable compare interrupt
		TIMSK |= (1 << OCIE1A);
	TCNT1 = 0;
	// initialize compare value
		OCR1A = 8333; // ok. 1SEK

		sek_count=0;

}

void initInterrupt1(void)
{

	    TCCR0=0b01110101; //Configure TCCR0 as explained in the article
	    OCR0=0; // Set OCR0 to 0 so that the duty cycle is initially 0 and the motor is not rotating, 127- plynnie jedzie
	   /*	TCCR0= [FOC0	|WGM00|	COM01|	COM00|	WGM01|	CS02|	CS01|	CS00]
	    Set bits WGM00 and WGM01 to 1 and 0 respectively. This enables the phase correct PWM mode.
	    Set bits COM00 and COM01 to 0 and 1 respectively. This means that the generated PWM will be an inverted PWM.
	    Set bits CS00 and CS01 and CS02 to 1, 0 and 1 respectively. This means that the counter will be clocked from the system clock divided by 1024.*/
}

void initInterrupt0(void) {
    GICR |= (1 << INT0); // general interrupt mask register -> enable interrupt on INT0. dla atmegi 8- GIMSK
    MCUCR &= ~(1 << ISC00); // trigger interrupt on falling edge
    MCUCR |= (1 << ISC01); // trigger interrupt on falling edge
}

uint16_t time_switch()
{
	unsigned char io = PINC; // normalnie pind
	uint16_t tmp;

	if((io & DIP1)==0) tmp=15; // GDY ZALACZONY POSZCZEGOLNY SWITCH. trzeba zresetowac po zmianie
	else if((io & DIP2)==0) tmp=20;
	else if((io & DIP3)==0) tmp=30;
	else if((io & DIP4)==0) tmp=40;
	else tmp=10; // domyslnie

	return tmp;
}

void greeting(){
	char napis1[] = "HELLO";
	lcd_cls();
		lcd_locate(1,1);
		lcd_str(napis1);
		_delay_ms(1500);
		lcd_cls();
}

ISR(INT0_vect) { //TYLKO NA PINIE PD2!!!
	lcd_cls();
    uint16_t timer = 0;
    while(bit_is_clear(PIND, PD2)) { // button hold down- WYKRYWA STAN NISKI- GDY GUZIK ZWIERA DO MASY
        timer++; // count how long button is pressed
        lcd_locate(1,1);
        lcd_int(timer);
           _delay_ms(500);
    }
    	if(timer !=0) motor_wait = timer;

}
ISR(TIMER1_COMPA_vect) //tik co ok. 1 sekunde
{
	   sek_count++; // zwieksz globalna ilosc sekund

	   lcd_locate(0,1);
	   lcd_int(motor_wait);
	   lcd_locate(0,6);
	   lcd_int(sek_count);
	   _delay_ms(500);
	   lcd_cls();
	   //tu sprawdzanie globalnej flagi sekund

	   if(sek_count==czas_pracy && MOTOR_STAN==ACTIVE)  {
		   MOTOR_STAN=IDLE;
		   OCR0 =0; // WLACZA SILNIK LUB WYLACZA CO 3 SEKUNDY NA PRZEMIAN
		   sek_count =0; //zliczaj od nowa sekundy
	   }
	   else if(sek_count==motor_wait && MOTOR_STAN==IDLE){
		   MOTOR_STAN=ACTIVE;
		   OCR0 = 50;
		   sek_count =0; //zliczaj od nowa sekundy
	   }





}
int main(void)
{

	  DDRD |= (1<<PD1); //Nakes first pin of PORTC as Output
	  PORTD |= (1<<PD1); //WYLACZA LED BO DO LED DO VCC. GDYBY LED BYL DO MASY TO TA KOMENDA WLACZA LED
	io_init();
	lcd_init();


	greeting();
	motor_wait= time_switch();

	initInterrupt1();//pwm
	initInterrupt0();//test guzik
	initInterrupt2();//pokazuje czas co 3 sekundy

	sei();

	 while(1);

	 return 0;
}




