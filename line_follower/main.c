#include <stdint.h>
#include "msp.h"
#include "../inc/BumpInt.h"
#include "../inc/Clock.h"
#include "../inc/SysTickInts.h"
#include "../inc/Reflectance.h"


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



// fsm state for robot (rory)
struct state{
    uint8_t left; // pwm % for left motor
    uint8_t right;// pwm % for right motor
    const struct state*next[8];
};
typedef const struct state state_t;

#define straight &fsm[0]
#define slight_left &fsm[1]
#define slight_right &fsm[2]
#define harder_left &fsm[3]
#define harder_right &fsm[4]
#define all_left &fsm[5]
#define all_right &fsm[6]
#define err_left &fsm[7]
#define err_right &fsm[8]
#define err_straight &fsm[9]

// fsm for our robot (rory)
//state_t fsm[10]={
//    {15,15,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }},// straight
//    {20,10,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_right }},// slight_left
//    {10,20,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }},// slight_right
//    {20,5 ,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_right }},// harder_left
//    {5 ,20,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }},// harder_right
//    {25 ,0,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_right }},// all_left
//    {0, 25,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }}, // all_right
//    {25,0 ,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }}, // err_left
//    {0 ,25,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_right }},// err_right
//    {15,15,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left}} // err_straight
//};

state_t fsm[10]={
    {15,15,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_straight }},// straight
    {15,10,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }},// slight_left
    {10,15,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_right }},// slight_right
    {15,5 ,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }},// harder_left
    {5 ,15,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_right }},// harder_right
    {20, 0,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }},// all_left
    {0 ,20,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_right }}, // all_right
    {20,0 ,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_left }}, // err_left
    {0 ,20,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_right }},// err_right
    {20,5,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, err_straight}} // err_straight
};

// Using SysTick to read reflectance (rory)
uint8_t sensor_value;
uint8_t bump_value;
uint8_t interrupt_count;
int32_t reflectance_value;

void SysTick_Handler(void){
    if(interrupt_count == 0){
        Reflectance_Start();
    }else if(interrupt_count == 1){
        sensor_value = Reflectance_End();
        reflectance_value = Reflectance_Position(sensor_value);
    }
    interrupt_count = (interrupt_count + 1) % 10;
}

state_t* state;
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
    Motor_Forward_2();
    state = straight;
}
int main(void) {
    //Clock_Init12MHz();  // Use 12 MHz instead of 48 MHz for power savings
    Clock_Init48MHz(); 
    ConfigureUnusedPins();  // Configure all unused pins as outputs and set them low
    Motor_Control_Init();  // Initialize motor control GPIO
    Motor_Forward_2();     // Start moving forward
    BumpInt_Init(BumpTask); // (andreea) initialization for bump and edge interrupt
    SysTick_Init(48000, 7);// may need to change priority
    Reflectance_Init();

    // Define PWM period (~3.2 kHz for 48 MHz clock)
    PWM_Init(15000);         // Initialize PWM


    state = straight;
    reflectance_value = 0;
    sensor_value = 0x01; // want sooome values but don't want the bad case
    while (1) {
        if(reflectance_value == 0 && (sensor_value == 0 || sensor_value == 0xFF)){
            state = state->next[7]; // bad!!
            P2->OUT = 0x07; // white
        }else if(reflectance_value < -23799){
            P2->OUT = 0x01;//red
            state = state->next[0]; //way off on left
        }else if(-23799 < reflectance_value && reflectance_value < -20000){
            state = state->next[1];
            P2->OUT = 0x04; //blue
        }else if(-20000 < reflectance_value && reflectance_value < -10000){
            state = state->next[2];
            P2->OUT = 0x02; // green
        }else if(-10000 < reflectance_value && reflectance_value < 10000){
            state = state->next[3];
            P2->OUT = 0x00; // dark
        }else if(10000 < reflectance_value && reflectance_value < 20000){
            state = state->next[4];
            P2->OUT = 0x06; // sky_blue
        }else if(20000 < reflectance_value && reflectance_value < 23799){
            state = state->next[5];
            P2->OUT = 0x05;//pink
        }else if(reflectance_value > 23799){
            state = state->next[6];
            P2->OUT = 0x03;// yellow
        }

        PWM_SetDutyPercentage(state->left, state->right);


    }
}
