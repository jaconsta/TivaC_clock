// Clock, main.c

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/pin_map.h"
#include "inc/tm4c123gh6pm.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_types.h"

// Prototype function
void portA_init(void);
void portE_init(void);
void portF_init(void);
void shiftMinutes(void);
void shiftHours(void);
void flashSOS(void);
void delay(unsigned long sec);
void enable_interrupts(void);

// Global variables
unsigned int portA_advance = 0x04;
unsigned int portE_advance = 0x01;

int main(void){
	//unsigned int unit_seconds = 0;
	portA_init();
	portE_init();
	portF_init();
	GPIO_PORTA_DATA_R |= portA_advance;
	delay(1);
	GPIO_PORTE_DATA_R |= portE_advance;
	while (1){
		delay(1);

		shiftMinutes();
		GPIO_PORTF_DATA_R |= 0x08;
		delay(1);

		GPIO_PORTF_DATA_R &= ~0x08;
	}

}

//Initialize port A
void portA_init(void) {
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000001;		// 1. Enable Port Clock
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTA_CR_R |= 0xFC;				// Allow changes to P7A-2
	GPIO_PORTA_AMSEL_R &= ~0xFC;			// 3. Disable analog function
	GPIO_PORTA_PCTL_R &= ~0xFFFFFF00;// 4. GPIO clear bit PCTL
	GPIO_PORTA_DIR_R |= 0xFC;				// 5.2 P7A-2 as output
	GPIO_PORTA_AFSEL_R &= ~0xFC;			// 6 Disable alternate function
	GPIO_PORTA_DEN_R |= 0xFC;				// 7. Enable digital pins P7A-2
}
//Initialize port E
void portE_init(void){
	volatile unsigned long delay;
	
	SYSCTL_RCGC2_R |= 0x10; // 1. Enable port clock
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTE_CR_R |= 0x0F;				// Allow changes to PF3-0
	GPIO_PORTE_AMSEL_R &= 0x00;			// 3. Disable analog function
	GPIO_PORTE_PCTL_R &= 0x00000000;// 4. GPIO clear bit PCTL
	GPIO_PORTE_DIR_R |= 0x0F;				// 5.2 PE3-0 as output
	GPIO_PORTE_AFSEL_R &= 0x00;			// 6 Disable alternate function
	//GPIO_PORTE_PUR_R &= 0x0F;				// Enable pull-up resistors on PE3-0
	GPIO_PORTE_DEN_R |= 0x0F;				// 7. Enable digital pins PE3-0
}
//Initialize port F
void portF_init(void) {
	volatile unsigned long delay;
	// For this microcontroller is a 7 step process to set each port
	SYSCTL_RCGC2_R |= 0x00000020;		// 1. Enable Port Clock
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTF_LOCK_R = 0x4C4F434B;	// 2. Unlock port F PF0
	GPIO_PORTF_CR_R |= 0x1F;				// Allow changes to PF4-0
	GPIO_PORTF_AMSEL_R &= 0x00;			// 3. Disable analog function
	GPIO_PORTF_PCTL_R &= 0x00000000;// 4. GPIO clear bit PCTL
	GPIO_PORTF_DIR_R	&= ~0x11;			// 5.1 PF4, PF0 as inputs
	GPIO_PORTF_DIR_R |= 0x0E;				// 5.2 PF3 as output
	GPIO_PORTF_AFSEL_R &= 0x00;			// 6 Disable alternate function
	GPIO_PORTF_PUR_R &= 0x11;				// Enable pull-up resistors on PF4, PF0
	GPIO_PORTF_DEN_R |= 0x1F;				// 7. Enable digital pins PF4, PF0
}

// Calls a delay
// Assumes a 80 MHz clock
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

void shiftMinutes(){
  if((GPIO_PORTA_DATA_R & 0xF0) == 0xF0){
		GPIO_PORTA_DATA_R &= ~0xFC;
		shiftHours();
		GPIO_PORTA_DATA_R |= portA_advance;
	} else {
		GPIO_PORTA_DATA_R += portA_advance; //<<= portA_advance;
	}
	return;
}
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
