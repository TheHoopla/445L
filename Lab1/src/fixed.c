#include "fixed.h"

void ST7735_sDecOut3(int32_t n){
	int x = n;
	if(n > 9999 || n < -9999){
		/// *.***	
		ST7735_OutChar(' '); //  
		ST7735_OutChar('*'); //*
		ST7735_OutChar('.'); //.
		ST7735_OutChar('*'); //*
		ST7735_OutChar('*'); //*
		ST7735_OutChar('*'); //*

	} 
	else{
		if(n < 0){
			ST7735_OutChar('-'); // -
			x = -n;
		}
		else{
			ST7735_OutChar(' '); //
		}
		ST7735_OutChar(x/1000 + '0');
		ST7735_OutChar('.');
		ST7735_OutChar((x/100)%10 + '0');
		ST7735_OutChar(((x/10)%100)%10 + '0');
		ST7735_OutChar((x%10) + '0');
	}
}

void ST7735_uBinOut8(uint32_t n){
	if(n >= 256000){ 
		ST7735_OutChar(' ');
		ST7735_OutChar('*'); 
		ST7735_OutChar('*'); 
		ST7735_OutChar('*');
		ST7735_OutChar('.'); 
		ST7735_OutChar('*');
		ST7735_OutChar('*');
  }
	else{
		ST7735_OutChar(' ');
		uint32_t x = ((n*100)/256);
		uint32_t container1;
		uint32_t container2;
		container1 = (x/10000 + '0');
		if(container1 != '0'){
			ST7735_OutChar(container1);
		}else{
			ST7735_OutChar(' ');
		}
		container2 = ((x/1000)%10 + '0');
		if(container1 != '0' || container2 != '0'){
			ST7735_OutChar(container2);
		}else{
			ST7735_OutChar(' ');
		}
		ST7735_OutChar(((x/100)%100)%10 + '0');
		ST7735_OutChar('.'); //.
		ST7735_OutChar((((x/10)%1000)%100)%10 + '0');
		ST7735_OutChar(x%10 + '0');
	}
}

int32_t Xmin1;
int32_t Xmax1;
int32_t Ymin1;
int32_t Ymax1;

void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, int32_t minY, int32_t maxY){
	Xmin1 = minX;
	Xmax1 = maxX;
	Ymin1 = minY;
	Ymax1 = maxY;                  
  ST7735_OutString(title);
  ST7735_PlotClear(minY, maxY);  // range from 0 to 4095
}

void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]){
		uint32_t j;
	  for(j=0;j<num;j++){
			if(bufX[j] <= Xmax1 && bufX[j] >= Xmin1 && bufY[j] <= Ymax1 && bufY[j] >= Ymin1){
				int32_t X = (127*(bufX[j]-Xmin1))/(Xmax1-Xmin1);
				int32_t Y = 32+(127*(Ymax1-bufY[j]))/(Ymax1-Ymin1); 
        ST7735_DrawPixel(X,   Y,   ST7735_BLACK); 
			}
  }   // called 128 times
}


/***********************************************************************************/
volatile float T;    // temperature in C 
volatile uint32_t N; // 12-bit ADC value 
void Test1(void){
  for(N=0; N<4096; N++){ 
    T = 10.0+
0.009768*N;      
  } 
}

// version 2: C fixed-point 
volatile uint32_t T1;    // temperature in 0.01 C 
volatile uint32_t N;    // 12-bit ADC value 
void Test2(void){ 
  for(N=0; N<4096; N++){ 
    T1 = (1000+(125*N+64))>>7;   
  } 
} 

//;Version 3 assembly floating point 
//; run with floating-point hardware active 
//        AREA    DATA, ALIGN=2 
//T       SPACE   4 
//N       SPACE   4 
//        AREA    |.text|, CODE, READONLY, ALIGN=2 
//        THUMB 
//Test3 
//      MOV R0,#0 
//      LDR R1,=N    ;pointer to N 
//      LDR R2,=T    ;pointer to T 
//      VLDR.F32 S1,=0.009768    
//      VLDR.F32 S2,=10    
//loop3 STR R0,[R1]          ; N is volatile 
//      VMOV.F32 S0,R0 
//      VCVT.F32.U32 S0,S0   ; S0 has N 
//      VMUL.F32 S0,S0,S1    ; N*0.09768 
//      VADD.F32 S0,S0,S2    ; 10+N*0.0968 
//      VSTR.F32 S0,[R2]     ; T=10+N*0.0968 
//      ADD R0,R0,#1 
//      CMP R0,#4096 
//      BNE loop3 
//      BX  LR 
//	  
//;version 4, assembly fixed point 
//        AREA    DATA, ALIGN=2 
//T       SPACE   4 
//N       SPACE   4 
//        AREA    |.text|, CODE, READONLY, ALIGN=2 
//        THUMB 
//Test4 PUSH {R4,R5,R6,LR} 
//      MOV R0,#0 
//      LDR R1,=N   ;pointer to N 
//      LDR R2,=T   ;pointer to T 
//      MOV R3,#125    
//      MOV R4,#64 
//      MOV R5,#1000    
//loop4 STR R0,[R1]          ; N is volatile 
//      MUL R6,R0,R3         ; N*125 
//      ADD R6,R6,R4         ; N*125+64 
//      LSR R6,R6,#7         ; (N*125+64)/12
//	  ADD R6,R6,R5         ; 1000+(N*125+64)/128 
//      STR R6,[R2]          ; T = 1000+(N*125+64)/128 
//      ADD R0,R0,#1 
//      CMP R0,#4096 
//      BNE loop4 
//      POP {R4,R5,R6,PC}

