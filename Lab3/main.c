#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "switch.h"
#include "timer.h"
#include "speaker.h"
#include "../Lab1/src/ST7735.c"

//buttonPressed Values
//-1 = waiting
//0 = menu
//1 = scroll
//2 = select
//3 = hour increment
//4 = minute increment

uint32_t buttonPressed = -1;
uint32_t timeHours = 0;
uint32_t timeMinutes = 0;
uint32_t alarmHours = 0;
uint32_t alarmMinutes = 0;
uint32_t alarmBool= 0;

void setTime(void){
	//update ST7735 for Set_Time_View
	while(buttonPressed != 0){
		switch(buttonPressed){
			case (3): timeHours++; //increment hours
					  timerUpdate(timeHours, timeMinutes);
			          //update ST7735
					  break;
			case (4): timeMinutes++; //increment minutes
					  timerUpdate(timeHours, timeMinutes);
			          //update ST7735
					  break;
			default:
		}
		buttonPressed = pullCurrentButton();
	}
}

void setAlarm(void){
	//update ST7735 for Set_Alarm_View
	while(buttonPressed != 0){
		//delay for input interrupt
		switch(buttonPressed){
			case (3): alarmHours++; //increment hours
					  //update ST7735
					  break;
			case (4): alarmMinutes++; //increment minutes
					  //update ST7735
					  break;
			default:
		}
		buttonPressed = pullCurrentButton();
	}
}

void alarmToggle(void){
	//update ST7735 for Toggle_Alarm_View
	while(buttonPressed != 0){
		//delay for input interrupt
		if(buttonPressed == 2){
			alarmBool = !alarmBool;
			//update ST7735
		}
		buttonPressed = pullCurrentButton();
	}
}

void screenManager(uint32_t selection){
	switch (selection){
			//delay for input interrupt
			case (0):
					//Set Time
					setTime();
					break;
			case (1):
					//Set Alarm
					setAlarm();
					break;
			case (2):
					//Toggle Alarm On/Off
					alarmToggle();
					break;
			default:
	}
	buttonPressed = pullCurrentButton();
}

int main(void) {
	uint32_t selection = 0;
	while(1){
		switch (buttonPressed){
		//delay
		case (1):
				//Scroll Selection Down
				if(selection >= 2){
					selection = 0;
					//update ST7735
				}
				else{
					selection++;
					//update ST7735
				}
				break;
		case (2):
				//Selects Current Menu Option
				screenManager(selection);
				break;
		default:
		}
		buttonPressed = pullCurrentButton();
	}
}
