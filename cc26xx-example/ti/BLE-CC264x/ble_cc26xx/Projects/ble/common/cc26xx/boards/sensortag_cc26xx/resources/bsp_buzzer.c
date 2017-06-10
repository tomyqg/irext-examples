/*
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <driverlib/ioc.h>
#include <driverlib/timer.h>
#include "bsp.h"
#include "bsp_buzzer.h"


void bspBuzzerInit(void)
{
  // Configure pin as PWM output
  IOCPortConfigureSet(BSP_IOID_BUZZER, IOC_PORT_MCU_TIMER0, IOC_STD_OUTPUT);
  
  // Use GPT0, Channel A (16 bit timer)
  TimerConfigure(GPT0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM);
}


void bspBuzzerEnable(int freq)
{
  // Stop timer
  TimerDisable(GPT0_BASE, TIMER_A);
  
  if (freq > 0)
  {
    uint32_t load;
    
    load = (48000000 / freq );
    
    // Timer A; freq 48 MHz / freq
    TimerLoadSet(GPT0_BASE, TIMER_A, load);  
    TimerMatchSet(GPT0_BASE, TIMER_A, load / 2 );    // 50 per cent duty cycle
    
    // Start timer
    TimerEnable(GPT0_BASE, TIMER_A);
  }
}

