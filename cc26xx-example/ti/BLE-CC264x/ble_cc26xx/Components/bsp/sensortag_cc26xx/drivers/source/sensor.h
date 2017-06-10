/**************************************************************************************************
*  Filename:       sensor.h
*  Revised:        $Date: 2015-01-31 05:01:21 -0800 (Sat, 31 Jan 2015) $
*  Revision:       $Revision: 14988 $
*
*  Description:    Sensor driver shared code.
*
*  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
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
**************************************************************************************************/
#ifndef SENSOR_H
#define SENSOR_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "stdbool.h"
#include "stdint.h"

#ifdef TI_DRIVERS_I2C_INCLUDED
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#endif
  
/*********************************************************************
 * CONSTANTS and MACROS
 */

 /* Sensor I2C addresses */
#define BMP280_I2C_ADDRESS                    0x40 // Pressue
#define TMP007_I2C_ADDRESS                    0x44 // IR temperature
#define OPT3001_I2C_ADDRESS                   0x45 // Optical sensor
#define MPU9250_I2C_ADDRESS                   0x68 // Gyro/Accelerometer
#define SHT21_I2C_ADDRESS                     0x77 // Humidity sensor
#define MAG_I2C_ADDRESS                       0x0C // Compass (packaged with MPU9250)
  
/* Bit values for power on self-test */
#define ST_IRTEMP                             0x01
#define ST_HUMIDITY                           0x02
#define ST_LIGHT                              0x04
#define ST_PRESSURE                           0x08
#define ST_MPU                                0x10
  
#ifdef FEATURE_MAGNETOMETER
#define ST_MAG                                0x20
#else
#define ST_MAG                                0x00
#endif
  
#define ST_FLASH                              0x40
#define ST_DEVPACK                            0x80
  
#define ST_TEST_MAP                           ( ST_IRTEMP | ST_HUMIDITY | ST_LIGHT | ST_PRESSURE | ST_MPU | ST_MAG | ST_FLASH)
  
/* Self test assertion; return FALSE (failed) if condition is not met */
#define ST_ASSERT(cond) st( if (!(cond)) {bspI2cDeselect(); return false;} )

/* Data to use when an error occurs */
#define ST_ERROR_DATA                         0xCC
  
/* Loop enclosure for macros */
#define st(x)      do { x } while (__LINE__ == -1)

/* Conversion macros */
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

/* Delay */
#ifdef TI_DRIVERS_I2C_INCLUDED
#define delay_ms(i) Task_sleep( ((i) * 1000) / Clock_tickPeriod )
#define MS_2_TICKS(ms) ( ((ms) * 1000) / Clock_tickPeriod ) 
#else
#define delay_ms(i) ( CPUdelay(8000*(i)) )
#endif

/*********************************************************************
 * FUNCTIONS
 */
uint8_t  sensorTestExecute(void);
uint8_t  sensorTestResult(void);
bool     sensorReadReg(uint8_t addr, uint8_t *pBuf, uint8_t nBytes);
bool     sensorWriteReg(uint8_t addr, uint8_t *pBuf, uint8_t nBytes);
void     sensorSetErrorData(uint8_t *pBuf, uint8_t nBytes);

void     convertToLe(uint8_t *data, uint8_t len);
uint16_t floatToSfloat(float data);
float    sfloatToFloat(uint16_t rawData);
uint16_t intToSfloat(int data);
/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_H */
