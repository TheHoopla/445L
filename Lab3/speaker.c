/*
 * speaker.c
 *
 *  Created on: Feb 10, 2016
 *      Author: jacobwilliamson
 */
#include "speaker.h"

void output(uint32_t output){
	//output data to correct pin for speaker
}

void soundAlarm(void){
	uint32_t timer = 4000;
	while(timer > 0){
		timer--;
		output(1);
	}
}
