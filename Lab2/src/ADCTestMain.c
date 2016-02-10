// ADCTestMain.c
// Runs on TM4C123
// This program periodically samples ADC channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// September 5, 2015

//For LCD**************
// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected 
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO)
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V 
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "ADCSWTrigger.h"
#include "../../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "../../Lab1/src/ST7735.c"
#include "../../Lab1/src/fixed.c"

#define LENGTH(x) (sizeof(x)/sizeof(*(x)))
#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

uint32_t time[1000];
uint32_t values[1000];
uint32_t count=0;


volatile uint32_t ADCvalue;
// This debug function initializes Timer0A to request interrupts
// at a 100 Hz frequency.  It is similar to FreqMeasure.c.

uint32_t findJitter(void){
	uint32_t i;
	uint32_t min = UINT_MAX;
	uint32_t max = 0;
	for(i = 0; i < 1000; i++){
		if(time[i]  > max){
			max = time[i];
		}
		if(time[i] < min){
			min = time[i];
		}
	}
	uint32_t max_diff = max - min;
	uint32_t j;
	uint32_t min_diff = UINT_MAX;
  for(i = 0; i < 1000; i++){
		for(j = 0; j < 1000; j++){
			if(j != i && (abs(time[i]-time[j]) < min_diff)){
					min_diff = abs(time[i]-time[j]);
			}
		}
	}
	return max_diff - min_diff;
}

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
  TIMER0_TAILR_R = 799999;         // start value for 100 Hz interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}

void Timer1_Init(void){ 
  volatile uint32_t delay; 
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1 
  delay = SYSCTL_RCGCTIMER_R;   // allow time to finish activating 
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup 
  TIMER1_CFG_R = 0x00000000;    // 2) configure for32-bit mode 
  TIMER1_TAMR_R = 0x00000002;   // 3) configure forperiodic mode, down-count  
  TIMER1_TAILR_R = 0xFFFFFFFF;  // 4) reload value 
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution 
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A 
} 

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;	// acknowledge timer0A timeout
  PF2 ^= 0x04;                   // profile
  PF2 ^= 0x04;                   // profile
  ADCvalue = ADC0_InSeq3();
	if(count < 1000){
	   time[count] = TIMER1_TAR_R;
	   values[count] = ADCvalue;
		 count++;
	}
     PF2 ^= 0x04;   	// profile
}

void PlotPMF(void);
void ST7735_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

int main(void){
  PLL_Init(Bus80MHz);                   // 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
	ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(0);  // set screen to black
	ST7735_SetCursor(0,0);
	Timer1_Init();
  Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts
  GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x06;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x06;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  PF2 = 0;                      // turn off LED
  EnableInterrupts();
  while(1){
    PF1 ^= 0x02;  // toggles when running in main
		if(count == 1000){
			//DisableInterrupts();
			PlotPMF();
			count++;
			//break;
		}
		}
}

void PlotPMF(void){		
	uint32_t i;
	uint32_t min = UINT_MAX;
	uint32_t max = 0;
	for(i = 0; i < 1000; i++){
		if(values[i]  > max){
			max = values[i];
	}
		if(values[i] < min){
			min = values[i];
		}
	}	
	uint32_t num = max - min;
	
	uint32_t freq[1000] = {0};
	uint32_t j;
	for(j = 0; j < max - min; j++){
  	freq[(values[j]-min)]++;  
	}
	uint32_t max_freq = 0;
	for(j = 0; j < num; j++){
		if(freq[j] > max_freq){
			max_freq = freq[j];
		}
	}
	ST7735_XYplotInit("PMF Graph", min, max, 0, max_freq);
	for(j = 0; j < 128; j++){
		ST7735_PlotBar(freq[j]);
		ST7735_PlotNext();
	}
}
	
	void ST7735_Line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color){
		if (x2 > x1){
			if(y2 > y1){
				uint16_t i;
				for(i = x1; i <= x2; i++){
					uint16_t m = (y2 - y1) / (x2 - x1);
					uint16_t b = y1/(m * x1);
					ST7735_DrawPixel(i,(m*i + b),color);
				}
			}
			else{
			}
		}
		else{
			if(y2 < y1){
			}
			else{
			}
		}
	}
