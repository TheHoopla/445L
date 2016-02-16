#include "switch.h"

void Switch_Init(void){
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000008;     // 1) activate clock for Port D
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  	  	  	  	  	  	  	  	   // 2) no need to unlock GPIO PD0-3
  GPIO_PORTD_AMSEL_R = 0x00;        // 3) disable analog on PD0-3
  GPIO_PORTD_PCTL_R = 0x00;		   // 4) PCTL GPIO on PD0-3
  GPIO_PORTD_DIR_R &= ~0x0F;        // 5) direction PD0-3 input
  GPIO_PORTD_AFSEL_R &= ~0x0F;      // 6) PD0-3 regular port function
  GPIO_PORTD_DEN_R |= 0x0F;         // 7) enable PD0-3 digital port
}

unsigned long Switch_Input(void){
  return (GPIO_PORTD_DATA_R&0x0F);
}

int pullCurrentButton(void){
	unsigned long buttonArray = Switch_Input();
	switch(buttonArray){
		case(0x01): return 0;
		case(0x02): return 1;
		case(0x04): return 2;
		case(0x08): return 3;
		default:		return -1;
	}
 }

