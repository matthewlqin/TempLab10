// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/17/2020 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2019

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2019

 Copyright 2019 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "DAC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Buttons.h"
#include "TExaS.h"

extern void Delay1ms(uint32_t n);
extern int8_t lang;

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

typedef enum {dead, alive} status_t;
struct player{
	int32_t x;
	int32_t y;
	int32_t vx, vy; //pixels/30Hz
	const unsigned short *image;
	const unsigned short *black;
	uint32_t w;
	uint32_t h;
	status_t life;
};

typedef struct player player_t;
int Flag;
player_t Player;

void GameDraw(void){
	if(Player.life == alive){
		ST7735_DrawBitmap(Player.x, Player.y, Player.image, Player.w, Player.h);
	}
	else{
		ST7735_DrawBitmap(Player.x, Player.y, Player.black, Player.w, Player.h);
	}	
}

void GameInit(void){
	Player.x=53; 
	Player.y=151;
	Player.vx=0;
	Player.vy=0;
	Player.image=PlayerShip0;
	Player.black=BlackShip;
	Player.w=18;
	Player.h=8;
	Player.life=alive;
	GameDraw();
}

void GameMove(void){ //  checks if player is alive to move
		// ******IF STATEMENTS TO CHECK POSITION OF ENEMIES IN RELATION TO PLAYER*******
		// if(enemy in same x and y range as top of player then player.life=dead)
	//else{
			Player.x = Player.vx; // Player.vx updated in background thread
		//}
	GameDraw();
}

void SysTick_Init(uint32_t period){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = period-1; 
	NVIC_ST_CURRENT_R = 0;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x40000000; // set priority to 2
	NVIC_ST_CTRL_R = 7;
}

uint32_t ADCMail;
void SysTick_Handler(void){
	ADCMail = ADC_In();
	ADCMail = ADCMail/37; // convert to position on LCD
	Player.vx= ADCMail;
	ST7735_DrawBitmap(Player.x, Player.y, Player.black, Player.w, Player.h);
	GameMove();
}

int main(void){
	Button_Init(); // enables edge triggered interrupts
  Random_Init(1);
	ADC_Init();
	DAC_Init();

  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
	ST7735_SetCursor(1, 1);
  ST7735_OutString("Choose a language"); // display language options
	ST7735_SetCursor(1, 2);
	ST7735_OutString("Elegir un idioma");
	ST7735_SetCursor(1, 4);
  ST7735_OutString("Button 2: English"); //PF0
	ST7735_SetCursor(1, 5);
	ST7735_OutString("Bot\xA2n 1: Espa\xA4ol"); //PF4
	
	while((GPIO_PORTF_DATA_R & 0x11)==0x11){ // wait for button for language to be pressed
	}

	GameInit();
  ST7735_FillScreen(0x0000); 
	SysTick_Init(80000000/30);
	EnableInterrupts();
  
  ST7735_DrawBitmap(53, 140, Bunker0, 18,5);

  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);

	while(1){}

  ST7735_FillScreen(0x0000);   // set screen to black
	// ********DISPLAY SCORE IN LANGUAGE SELECTED********
	
	if(lang==1){
		ST7735_SetCursor(1, 1);
		ST7735_OutString("FIN DEL JUEGO");
		ST7735_SetCursor(1, 2);
		ST7735_OutString("Resultado:");
		ST7735_SetCursor(11, 2);
		LCD_OutDec(1234);
	}
	else{
		ST7735_SetCursor(1, 1);
		ST7735_OutString("GAME OVER");
		ST7735_SetCursor(1, 2);
		ST7735_OutString("Score:");
		ST7735_SetCursor(7, 2);
		LCD_OutDec(1234);
	}
  
}
