#include <string.h>
#include <stdio.h>
#include "UART.h"
#include "stm32l476xx.h"
#include "SysClock.h"
#include "LED.h"
#include "input_pa0_test.h"

#define CLK_SPEED 80000000  // This is the 80Mhz
#define MAX_L_VAL 9950
#define MIN_L_VAL 50
#define RUNS 1000
#define POST_OK 100000


char post_again[] = "Try Post Again? (Y = On, N = off):\r\n\r\n";
uint32_t start_time = 0; // Var for POST routine


void try_post_again(){
	char rxByte;
	USART_Write(USART2,(uint8_t *)post_again, strlen(post_again));
	rxByte = USART_Read(USART2);
	
	if (rxByte == 'N' || rxByte == 'n'){
			USART_Write(USART2, (uint8_t *)"Powering Off\r\n\r\n", 16);
		}
		else if (rxByte == 'Y' || rxByte == 'y'){
			start_time = (uint32_t)TIM5->CCR1; 
		}
}



/**
	Power on Self Test
**/
void POST(void){
	
	uint8_t finished_flag = 0;
	uint16_t curr_time = 0;
	
	USART_Write(USART2,(uint8_t *)"Power On Self Test... \r\n", 27);
	
	// start input capture
	Run_Capture();
	start_time = (uint32_t)TIM5->CCR1;
	
	while(!finished_flag){
		
		//Have reading in CCR1
		if(TIM5->SR & 0x2){
			curr_time = (uint32_t)TIM5->CCR1;
			
			// check if less than 100 ms
			if(curr_time - start_time <= POST_OK){
				finished_flag = 1;
				USART_Write(USART2,(uint8_t *)"Power On Self Test OK \r\n", 26);
				break;
			} else{
				USART_Write(USART2,(uint8_t *)"Power On Self Test Failed \r\n", 30);
				try_post_again();
			}
		}
		TIM5->CR1 &= 0x0;
	}
	
	
	
}


int main(void){
	char rxByte;
	
	// Set Default lower bound
	int lower_lim = 950;
	int upper_lim = lower_lim + 100;
	
	// Init all peripherals
	System_Clock_Init();
	init_pa0();
	init_tim5_clock(); 
  UART2_Init(); 
	
	POST();
}


