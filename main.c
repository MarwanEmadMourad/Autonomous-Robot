#include <stdint.h>
#include "inc/tm4c123gh6pm.h"


// Autonomous robot that avoids obstacles using 2 stepper motors and 2 pump senseors
// PB0-PB3-->Left Stepper motor l293 chip
// PB4-PB7-->Right Stepper motor l293 chip
// PE0,PE1 are connected to the 2 pump sensors (Positive Logic)


void PORTE_PORTB_init(void){

    *((volatile uint32_t*)0x400FE608) |= 0x00000012 ;  // initializing clock to port B & E
    *((volatile uint32_t*)0x40005400) |= 0x000000FF ;  // initializing (PB0-PB7) as o/p
    *((volatile uint32_t*)0x40024400) &= ~0x03   ;     // initializing PE0 and PE1 as i/p
    *((volatile uint32_t*)0x4000551C) |= 0x000000FF ;  // activating digital enable on PB0-PB5
    *((volatile uint32_t*)0x4002451C) |= 0x03 ;        // activating digital enable on PE0,PE1
    GPIO_PORTE_AMSEL_R &= ~0x03;                       // disable analog function on PE1-0
    GPIO_PORTE_PCTL_R &= ~0x000000FF;                  // enable regular GPIO
    GPIO_PORTE_AFSEL_R &= ~0x03;                       // regular function on PE1-0
    GPIO_PORTB_AMSEL_R &= ~0x3F;                       // disable analog function on PB5-0
    GPIO_PORTB_PCTL_R &= ~0x00FFFFFF;                  // enable regular GPIO
    GPIO_PORTB_AFSEL_R &= ~0x3F;                       // regular function on PB5-0
}

void delay(float N){

    // N is the time value in seconds

    uint32_t time = 1600000 ;        //number of bus cycles that is equivalent to 1sec obtained from Systick timer
    uint32_t x = N*time ;            //value of the total delay time multiplied by the bus cycles number
    while (x)                        //delay loop
    {x-- ;}
}

typedef struct State{

    //here we are defining a new data type (Stype) contains the following members

        unsigned long out ;       // State output

        unsigned long wait ;      // time to stay in each state

        unsigned long next[4];    // here an element of next array refers to the input value of the sensors
                                  // 00 --> no obstacle so keep moving forward
                                  // 01 --> obstacle to the right so turn left
                                  // 10 --> obstacle to the left so turn right
                                  // 11 --> obstacle ahead so move backwards


}Stype;

int main(void)
{
    PORTE_PORTB_init();          //Initializing Port B&E

	Stype FSM[10] = {

	//these states represents the output to the steppers in order to move them forward,backward,left and right.

	{0x55, 0.05 , {1,4,7,3}},              //  0x55-->0x66-->0xAA-->0X99-->0x55 :is the complete cycle to  move the steppers forward
    {0x66, 0.05 , {2,2,2,0}},
    {0xAA, 0.05 , {3,3,3,1}},              //  0x55-->0x99-->0xAA-->0x66-->0x55 :is the complete cycle to  move the steppers backwards
    {0x99, 0.05 , {0,0,0,2}},
    {0x69, 0.05 , {5,5,5,5}},              //  0x55-->0x69-->0xAA-->0x96-->0x55 :is the complete cycle to  turn the steppers left
    {0xAA, 0.05 , {6,6,6,6}},
    {0x96, 0.05 , {0,0,0,0}},
    {0x96, 0.05 , {8,8,8,8}},              //  0x55-->0x96-->0xAA-->0x69-->0x55 :is the complete cycle to  turn the steppers right
    {0xAA, 0.05 , {9,9,9,9}},
    {0x69, 0.05 , {0,0,0,0}},

	};

    unsigned long Current_state,input ;

	Current_state = 0 ;                    // assume that the initial state is there is no obstacle ahead

	while (1){

	    GPIO_PORTB_DATA_R |= FSM[Current_state].out;     //this is the o/p that will go to the motors 8 pins in order to make them move as designed

	    delay(FSM[Current_state].wait) ;                 //stay in the state for 50 msecs

	    input = GPIO_PORTE_DATA_R & 0x03 ;               //take the sensor input

	    Current_state = FSM[Current_state].next[input] ; //update your current state based on the taken input
	}
}
