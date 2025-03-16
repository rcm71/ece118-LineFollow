#include <stdint.h>
#include "msp.h"
#include "../inc/bump.h"
#include "../inc/BumpInt.h"
#include "../inc/Clock.h"
#include "../inc/CortexM.h"
#include "../inc/FlashProgram.h"
#include "../inc/LaunchPad.h"
#include "../inc/Motor.h"
#include "../inc/MotorSimple.h"
#include "../inc/SysTick.h"
#include "../inc/TExaS.h"
#include "../inc/TimerA1.h"




// Function to initialize TimerA for Center-Aligned PWM
void PWM_Init(uint16_t period) {
    P2->DIR |= BIT6 | BIT7;     // Set P2.6 and P2.7 as outputs (PWM)
    P2->SEL0 |= BIT6 | BIT7;    // Select TimerA function
    P2->SEL1 &= ~(BIT6 | BIT7);

    TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 | // SMCLK (48 MHz system clock)
                    TIMER_A_CTL_MC_3 |     // Up/Down mode (Center-Aligned PWM)
                    TIMER_A_CTL_CLR;       // Clear timer

    TIMER_A0->CCR[0] = period - 1;         // Set PWM period

    TIMER_A0->CCTL[3] = TIMER_A_CCTLN_OUTMOD_6;  // Center-aligned mode for right motor
    TIMER_A0->CCR[3] = 14999;                    // Initialize right motor duty cycle to off

    TIMER_A0->CCTL[4] = TIMER_A_CCTLN_OUTMOD_6;  // Center-aligned mode for left motor
    TIMER_A0->CCR[4] = 14999;                    // Initialize left motor duty cycle to off
}

// Function to convert percentage (0-100%) to CCR value (0-14999)
inline uint16_t PercentageToCCR(uint8_t percentage) {
    if (percentage == 0) {
        return 14999;  // 0% means motor is off
    }
    // Map 1-100% to 0-14999
    return (uint16_t)((100 - percentage) * 14999 / 100);
}

// Function to set PWM duty cycle in percentage (0-100%)
void PWM_SetDutyPercentage(uint8_t dutyL, uint8_t dutyR) {
    TIMER_A0->CCR[3] = PercentageToCCR(dutyR);  // Update right motor duty cycle
    TIMER_A0->CCR[4] = PercentageToCCR(dutyL);  // Update left motor duty cycle
}

// Function to initialize motor direction and enable pins
void Motor_Control_Init(void) {
    P5->DIR |= BIT4 | BIT5;  // Set P5.4 (Left Dir) and P5.5 (Right Dir) as outputs
    P3->DIR |= BIT6 | BIT7;  // Set P3.6 (Right Enable) and P3.7 (Left Enable) as outputs
}

// Function to move motors forward
void Motor_Forward_2(void) {
    P5->OUT &= ~BIT4;  // Left motor forward (LOW)
    P5->OUT &= ~BIT5;  // Right motor forward (LOW)
    P3->OUT |= BIT6;   // Enable right motor
    P3->OUT |= BIT7;   // Enable left motor
}

// (andreea) Function to move motors backward
void Motor_Backward_2(uint32_t time) {
    P5->OUT |= BIT4;  // Left motor backward (HIGH)
    P5->OUT |= BIT5;  // Right motor backward (HIGH)
    P3->OUT |= BIT6;   // Enable right motor
    P3->OUT |= BIT7;   // Enable left motor


    Clock_Delay1ms(time);
}

// Function to stop motors
void Motor_Stop_2(void) {
    P3->OUT &= ~BIT6;  // Disable right motor
    P3->OUT &= ~BIT7;  // Disable left motor
}

// Function to configure all unused GPIO pins as outputs and set them low
void ConfigureUnusedPins(void) {
    // Configure all unused pins as outputs and set them low
    P1->DIR = 0xFF; P1->OUT = 0x00;
    P2->DIR = 0xFF; P2->OUT = 0x00;
    P3->DIR = 0xFF; P3->OUT = 0x00;
    P4->DIR = 0xFF; P4->OUT = 0x00;
    P5->DIR = 0xFF; P5->OUT = 0x00;
    P6->DIR = 0xFF; P6->OUT = 0x00;
    P7->DIR = 0xFF; P7->OUT = 0x00;
    P8->DIR = 0xFF; P8->OUT = 0x00;
    P9->DIR = 0xFF; P9->OUT = 0x00;
    P10->DIR = 0xFF; P10->OUT = 0x00;
}

// (andreea) Function to define task called when bump sensor is triggered
void BumpTask(uint8_t bump) { // bump is a 6 bit number
    // stop motors immediately
    Motor_Stop_2();

    if ((bump & 0x01) | (bump & 0x02) | (bump & 0x03)) { // bump 0, 1, or both pressed

        // move motor backwards for 1 second
        Motor_Backward_2(1000);
        PWM_SetDutyPercentage(20, 10);  // Turn right
        Clock_Delay1ms(2000);

    }
    else if ((bump & 0x04) | (bump & 0x08) | (bump & 0x0C)) { // bump 2, 3, or both pressed
        // move motor backwards for 2 seconds
        Motor_Backward_2(2000);
        PWM_SetDutyPercentage(20, 10);  // Turn right
        Clock_Delay1ms(2000);
    }
    else if ((bump & 0x10) | (bump & 0x20) | (bump & 0x30)) { // bump 4, 5, or both pressed
        // move motor backwards for 1 second and turn left
        Motor_Backward_2(1000);
        PWM_SetDutyPercentage(10, 20);  // Turn left
        Clock_Delay1ms(2000);
    }
    else { // anything else, wild card
        Motor_Backward_2(1000);
        PWM_SetDutyPercentage(10, 20);  // Turn left
        Clock_Delay1ms(2000);
    }
}

int main(void) {
    Clock_Init12MHz();  // Use 12 MHz instead of 48 MHz for power savings
    ConfigureUnusedPins();  // Configure all unused pins as outputs and set them low
    Motor_Control_Init();  // Initialize motor control GPIO
    Motor_Forward_2();     // Start moving forward
    BumpInt_Init(BumpTask); // (andreea) initialization for bump and edge interrupt

    uint16_t period = 15000;  // Define PWM period (~3.2 kHz for 48 MHz clock)
    PWM_Init(period);         // Initialize PWM

    while (1) {
        PWM_SetDutyPercentage(10, 10);  // Move straight
        Clock_Delay1ms(2000);

       PWM_SetDutyPercentage(20, 10);  // Turn right
       Clock_Delay1ms(2000);

       PWM_SetDutyPercentage(10, 20);  // Turn left
       Clock_Delay1ms(2000);

       PWM_SetDutyPercentage(0, 0);    // Stop motors
       Clock_Delay1ms(2000);
    }
}
