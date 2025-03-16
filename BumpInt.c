// Negative logic bump sensors
// P4.7 Bump5, left side of robot
// P4.6 Bump4
// P4.5 Bump3
// P4.3 Bump2
// P4.2 Bump1
// P4.0 Bump0, right side of robot

#include <stdint.h>
#include "msp.h"

// set void (*task)(uint8)
void (*Bump_task)(uint8_t);

// Initialize Bump sensors
// Make six Port 4 pins inputs
// Activate interface pull-up
// pins 7,6,5,3,2,0
// Interrupt on falling edge (on touch)
void BumpInt_Init(void(*task)(uint8_t)){
    // write this as part of Lab 14
    Bump_task = task;

    P4->SEL0 &= ~0xED;
    P4->SEL1 &= ~0xED;
    P4->DIR &= ~0xED;
    P4->REN |= 0xED;
    P4->OUT |= 0xED;
    P4->IES |= 0xED;
    P4->IFG &= ~0xED;
    P4->IE |= 0xED;

    NVIC->IP[8] = (NVIC->IP[8]&0x00FFFFFF)|0x40000000; // priority 2
    NVIC->ISER[1] = 0x00000040;     // enable interrupt 38 in NVIC
}

// Read current state of 6 switches
// Returns a 6-bit positive logic result (0 to 63)
// bit 5 Bump5
// bit 4 Bump4
// bit 3 Bump3
// bit 2 Bump2
// bit 1 Bump1
// bit 0 Bump0
uint8_t Bump_Read(void){
    // write this as part of Lab 14
    uint8_t bumpRead, returnval;
    bumpRead = ~P4->IN;

    returnval = (bumpRead & 0x80) >> 2  |
            (bumpRead & 0x40) >> 2  |
            (bumpRead & 0x20) >> 2  |
            (bumpRead & 0x08) >> 1  |
            (bumpRead & 0x04) >> 1  |
            (bumpRead & 0x01);

    return returnval;
}

// triggered on touch, falling edge
void PORT4_IRQHandler(void){
    // clear interrupt flags -- may not be necessary?
    P4->IFG &= ~0xED;

    // call task with bump_read argument
    Bump_task(Bump_Read());
}

