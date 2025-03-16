// Lab14_EdgeInterruptsmain.c
// Runs on MSP432, interrupt version
// Main test program for interrupt driven bump switches the robot.
// Daniel Valvano and Jonathan Valvano
// July 11, 2019

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2019
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2019, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// Negative logic bump sensors
// P4.7 Bump5, left side of robot
// P4.6 Bump4
// P4.5 Bump3
// P4.3 Bump2
// P4.2 Bump1
// P4.0 Bump0, right side of robot

#include <stdint.h>
#include "msp.h"
#include "../inc/Clock.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"
#include "../inc/Motor.h"
#include "../inc/BumpInt.h"
#include "../inc/TExaS.h"
#include "../inc/TimerA1.h"
#include "../inc/FlashProgram.h"

void Port2_Init(void){
  P2->DIR |= 0x07;
  P2->SEL0 &= ~0x07;
  P2->SEL1 &= ~0x07;
  P2->OUT &= ~0x07;
}

// (andreea) Function to define task called when bump sensor is triggered
void BumpTask(uint8_t bump) { // bump is a 6 bit number
    // stop motors immediately

    if (bump & 0x01) { // bump 0 = red
        P2->OUT = (P2->OUT & ~0x07) | 0x01;
    }
    else if (bump & 0x02) { // bump 1 = blue
        P2->OUT = (P2->OUT & ~0x07) | 0x04;
    }
    else if (bump & 0x04) { // bump 2 = green
        P2->OUT = (P2->OUT & ~0x07) | 0x02;
    }
    else if (bump & 0x08) { // bump 3 = yellow
        P2->OUT = (P2->OUT & ~0x07) | 0x03;
    }
    else if (bump & 0x010) { // bump 4 = sky blue
        P2->OUT = (P2->OUT & ~0x07) | 0x06;
    }
    else if (bump & 0x20) { // bump 5 = white
        P2->OUT = (P2->OUT & ~0x07) | 0x07;
    }
    else { // black
        P2->OUT = (P2->OUT & ~0x07) | 0x00;
    }
}

int main(void) {
    Clock_Init48MHz();
    BumpInt_Init(BumpTask); // (andreea) initialization for bump and edge interrupt
    Port2_Init();

    while (1) {

         uint32_t i, j, temp, length;
         uint32_t a[100]={5000,5308,5614,5918,6219,6514,
         6804,7086,7361,7626,7880,8123,8354,8572,8776,8964,9137,
         9294,9434,9556,9660,9746,9813,9861,9890,9900,9890,9861,
         9813,9746,9660,9556,9434,9294,9137,8964,8776,8572,8354,
         8123,7880,7626, 7361,7086,6804,6514,6219,5918,5614,
         5308,5000,4692,4386,4082,3781,3486,3196,2914,2639,2374,
         2120,1877,1646,1428,1224,1036,863,706,566,444,340,254,
         187,139,110,100,110,139,187,254,340,444,566,706,863,
         1036, 1224, 1428, 1646,1877,2120,2374,2639,2914,
         3196,3486,3781,4082,4386,4692};
         length = 100;
         for (i = 0; i < length; i++)
         {
             for (j = 0; j < length - i - 1; j++)
             {
                 if (a[j + 1] < a[j])
                 {
                 temp = a[j];
                 a[j] = a[j + 1];
                 a[j + 1] = temp;
                 }
             }
         }

    }
}

