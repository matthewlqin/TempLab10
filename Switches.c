#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "switches.h"

void RedSwitch_Init(void){ // English
volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x10; //turn on Port E 
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_DIR_R &= ~(0x01); //PE0 is input
	GPIO_PORTE_DEN_R |= 0x01;
}

void BlueSwitch_Init(void){ // Spanish
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x10; //turn on Port E 
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_DIR_R &= ~(0x02); //PE0 is input
	GPIO_PORTE_DEN_R |= 0x02;
}
