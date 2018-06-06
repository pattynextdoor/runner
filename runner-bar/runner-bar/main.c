/*
 * CS120B_FinalProject.c
 *
 * Created: 5/22/2018 6:57:46 PM
 * Author : Patrick T
 */ 

#include <avr/io.h>
#include <avr/eeprom.h>
#include <timer.h>
#include <bit.h>
#include <usart_ATmega1284.h>


enum lightStates { light_init, light_tick } lightState;
	
unsigned long refreshRate = 50;
unsigned char tmpA;
unsigned char forwards = 1;
unsigned char signalSent = 0;
uint16_t joystickVal;


void set_PWM(double frequency) {
	// 0.954 hz is lowest frequency possible with this function,
	// based on settings in PWM_on()
	// Passing in 0 as the frequency will stop the speaker from generating sound
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		
		// prevents OCR3A from underflowing, using prescaler 64
		// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		
		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

// initialize adc
void adc_init()
{
	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// read adc value
uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
	
	// start single conversion
	// write '1' to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

enum stickStates { stick_init, stick_wait, stick_left, stick_right } stickState;
	
void stickTick() 
{
	switch(stickState)
	{
		case stick_init:
			stickState = stick_wait;
			break;
			
		case stick_wait:
			joystickVal = adc_read(0);
			// Right
			if(joystickVal > 500) {
				PORTB = 0x01;
				if(USART_IsSendReady(0)) {
					USART_Send((unsigned char)8, 0);
				}
				stickState = stick_right;
				break;
			}
			// Left
			else if (joystickVal < 300) {
				PORTB = 0x02;
				if(USART_IsSendReady(0)) {
					USART_Send((unsigned char)7, 0);
				}
				stickState = stick_left;
				break;
			}
			// Neutral
			else {
				PORTB = 0x00;
				break;
			}
			
		case stick_left:
			joystickVal = adc_read(0);
			
			if(joystickVal > 300) {
				stickState = stick_wait;
			}
			
			break;
			
		case stick_right:
			joystickVal = adc_read(0);
			
			if(joystickVal < 500) {
				stickState = stick_wait;
			}
			
			break;
	}	
}

void lightTick() 
{
	switch(lightState)
	{
		case light_init:
			lightState = light_tick;
			tmpA = PORTA;
			break;
			
		case light_tick:
			if (tmpA * 2 <= 0xFF && forwards)
			{
				tmpA *= 2;
			}
			else if (tmpA * 2 > 0xFF)
			{
				forwards = !forwards;
			}
			
			if (tmpA / 2 >= 0x04 && !forwards)
			{
				tmpA /= 2;
			}
			else if (tmpA / 2 < 0x04)
			{
				forwards = !forwards;
			}
			
			break;
	}
	PORTA = tmpA;
}

enum lightSpeedStates { speed_init, speed1, speed2, speed3, speed4, speed5 } lightSpeedState;

void lightSpeedTick() 
{
	switch(lightSpeedState)
	{
		case speed_init:
			lightSpeedState = speed1;
			break;
			
		case speed1:
			refreshRate = 40;
			lightSpeedState = speed2;
			while(!USART_IsSendReady(0)) {};
			USART_Send('a', 0);
			break;
			
		case speed2:
			refreshRate = 30;
			lightSpeedState = speed3;
			while(!USART_IsSendReady(0)) {};
			USART_Send('b', 0);

			break;
			
		case speed3:
			refreshRate = 20;
			lightSpeedState = speed4;
			while(!USART_IsSendReady(0)) {};
			USART_Send('c', 0);
			break;
			
		case speed4:
			refreshRate = 10;
			lightSpeedState = speed5;
			while(!USART_IsSendReady(0)) {};
			USART_Send('d', 0);
			break;
		
		case speed5:
			refreshRate = 5;
			while(!USART_IsSendReady(0)) {};
			USART_Send('e', 0);
			break;
					
	}
	USART_Flush(0);
}


int main(void)
{
	adc_init();
	
	DDRA = 0xFC; PORTA = 0x04;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0x00; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	unsigned long transitionTime = 1000;
	
	unsigned long light_timeElapsed = 0;
	unsigned long speed_timeElapsed = 0;
	unsigned long player_timeElapsed = 0;
	
	initUSART(0);
	USART_Flush(0);
		
	TimerSet(refreshRate);
	TimerOn();
	
	stickState = stick_init;
	lightState = light_init;
	lightSpeedState = speed_init;

	unsigned long highScore = eeprom_read_float((float*)46);

    while (1) 
    {
		stickTick();
		if (speed_timeElapsed >= transitionTime)
		{
			lightSpeedTick();
			TimerSet(refreshRate);
			speed_timeElapsed = 0;
		}
		if (light_timeElapsed >= refreshRate)
		{
			lightTick();
			light_timeElapsed = 0;			
		}
		
		if(USART_HasReceived(0)) {
			if(player_timeElapsed > highScore) {
				eeprom_write_float((float*)46, (float)player_timeElapsed);				
			}
		}
		while (!TimerFlag) {}
		TimerFlag = 0;
		player_timeElapsed += refreshRate;
		speed_timeElapsed += refreshRate;
		light_timeElapsed += refreshRate;
    }
}

