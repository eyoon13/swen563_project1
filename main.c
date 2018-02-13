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
#define NEW_LINE 13


char post_again[] = "Try Post Again? (Y = On, N = off):\r\n\r\n";
char run_again[]  = "Would you like to run another test? (Y = On, N = off):\r\n\r\n";
uint32_t start_time = 0; // Var for POST routine
uint8_t buffer[100];
uint16_t holds[101];
uint16_t low_limit = 950; 


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
	
	// Begin Input Capture
	Run_Capture();
	start_time = (uint32_t)TIM5->CCR1;
	
	while(!finished_flag){
		if(TIM5->SR & 0x2){
			curr_time = (uint32_t)TIM5->CCR1;
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

void get_delta(){

	uint8_t has_reading = 0;
	uint32_t prev_reading = 0;
	uint32_t curr_reading = 0;
	uint32_t delta = 0; 
	uint32_t count = 0;

	Run_Capture();

	while(1){

		if(count >= RUNS) { 
			TIM5->CR1 = 0x0;
			break; 
		}
		if(!has_reading){
			prev_reading = (uint32_t)TIM5->CCR1;
			has_reading = 1;
		} else {
			curr_reading = (uint32_t)TIM5->CCR1;
			delta = curr_reading - prev_reading;
			prev_reading = curr_reading;
			if(delta >= lower_lim && delta <= lower_lim+100){
				holds[delta - lower_lim]++;
				count++;
			}
		}
	}
}

void make_table(){
	int n = 0;

	int table_start = lower_lim;

	USART_Write(USART2, (uint8_t *)"\r\nBuilding Table...\r\n", 25);

	for(int i = 0; i < 101; i++){


		if(holds[i] != 0){
			n = sprintf((char *)buffer, "Time %dus count %d\r\n", table_start, holds[i]);
			USART_Write(USART2, buffer, n);
		}
		table_start+=1;
	}
	// Create a line for spacing
	USART_Write(USART2, (uint8_t *)"\r\n", 4);
}

void update_l_bound(){

	char rxByte;
	char num_value_array[5] = {'\0', '\0', '\0', '\0', '\0'};
	int i = 0;

	n = sprintf((char *)buffer, "Input new lower limit (Value between %d and %d):\r\n", MIN_L_VAL, MAX_L_VAL);
	USART_Write(USART2, buffer, n);

	while((rxByte != NEW_LINE) && (i<5){
		n = sprintf((char *)buffer, "%c", rxByte);
		USART_Write(USART2, buffer, n);
		num_value_array[i] = rxByte;
		i++;
		rxByte = USART_Read(USART2);
	}
	sscanf(inputBuffer, "%d", &lower_lim);
	n = sprintf((char *)buffer, "Updated lower limit: %d\r\n", lower_lim);
	USART_Write(USART2, buffer, n);

}

void set_bounds(){
	int n = 0;
	char rxByte;

	n = sprintf((char *)buffer, "Current Bound %d microseconds. Would you like to update? (Y / N):\r\n", lower_lim);
	USART_Write(USART2, buffer, n);

	do {
		n = sprintf((char *)buffer, "Keep new lower bound?(Y / N):\r\n", 32);
		USART_Write(USART2, buffer, n);
		rxByte = USART_Read(USART2);
	} while (rxByte != 'Y' && rxByte != 'y' && rxByte != 'N' && rxByte != 'n');

	rxByte = USART_Read(USART2);

	if (rxByte == 'N' || rxByte == 'n'){
		lower_lim = 0;
		update_l_bound();
	}
		
	upper_lim = lower_lim + 100;
	
	n = sprintf((char *)buffer, "Limits changed Lower: %d, Upper: %d\r\n", lower_lim, upper_lim);
	USART_Write(USART2, buffer, n);

	n = sprintf((char *)buffer, "Press <Return> key to start\r\n");
	USART_Write(USART2, buffer, n);
	while (USART_Read(USART2) != NEW_LINE){
		;
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

	while(1){
	// Set Bounds
		set_bounds();

	// Get Deltas
		get_delta();

	// Create Table & Send to UART
		make_table();

		USART_Write(USART2, (uint8_t *)run_again, strlen(run_again));

		if (rxByte == 'N' || rxByte == 'n'){
				USART_Write(USART2, (uint8_t *)"Exiting...\r\n", 14);
				break;
		}

	// reset
		for(int i = 0; i < 101; i++){
			results[i] = 0;
		}

	}

}


