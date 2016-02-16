#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "switch.h"
#include "timer.h"
#include "speaker.h"
#include "PLL.h"
#include "../../Lab1/src/ST7735.c"
#include "../../inc/tm4c123gh6pm.h"

int buttonPressed = -1;
int timeHours = 0;
int timeMinutes = 0;
int alarmHours = 0;
int alarmMinutes = 0;
int alarmBool= 0;
int secondsCount = 0;

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
  TIMER0_TAILR_R = 39999999;  // 1 second interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}

void alarmActivate(void){
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("   WAKE   \n");
	int i;
	i = 0;
	while(i < 2500000){
		i++;
	}
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("  ME  \n");
	i = 0;
	while(i < 2500000){
		i++;
	}
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("  UP  \n");
	i = 0;
	while(i < 2500000){
		i++;
	}
	ST7735_SetCursor(0,0);
	ST7735_FillScreen(0);
	ST7735_OutString("WAKE ME UP INSIDE\n");
	i = 0;
	while(i < 4000000){
		i++;
	}
	ST7735_FillScreen(0);
	buttonPressed = 0;
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;	// acknowledge timer0A timeout
  if(secondsCount < 1){
	  secondsCount++;
  }
  else{
	  secondsCount = 0;
	  timeMinutes++;
	  if(timeMinutes == 60){
		  timeMinutes = 0;
		  timeHours++;
		  if(timeHours == 24){
			  timeHours = 0;
		  }
	  }
	  if(timeMinutes == alarmMinutes && timeHours == alarmHours && alarmBool){
		  buttonPressed = 0;
		  alarmActivate();
	  }
  }
}


void outputClock(int hours, int minutes, int isHourSelected){
		 char hourStr[2];
		  char minStr[3];
		  if(minutes < 10){
			  minStr[0] = '0';
			  minStr[1] = minutes + '0';
		  }
		  else{
			  minStr[0] = (minutes/10) + '0';
			  minStr[1] = (minutes % 10) + '0';
		  }
		  if(hours < 10){
			  hourStr[0] = '0';
			  hourStr[1] = hours + '0';
		  }
		  else{
			  hourStr[0] = (hours/10) + '0';
			  hourStr[1] = (hours % 10) + '0';
		  }
		  //minStr[2] = ' ';
		  ST7735_SetTextColor(ST7735_YELLOW);
		  ST7735_SetCursor(0,1);
		  ST7735_OutString("Time: ");
		  if(isHourSelected){
			  ST7735_OutString("[");
		  }
		  ST7735_OutString(hourStr);
		  if(isHourSelected){
			  ST7735_OutString("]");
		  }
		  ST7735_SetTextColor(ST7735_YELLOW);
		  ST7735_OutChar(':');
		  if(!isHourSelected){
			  ST7735_OutString("[");
		  }
		  ST7735_OutString(minStr);
		  if(!isHourSelected){
			  ST7735_OutString("]");
		  }
}

void setTime(void){
	//update ST7735 for Set_Time_View
	ST7735_OutString("*** SET TIME ***\n");
	int isHourSelected = 1;
	while(buttonPressed != 0){
		switch(buttonPressed){
			case (1): //Toggle Minute or Hour
					  isHourSelected = !isHourSelected;
					  break;
			case (2): //Increment
					  if(isHourSelected){
					  	  timeHours++;
					  	  timeHours %= 24;
					  }
					  else{
						  timeMinutes++;
						  timeMinutes %= 60;
					  }
					  break;
			case (3): //Decrement
					  if(isHourSelected){
						 timeHours--;
					     if(timeHours < 0){
					    	 	 timeHours = 24;
					     }
					   }
					   else{
						 timeMinutes--;
					     if(timeMinutes < 0){
					    	 	 timeMinutes = 60;
					     }
					   }
					  break;
			default:
				      break;
		}
		int i = 0;
		while(i < 500000){
			i++;
		}
		if(buttonPressed != 0){
			buttonPressed = pullCurrentButton();
		}
		outputClock(timeHours,timeMinutes,isHourSelected);
	}
	EnableInterrupts();
	ST7735_FillScreen(0);
	ST7735_SetCursor(0,0);
	ST7735_OutString("***  MENU  ***\n");
	ST7735_OutString("---> Set Time\n");
	ST7735_OutString("Set Alarm\n");
	ST7735_OutString("Alarm Toggle\n");
}

void setAlarm(void){
	//update ST7735 for Set_Time_View
	ST7735_OutString("Set Alarm\n");
	int isHourSelected = 0;
	while(buttonPressed != 0){
		switch(buttonPressed){
			case (1): //Toggle Minute or Hour
					  isHourSelected = !isHourSelected;
					  break;
			case (2): //Increment
					  if(isHourSelected){
					  	  alarmHours++;
					  	alarmHours %= 24;
					  }
					  else{
						  alarmMinutes++;
						  alarmMinutes %= 60;
					  }
					  break;
			case (3): //Decrement
					  if(isHourSelected){
						  alarmHours--;
					     if(alarmHours < 0){
					    	    alarmHours = 24;
					     }
					   }
					   else{
						 alarmMinutes--;
					     if(alarmMinutes < 0){
					    	 	 alarmMinutes = 60;
					     }
					   }
					  break;
			default:
					  break;
		}
		int i = 0;
		while(i < 500000){
			i++;
		}
		if(buttonPressed != 0){
			buttonPressed = pullCurrentButton();
		}
		outputClock(alarmHours,alarmMinutes,isHourSelected);
	}
	ST7735_FillScreen(0);
	ST7735_SetCursor(0,0);
	ST7735_OutString("***  MENU  ***\n");
	ST7735_OutString("---> Set Time\n");
	ST7735_OutString("Set Alarm\n");
	ST7735_OutString("Alarm Toggle\n");
	alarmBool = 1;
}

void alarmToggle(void){
	//update ST7735 for Toggle_Alarm_View
	ST7735_FillScreen(0);
	ST7735_SetCursor(0,0);
	ST7735_OutString("*** ALARM TOGGLE ***\n");
	if(!alarmBool){
		ST7735_OutString("Alarm On\n");
		ST7735_OutString("[Alarm Off]\n");
	}
	else{
		ST7735_OutString("[Alarm On]\n");
		ST7735_OutString("Alarm Off\n");
	}
	while(buttonPressed != 0){
		//delay for input interrupt
		if(buttonPressed == 2 || buttonPressed == 3){
			if(alarmBool){
				ST7735_SetCursor(0,0);
				ST7735_FillScreen(0);
				ST7735_OutString("*** ALARM TOGGLE ***\n");
				ST7735_OutString("Alarm On\n");
				ST7735_OutString("[Alarm Off]\n");
				alarmBool = 0;
			}
			else{
				ST7735_SetCursor(0,0);
				ST7735_FillScreen(0);
				ST7735_OutString("*** ALARM TOGGLE ***\n");
				ST7735_OutString("[Alarm On]\n");
				ST7735_OutString("Alarm Off\n");
				alarmBool = 1;
			}
		}
		int i = 0;
		while(i < 500000){
			i++;
		}
		if(buttonPressed != 0){
			buttonPressed = pullCurrentButton();
		}
	}
	ST7735_FillScreen(0);
	ST7735_SetCursor(0,0);
	ST7735_OutString("***  MENU  ***\n");
	ST7735_OutString("---> Set Time\n");
	ST7735_OutString("Set Alarm\n");
	ST7735_OutString("Alarm Toggle\n");
}

void screenManager(int selection){
	ST7735_FillScreen(0);
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
	int i = 0;
	while(i < 100000){
		i++;
	}
	buttonPressed = pullCurrentButton();
}

int main(void) {
	PLL_Init(Bus80MHz);
	Switch_Init();
	int selection = 0;
	Timer0A_Init100HzInt();
	ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(0);
	ST7735_SetCursor(0,0);
	ST7735_OutString("***  MENU  ***\n");
	ST7735_OutString("---> Set Time\n");
	ST7735_OutString("Set Alarm\n");
	ST7735_OutString("Alarm Toggle\n");
	while(1){
		//buttonPressed Values
		//-1 = waiting
		//0 = menu
		//1 = select
		//2 = scroll down/decriment
		//3 = scroll up/increment
		switch (buttonPressed){
		case (1):
				//Selects Current Menu Option
				screenManager(selection);
				selection = 0;
				break;
		case (3):
				//Scroll Selection Down
				if(selection >= 2){
					selection = 0;
				}
				else{
					selection++;
				}
				ST7735_SetCursor(0,0);
				ST7735_FillScreen(0);
				ST7735_OutString("***  MENU  ***\n");
				switch (selection){
						case (0):
								ST7735_OutString("---> Set Time\n");
								ST7735_OutString("Set Alarm\n");
								ST7735_OutString("Alarm Toggle\n");
								break;
						case (1):
								ST7735_OutString("Set Time\n");
								ST7735_OutString("---> Set Alarm\n");
								ST7735_OutString("Alarm Toggle\n");
							    break;
						case (2):
								ST7735_OutString("Set Time\n");
								ST7735_OutString("Set Alarm\n");
								ST7735_OutString("---> Alarm Toggle\n");
							    break;
						default:
								ST7735_OutString("break\n");
								break;
				}
				break;
		case (2):
				//Scroll Selection Up
				if(selection <= 0){
					selection = 2;
				}
				else{
					selection--;
				}
				ST7735_SetCursor(0,0);
				ST7735_FillScreen(0);
				ST7735_OutString("***  MENU  ***\n");
				switch (selection){
						case (0):
								ST7735_OutString("---> Set Time\n");
								ST7735_OutString("Set Alarm\n");
								ST7735_OutString("Alarm Toggle\n");
								break;
						case (1):
								ST7735_OutString("Set Time\n");
								ST7735_OutString("---> Set Alarm\n");
								ST7735_OutString("Alarm Toggle\n");
							    break;
						case (2):
								ST7735_OutString("Set Time\n");
								ST7735_OutString("Set Alarm\n");
								ST7735_OutString("---> Alarm Toggle\n");
							    break;
						default:
								ST7735_OutString("break\n");
								break;
				}
				break;
		default:
			    break;
		}
		int i = 0;
		while(i < 100000){
			i++;
		}
		buttonPressed = pullCurrentButton();
		ST7735_SetCursor(0,0);
	}
}
