#include "../inc/tm4c123gh6pm.h"
#include <stdint.h>
#include "Sound.h"
#include "ST7735.h"
#include "Pictures.h"
#include "Timer1.h"
#include "TExas.h"

void BulletInit(void);
void BulletMove(void);
void Timer0_Init(void(*task)(void), uint32_t period);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

extern uint8_t sound;
extern uint8_t NoSound;

extern void EnemyDraw(void);
extern void EnemyMove(void);

// Buttons.c
// Use edge triggered interrupts to read PF0 and PF4
// PF0: Clear button and PF4: Drop button

void Button_Init(void){volatile long delay;                            
  SYSCTL_RCGCGPIO_R |= 0x20;           // activate port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
  delay = 100;                  //    allow time to finish activating
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0                              // 2) GPIO Port F needs to be unlocked
  GPIO_PORTF_AMSEL_R &= ~0x11;  // 3) disable analog on PF4,0
                                // 4) configure PF4,0 as GPIO
  GPIO_PORTF_PCTL_R &= ~0x000F000F;
  GPIO_PORTF_DIR_R &= ~0x11;    // 5) make PF4,0 in
  GPIO_PORTF_AFSEL_R &= ~0x11;  // 6) disable alt funct on PF4,0
	GPIO_PORTF_PUR_R |= 0x11;     
  GPIO_PORTF_DEN_R |= 0x11;     // 7) enable digital I/O on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     //    PF4,0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //    PF4,0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;     //    PF4,0 falling edge event (Neg logic)
  GPIO_PORTF_ICR_R = 0x11;      //    clear flag1-0
  GPIO_PORTF_IM_R |= 0x11;      // 8) arm interrupt on PF4,0
                                // 9) GPIOF priority 1
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF0FFFFF)|0x00200000;
  NVIC_EN0_R = 1<<30;   // 10)enable interrupt 30 in NVIC
}

uint8_t status=0;
uint8_t lang;
uint32_t reload1, reload2, reload3, reload4;
uint8_t pause=0;
void GPIOPortF_Handler(void){
	if(status==0){
		if((GPIO_PORTF_RIS_R & 0x10)==0x10){ //PF4
			lang=1; // Spanish
		}
		status++;
		GPIO_PORTF_ICR_R = 0x11;      // acknowledge flag4
	}
	
	if(status==1){
		if(TIMER0_CTL_R !=0){
			if ((GPIO_PORTF_RIS_R & 0x01)==0x01){ //PF0
				if(NoSound==0){ // no bullet on screen currently
				sound=0;
				Sound_Play();
				BulletInit();
				Timer0_Init(&BulletMove, 80000000/200); // 300 Hz
				}
			}
		}
		if((GPIO_PORTF_RIS_R & 0x10)==0x10){ //PF4
			TIMER0_CTL_R ^=1;
			TIMER1_CTL_R ^=1;
			TIMER2_CTL_R ^=1;
			NVIC_ST_CTRL_R ^=1;
		}
		GPIO_PORTF_ICR_R = 0x11;      // acknowledge flag4
	}
}


