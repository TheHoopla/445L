#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "switch.h"
#include "timer.h"
#include "speaker.h"
#include "../../Lab1/src/ST7735.c"

//buttonPressed Values
//-1 = waiting
//0 = menu
//1 = scroll
//2 = select
//3 = hour increment
//4 = minute increment

int32_t buttonPressed = -1;
uint32_t timeHours = 0;
uint32_t timeMinutes = 0;
uint32_t alarmHours = 0;
uint32_t alarmMinutes = 0;
uint32_t alarmBool= 0;
uint32_t secondsCount = 0;

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 79999999;         // 1 minute interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;	// acknowledge timer0A timeout
  if(secondsCount < 60){
	  secondsCount++;
  }

}

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
				      break;
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
			          break;
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
				    break;
	}
	buttonPressed = pullCurrentButton();
}

int main(void) {
	uint32_t selection = 0;
	Timer0A_Init100HzInt();
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
			    break;
		}
		buttonPressed = pullCurrentButton();
	}
}
