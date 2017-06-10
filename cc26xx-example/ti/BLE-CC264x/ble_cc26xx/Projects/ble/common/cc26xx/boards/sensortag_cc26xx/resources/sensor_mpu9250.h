/******************************************************************************
*  Filename:       sensor_mpu9250.h
*  Revised:        $Date: 2014-03-07 10:33:11 +0100 (fr, 07 mar 2014) $
*  Revision:       $Revision: 12329 $
*
*  Description:    This file contains the declaration to the HAL Invensense 
*                  MPU9250 abstraction layer.
*
*  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/
#ifndef SENSOR_MPU9250_H
#define SENSOR_MPU9250_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "stdint.h"
#include "stdbool.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */
#define ACC_RANGE_INVALID -1
  
#define ACC_RANGE_2G      0
#define ACC_RANGE_4G      1
#define ACC_RANGE_8G      2
#define ACC_RANGE_16G     3
  
#define MPU_AX_GYR_X      2
#define MPU_AX_GYR_Y      1
#define MPU_AX_GYR_Z      0
#define MPU_AX_GYR        0x07
  
#define MPU_AX_ACC_X      5
#define MPU_AX_ACC_Y      4
#define MPU_AX_ACC_Z      3
#define MPU_AX_ACC        0x38

#define MPU_AX_MAG        6

#define MPU_DATA_READY    0x01
#define MPU_MOVEMENT      0x40

 /* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */
#if CC2650_SENSORTAG >= 0060
void sensorMpu9250PowerOn(void);
void sensorMpu9250PowerOff(void);
bool sensorMpu9250PowerIsOn(void);
#endif

bool sensorMpu9250Init(void);
bool sensorMpu9250Test(void);

void sensorMpu9250Enable(uint16_t config);
bool sensorMpu9250IntEnable(bool enable);

bool sensorMpu9250AccSetRange(uint8_t range);
uint8_t sensorMpu9250AccReadRange(void);
bool sensorMpu9250AccRead(uint16_t *rawData);
float sensorMpu9250AccelConvert(int16_t rawValue);

bool sensorMpu9250GyroRead(uint16_t *rawData);
float sensorMpu9250GyroConvert(int16_t rawValue);
uint8_t sensorMpu9250IntStatus(void);

#ifdef MAG_INCLUDED
bool sensorMpu9250MagTest(void);
bool sensorMpu9250MagRead(uint16_t *rawData);
float sensorMpu9250MagConvert(int16_t rawValue);
#endif

#ifdef WOM_ENABLE
bool sensorMpu9250WomEnable(void);
#endif

/**************************************************************************************************
*/

#ifdef __cplusplus
};
#endif

#endif

/**************************************************************************************************
*/
