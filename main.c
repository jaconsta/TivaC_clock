//! Clock, main.c
/********************************************//**
 * 
 * Base logic to run the clock in seconds, minutes and hours
 * 
 *
 * Minutes run in 60 minutes binary format.
 * Hours run in 12 hour decade mode.
 * 
 * Designed for the Texas Instruments Tiva C launchpad
 * ARM Cortex M4 tm4c123gh6pm microcontroller.
 * 
 * Seconds use the Port F 3 pin
 * Minutes use the Port A 2 - 7 pins
 * Hour unit use the Port E 0-3 pins
 * Hour tens use the Port F 2 pin
 * 
 * Author: Javier Constain
 * Creation Date: October 2014
 * 
 ***********************************************/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
//#include "inc/hw_types.h"
#include "driverlib/gpio.h"

// Prototype function
void portA_init(void);
void portE_init(void);
void portF_init(void);
void shiftMinutes(void);
void shiftHours(void);
void flashSOS(void);
void delay(unsigned long sec);
void enable_interrupts(void);

/** Global variables */
unsigned int portA_advance = 0x04;
unsigned int portE_advance = 0x01;

//*****************************************************************************
//
// Flags that contain the current value of the interrupt indicator as displayed
// on the UART.
//
//*****************************************************************************
uint32_t g_ui32Flags;

int main(void){
	//
	// Enable lazy stacking for interrupt handlers.  This allows floating-point
	// instructions to be used within interrupt handlers, but at the expense of
	// extra stack usage.
	//
	ROM_FPULazyStackingEnable();

	//
	// Set the clocking to run directly from the crystal.
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
										 SYSCTL_XTAL_16MHZ);
	
	// Enable the peripherals used by this example.
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	// Enable processor interrupts.
	ROM_IntMasterEnable();

	// Configure the two 32-bit periodic timers.
	ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet());

	// Setup the interrupts for the timer timeouts.
	ROM_IntEnable(INT_TIMER0A);
	ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	// Enable the timers.
	ROM_TimerEnable(TIMER0_BASE, TIMER_A);

	portA_init();
	portE_init();
	portF_init();
	GPIO_PORTA_DATA_R |= portA_advance;
	//delay(1);
	GPIO_PORTE_DATA_R |= portE_advance;
	while (1){
 	}

}

/** Initialize port A. */
void portA_init(void) {
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000001;		// 1. Enable Port Clock
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTA_CR_R |= 0xFC;		// Allow changes to P7A-2
	GPIO_PORTA_AMSEL_R &= ~0xFC;		// 3. Disable analog function
	GPIO_PORTA_PCTL_R &= ~0xFFFFFF00;	// 4. GPIO clear bit PCTL
	GPIO_PORTA_DIR_R |= 0xFC;		// 5.2 P7A-2 as output
	GPIO_PORTA_AFSEL_R &= ~0xFC;		// 6 Disable alternate function
	GPIO_PORTA_DEN_R |= 0xFC;		// 7. Enable digital pins P7A-2
}
/** Initialize port E. */
void portE_init(void){
	volatile unsigned long delay;
	
	SYSCTL_RCGC2_R |= 0x10; 		// 1. Enable port clock
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTE_CR_R |= 0x0F;		// Allow changes to PF3-0
	GPIO_PORTE_AMSEL_R &= 0x00;		// 3. Disable analog function
	GPIO_PORTE_PCTL_R &= 0x00000000;	// 4. GPIO clear bit PCTL
	GPIO_PORTE_DIR_R |= 0x0F;		// 5.2 PE3-0 as output
	GPIO_PORTE_AFSEL_R &= 0x00;		// 6 Disable alternate function
	GPIO_PORTE_DEN_R |= 0x0F;		// 7. Enable digital pins PE3-0
}
/** Initialize port F. */
void portF_init(void) {
	volatile unsigned long delay;
	// For this microcontroller is a 7 step process to set each port
	SYSCTL_RCGC2_R |= 0x00000020;		// 1. Enable Port Clock
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTF_LOCK_R = 0x4C4F434B;		// 2. Unlock port F PF0
	GPIO_PORTF_CR_R |= 0x1F;		// Allow changes to PF4-0
	GPIO_PORTF_AMSEL_R &= 0x00;		// 3. Disable analog function
	GPIO_PORTF_PCTL_R &= 0x00000000;	// 4. GPIO clear bit PCTL
	GPIO_PORTF_DIR_R	&= ~0x11;	// 5.1 PF4, PF0 as inputs
	GPIO_PORTF_DIR_R |= 0x0E;		// 5.2 PF3 as output
	GPIO_PORTF_AFSEL_R &= 0x00;		// 6 Disable alternate function
	GPIO_PORTF_PUR_R &= 0x11;		// Enable pull-up resistors on PF4, PF0
	GPIO_PORTF_DEN_R |= 0x1F;		// 7. Enable digital pins PF4, PF0
}

/** Calls a delay. */
/** Assumes a 80 MHz clock. */
/** TODO: Adjust the delay to last one exact second. */
void delay(unsigned long time){
	unsigned long i;
	
	while(time > 0){
		i = 555555; //6666665;
		while(i>0){
			i = i - 1;
		}
		time = time - 1;

	}
	return;
}

/** Use timmer 0 to control the clock shift*/
/** TODO: Adjust the delay to last one exact second. */
void Timer0IntHandler(void) {
    //
    // Clear the timer interrupt.
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Toggle the flag for the first timer.
    HWREGBITW(&g_ui32Flags, 0) ^= 1;

    //
    // Update the interrupt status on the display.
    ROM_IntMasterDisable();

    // Shift clock time
		shiftMinutes();
		GPIO_PORTF_DATA_R ^= 0x08;
    ROM_IntMasterEnable();
		return;
}

/** Controls the minutes advance. */
void shiftMinutes(){
  if((GPIO_PORTA_DATA_R & 0xF0) == 0xF0){
		GPIO_PORTA_DATA_R &= ~0xFC;
		shiftHours();
		GPIO_PORTA_DATA_R |= portA_advance;
	} else {
		GPIO_PORTA_DATA_R += portA_advance; 
	}
	return;
}
/** Controls the hour advance. */
void shiftHours(){
	if(((GPIO_PORTE_DATA_R & 0x0F) == 0x02) && ((GPIO_PORTF_DATA_R & 0x04) == 0x04)){
		GPIO_PORTE_DATA_R &= ~0x0F;
		GPIO_PORTE_DATA_R |= portE_advance;
		GPIO_PORTF_DATA_R &= ~0x04;
	}
	if((GPIO_PORTE_DATA_R & 0x0F) == 0x09){
		GPIO_PORTE_DATA_R &= ~0x0F;
		GPIO_PORTE_DATA_R |= portE_advance;
		GPIO_PORTF_DATA_R |= 0x04; 
	} else {
		GPIO_PORTE_DATA_R += portE_advance; 
	}
	return;
	
}
