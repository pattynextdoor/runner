/*
 * CS120B_FinalProject.c
 *
 * Created: 5/22/2018 6:57:46 PM
 * Author : Patrick T
 */ 

#include <avr/io.h>
#include <timer.h>
#include <bit.h>

enum lightStates { light_init, light_tick } lightState;
	
unsigned long refreshRate = 50;
unsigned char tmpA;
unsigned char forwards = 1;

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
			break;
			
		case speed2:
			refreshRate = 30;
			lightSpeedState = speed3;
			break;
			
		case speed3:
			refreshRate = 20;
			lightSpeedState = speed4;
			break;
			
		case speed4:
			refreshRate = 10;
			lightSpeedState = speed5;
			break;
		
		case speed5:
			refreshRate = 5;
			break;
					
	}
}


int main(void)
{
	DDRA = 0xFF; PORTA = 0x04;
	
	unsigned long transitionTime = 1000;
	
	unsigned long light_timeElapsed = 0;
	unsigned long speed_timeElapsed = 0;
		
	TimerSet(refreshRate);
	TimerOn();
	
    while (1) 
    {
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
		while (!TimerFlag) {}
		TimerFlag = 0;
		speed_timeElapsed += refreshRate;
		light_timeElapsed += refreshRate;
    }
}

