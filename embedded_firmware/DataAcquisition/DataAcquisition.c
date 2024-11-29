/*
 * DataAcquisition.c
 *
 * Created: Sep. 2024, Bochert
 *
 * Program skeleton for the experiments in maritime 
 * systems laboratory of embedded system design.
 * Prior to modify the program, change the name in
 * the "Created:" line. 
 */ 

#define F_CPU 16000000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "fifo.h"
#include "uart.h"
#include "dataio.h"
#include "timer0interupt.h"

volatile uint16_t intnum=0;
volatile uint8_t *bufcounter;

void InitialiseHardware(void)
{
	sei();							// enables interrupts by setting the global interrupt mask
	AdcInit();						// initializes the a/d converter
	bufcounter = uart_init(19200);	// initializes the UART for the given baudrate
	PortInit();						// initializes the port settings
	StartTimer0Interrupt();			// timer 0 interrupt for 15 ms
}

int main(void)
/*
Example program for first experiments.
After initializing the interfaces and "Hello World" is send to the serial port.
In a period of a second port pin D7 is toggled and sample data are send to the 
serial port. These sample data contain an index, analog data input, digital port 
inputs and an interrupt counter.
*/
{
	char Text[64];
	uint16_t ADCValue;
	uint16_t index=0;
	
	// added variables
	int32_t temp = 0;
	//int16_t voltage = 0;
	int16_t v0, v100 = 0;
	int32_t a1 = 0; // 40.48 
	int32_t a0 = 0; // -8.5
	
	// initialize hardware
	InitialiseHardware(); 
	_delay_ms(5000);
	sprintf( Text,"\r\nHello temperature\r\n");
	uart_puts (Text); _delay_ms(1000);

	// calibration promting reviewd by prof muller
	sprintf(Text, "Press button to start auto calibration.\r\n");
	uart_puts(Text);
	_delay_ms(1000);
	
	// start of auto calibration
	while(ReadPortD(3) == 1)
	sprintf(Text, "Auto calibration. Put sensor in ice water and press button.\r\n");
	uart_puts(Text);
	_delay_ms(1000);
	
	// auto calibration value for ice water
	while(ReadPortD(3) == 1){}
	sprintf(Text, "Reading voltage for Ice Water.\r\n");
	uart_puts(Text);
	_delay_ms(50);
	v0 = ReadAdc(6);
		
	// auto calibration value for boiling water
	sprintf(Text, "Put the sensor in boiling water and press the button.\r\n");
	uart_puts(Text);
	_delay_ms(1000);
	
	while (ReadPortD(3) == 1){}
	sprintf(Text, "Reading voltage for boiling water.\r\n");
	uart_puts(Text);
	_delay_ms(50);
	v100 = ReadAdc(6);
		
	// calibration calculation	
	sprintf(Text, "Calibrating...\r\n");
	uart_puts(Text);
	_delay_ms(50);
	a1 = 100000/(v100-v0);
	a0 = -(100000*v0)/(v100-v0);
	_delay_ms(1000);
	sprintf(Text, "Calibration completed %d %d %ld %ld!\r\n", v0, v100, a0, a1);
	uart_puts(Text);
	_delay_ms(50);	
	
	// print loop
	while(1)
    {
		TogglePortD(7);
		index++;
		ADCValue=ReadAdc(6);
		sprintf( Text,"measurement nr: %d ADC: %d Ports: %d %d %d\r\n",index,ADCValue,ReadPortD(3),ReadPortD(4),intnum);
		uart_puts (Text); _delay_ms(1000);
		
		// added code
		//voltage = (int16_t)(5*ADCValue)/1023;
		temp = a1*ADCValue+a0;
		sprintf(Text, "Measured temperature: %04ld  degC\r\n", temp/100);
		Text[26] = Text[25];
		Text[25] = '.';
		uart_puts(Text);
		_delay_ms(500);
    }
	
}

ISR(TIMER0_COMP_vect)
/*
Interrupt service routine for timer 0 interrupt.
*/
{
	intnum++;
}