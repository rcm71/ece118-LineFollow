

// can prob optimize how much we import
#include "msp.h"
#include "../inc/Clock.h"
#include "../inc/SysTickInts.h"
#include "../inc/Reflectance.h"
#include "../inc/bump.h"



#define TOTAL_STATES 7



struct state{
    uint8_t left;
    uint8_t right;
    const struct state*next[7];
};
typedef const struct state state_t;

#define straight &fsm[0]
#define slight_left &fsm[1]
#define slight_right &fsm[2]
#define harder_left &fsm[3]
#define harder_right &fsm[4]
#define all_left &fsm[5]
#define all_right &fsm[6]


state_t fsm[TOTAL_STATES]={
    {30,30,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, all_right }},// straight
    {20,30,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, slight_right }},// slight_left
    {30,20,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, slight_left }},// slight_right
    {10,30,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, harder_right }},// harder_left
    {30,10,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, harder_left }},// harder_right
    {0 ,30,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, all_right }},// all_left
    {30, 0,{ all_left, harder_left, slight_left, straight, slight_right, harder_right, all_right, all_left }} // all_right
};


void PWM_SetDutyPercentage(uint8_t left, uint8_t right);


// Using SysTick to read reflectance
uint8_t sensor_value;
uint8_t flag;
uint8_t bump_value;
uint8_t interrupt_count;
volatile uint8_t semaphore;
uint32_t reflectance_value;

void SysTick_Handler(void){
    if(interrupt_count == 0){
        Reflectance_Start();
    }else if(interrupt_count == 1){
        flag = 1; // 'semaphore'
        sensor_value = Reflectance_End();
        reflectance_value = Reflectance_Position(sensor_value);
    }
    interrupt_count = (interrupt_count + 1) % 10;
}



void main(void)
{
    // init EVERYTHING
    // wheels, reflectance, bumps
    // turn everything else off
    // clock
    Clock_Init48MHz();
    Motor_Control_Init();
    SysTick_Init(48000, 2);// may need to change priority

    state_t* state = straight;
    reflectance_value = 0;
    sensor_value = 0x01; // want sooome values but don't want the bad case
    while(1){
        PWM_SetDutyPercentage(state->left, state->right);
        if(reflectance_value == 0 && sensor_value == 0){
            state = state->next[6]; // bad!!
        }else if(reflectance_value < -30000){
            state = state->next[0]; //way off on left
        }else if(-30000 < reflectance_value && reflectance_value < -20000){
            state = state->next[1];
        }else if(-20000 < reflectance_value && reflectance_value < -10000){
            state = state->next[2];
        }else if(-10000 < reflectance_value && reflectance_value < 10000){
            state = state->next[3];
        }else if(20000 < reflectance_value && reflectance_value < 30000){
            state = state->next[4];
        }else if(reflectance_value > 300000){
            state = state->next[5];
        }
        // wait??
        // what to do with bumps!?
    }



}
