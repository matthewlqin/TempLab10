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
#include "Timer2.h"
#include "Buttons.h"
#include "TExas.h"

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
		// **************GAME OVER***********************************
	}	
}

void PlayerInit(void){
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

///********************************************************************************************
struct bullet{
	int32_t Bx;
	int32_t By;
	int32_t Bvx, Bvy; //pixels/30Hz
	const unsigned short *Bimage;
	const unsigned short *Bblack;
	uint32_t Bw;
	uint32_t Bh;
	status_t Blife;
};

typedef struct bullet bullet_t;
bullet_t Bullet;

void BulletDraw(void){
	if(Bullet.Blife == alive){
		ST7735_DrawBitmap(Bullet.Bx, Bullet.By, Bullet.Bimage, Bullet.Bw, Bullet.Bh);
	}
	else{
		ST7735_DrawBitmap(Bullet.Bx, Bullet.By, Bullet.Bblack, Bullet.Bw, Bullet.Bh);
	}	
}

void BulletMove(void){
	ST7735_DrawBitmap(Bullet.Bx, Bullet.By, Bullet.Bblack, Bullet.Bw, Bullet.Bh);
	Bullet.By -= Bullet.Bvy;
	BulletDraw();
}

void BulletInit(void){
	Bullet.Bx=(Player.x)+8; 
	Bullet.By=(Player.y)-7;
	Bullet.Bvx=0;
	Bullet.Bvy=1;
	Bullet.Bimage=bullet;
	Bullet.Bblack=blackBullet;
	Bullet.Bw=1;
	Bullet.Bh=7;
	Bullet.Blife=alive;
}

//*********************************************************************************************
uint32_t Score=0;
struct enemy{
	int32_t Ex;
	int32_t Ey;
	int32_t Evx, Evy;
	const unsigned short *Eimage;
	const unsigned short *Eblack;
	status_t life;
	uint32_t Ew;
	uint32_t Eh;
	uint32_t needDraw;
	uint32_t points;
};

typedef struct enemy enemy_t;
enemy_t Enemy[6][3];
int Anyalive; //determines if game should be over or not

void EnemyInit(void){ int i, j;
	for(i=0;i<6;i++){
		for(j=0; j<3; j++){
			Enemy[i][j].Ex = 20*i;
			Enemy[i][j].Ey = (25*j)+15;
			Enemy[i][j].Evx = 0;
			Enemy[i][j].Evy = 1;
			if(j==0){
				Enemy[i][j].Eimage = SmallEnemy10pointA;
				Enemy[i][j].points = 10;
			}
			if(j==1){
				Enemy[i][j].Eimage = SmallEnemy20pointA;
				Enemy[i][j].points = 20;
			}
			if(j==2){
				Enemy[i][j].Eimage = SmallEnemy30pointA;
				Enemy[i][j].points = 30;
			}
			Enemy[i][j].Eblack = BlackEnemy;
			Enemy[i][j].life = alive;
			Enemy[i][j].Ew = 16;
			Enemy[i][j].Eh= 10;
			Enemy[i][j].needDraw =1;
		}
	}
}

void EnemyDraw(void){ int i, j;
	if(Anyalive==1){
				if(Enemy[i][j].needDraw==1){
					if(Enemy[i][j].life == alive){
						ST7735_DrawBitmap(Enemy[i][j].Ex, Enemy[i][j].Ey, Enemy[i][j].Eimage, Enemy[i][j].Ew, Enemy[i][j].Eh);
					}
					if(Enemy[i][j].life == dead){
						ST7735_DrawBitmap(Enemy[i][j].Ex, Enemy[i][j].Ey, Enemy[i][j].Eblack, Enemy[i][j].Ew, Enemy[i][j].Eh);
						Enemy[i][j].needDraw = 0;
					}
				}
			}
	if(Anyalive==0){
		for(i=0;i<6;i++){
			for(j=0; j<3; j++){
				if(Enemy[i][j].life == dead){
						ST7735_DrawBitmap(Enemy[i][j].Ex, Enemy[i][j].Ey, Enemy[i][j].Eblack, Enemy[i][j].Ew, Enemy[i][j].Eh);
						Enemy[i][j].needDraw = 0;
				}
			}
		}
		// ******************GAME OVER*******************************
	}
}

void EnemyMove(void){ int i, j;
	Anyalive=0;
	for(i=0;i<6;i++){
		for(j=0; j<3; j++){
			if(Enemy[i][j].Ey>150){
					Enemy[i][j].life = dead;
			}
			if(Enemy[i][j].life == alive){ // checks laser shot
				if((((Enemy[i][j].Ey+20) >= Bullet.By)&&(Enemy[i][j].Ey <= Bullet.By))&&((Enemy[i][j].Ex <= Bullet.Bx) && ((Enemy[i][j].Ex+16) >= Bullet.Bx))){
					Enemy[i][j].life = dead;
					TIMER0_IMR_R = 0X00000000;
					ST7735_DrawBitmap(Bullet.Bx, Bullet.By, Bullet.Bblack, Bullet.Bw, Bullet.Bh);
					Score += Enemy[i][j].points;
				}
			}
			if(Enemy[i][j].life == alive){ // checks with player
				if(Player.x <= Enemy[i][j].Ex){
					if((Player.x+18) >= Enemy[i][j].Ex){
						if((Player.y-10) <= Enemy[i][j].Ey){
							Player.life = dead;
							GameDraw();
						}	
					}
				}
			}
			Anyalive=1;
			Enemy[i][j].Ey += Enemy[i][j].Evy;
			EnemyDraw();
		}
	}
}

//*********************************************************************************************

int main(void){
	Button_Init(); // enables edge triggered interrupts
  Random_Init(1);
	ADC_Init();
	DAC_Init();

  Output_Init();
  /*ST7735_FillScreen(0x0000);            // set screen to black
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
*/
	ST7735_FillScreen(0x0000);
	PlayerInit();
	EnableInterrupts();
	SysTick_Init(80000000/30);
  
  ST7735_DrawBitmap(53, 140, Bunker0, 18,5);
	
	EnemyInit();
	Timer2_Init(&EnemyMove, 80000000/10);
	
	while(1){}

  ST7735_FillScreen(0x0000);   // set screen to black
	
	if(lang==1){
		ST7735_SetCursor(1, 1);
		ST7735_OutString("FIN DEL JUEGO");
		ST7735_SetCursor(1, 2);
		ST7735_OutString("Resultado:");
		ST7735_SetCursor(11, 2);
		LCD_OutDec(Score);
	}
	else{
		ST7735_SetCursor(1, 1);
		ST7735_OutString("GAME OVER");
		ST7735_SetCursor(1, 2);
		ST7735_OutString("Score:");
		ST7735_SetCursor(7, 2);
		LCD_OutDec(Score);
	}
}
