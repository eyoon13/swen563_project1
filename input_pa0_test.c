// L. Kiser Feb. 8, 2017

#include "stm32l476xx.h"
#include "input_pa0_test.h"

// Turn on the peripheral clock for GPIOA
void init_pa0( void )
{
	RCC->AHB2ENR |=   RCC_AHB2ENR_GPIOAEN;
	GPIOA->MODER &= ~3 ;										// clear out bits 0 and 1 for PA0
	GPIOA->MODER |= 0x00000002 ;									// Enable alternate function mode (binary 10) for PA0
	GPIOA->AFR[0] |= 2; 								// Set for Timer5 CH 1  (AF2 of Pin PA0 on GPIOA)
}