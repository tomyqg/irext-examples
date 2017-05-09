/******************************************************************************
*  Filename:       sensor_tmp007.c
*  Revised:        $Date: 2014-03-07 10:33:11 +0100 (fr, 07 mar 2014) $
*  Revision:       $Revision: 12329 $
*
*  Description:    Driver for the TI TMP06 infrared thermophile sensor.
*
*  Copyright (C) 2014 - 2015 Texas Instruments Incorporated - http://www.ti.com/
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

/* -----------------------------------------------------------------------------
*                                          Includes
* ------------------------------------------------------------------------------
*/
#include "bsp_i2c.h"
#include "sensor.h"
#include "sensor_tmp007.h"
#include "math.h"
/* -----------------------------------------------------------------------------
*                                           Constants
* ------------------------------------------------------------------------------
*/

/* Slave address */
#define SENSOR_I2C_ADDRESS              0x44

/* TMP006 register addresses */
#define TMP007_REG_ADDR_VOLTAGE         0x00
#define TMP007_REG_ADDR_LOCAL_TEMP      0x01
#define TMP007_REG_ADDR_CONFIG          0x02
#define TMP007_REG_ADDR_OBJ_TEMP        0x03
#define TMP007_REG_ADDR_STATUS          0x04
#define TMP007_REG_PROD_ID              0x1F

/* TMP006 register values */
#define TMP007_VAL_CONFIG_ON            0x1000  // Sensor on state
#define TMP007_VAL_CONFIG_OFF           0x0000  // Sensor off state
#define TMP007_VAL_CONFIG_RESET         0x8000
#define TMP007_VAL_PROD_ID              0x0078  // Product ID

/* Bit values */
#define CONV_RDY_BIT                    0x4000  // Conversion ready 

/* Register length */
#define REGISTER_LENGTH                 2

/* Sensor data size */
#define DATA_SIZE                       4

/* Byte swap of 16-bit register value */
#define SWAP(v) ( (LO_UINT16(v) << 8) | HI_UINT16(v) )

// Sensor selection/deselection
#define SENSOR_SELECT()     bspI2cSelect(BSP_I2C_INTERFACE_0,SENSOR_I2C_ADDRESS)
#define SENSOR_DESELECT()   bspI2cDeselect()

/* -----------------------------------------------------------------------------
*                                           Local Functions
* ------------------------------------------------------------------------------
*/

/* -----------------------------------------------------------------------------
*                                           Local Variables
* ------------------------------------------------------------------------------
*/
static uint8_t buf[DATA_SIZE];
static uint16_t val;
/* -----------------------------------------------------------------------------
*                                           Public functions
* ------------------------------------------------------------------------------
*/


/*******************************************************************************
 * @fn          sensorTmp007Init
 *
 * @brief       Initialize the temperature sensor driver
 *
 * @return      none
 ******************************************************************************/
bool sensorTmp007Init(void)
{
  // Configure sensor 
  return sensorTmp007Enable(false);
}


/*******************************************************************************
 * @fn          sensorTmp007Enable
 *
 * @brief       Turn the sensor on/off
 *
 * @return      none
 ******************************************************************************/
bool sensorTmp007Enable(bool enable)
{
  bool success;
  
  SENSOR_SELECT();

  if (enable)
    val = TMP007_VAL_CONFIG_ON;
  else
    val = TMP007_VAL_CONFIG_OFF;
  
  val = SWAP(val);
  success = sensorWriteReg(TMP007_REG_ADDR_CONFIG, (uint8_t*)&val, 
                           REGISTER_LENGTH);
  
  SENSOR_DESELECT();
  
  return success;
}


/*******************************************************************************
 * @fn          sensorTmp007Read
 *
 * @brief       Read the sensor voltage and sensor temperature registers
 *
 * @param       rawtTemp - temperature in 16 bit format
 *
 * @param       rawtObjTemp - object temperature in 16 bit format
 *
 * @return      TRUE if valid data
 ******************************************************************************/
bool sensorTmp007Read(uint16_t *rawTemp, uint16_t *rawObjTemp)
{
  bool success;
  
  SENSOR_SELECT();
  
  success = sensorReadReg(TMP007_REG_ADDR_STATUS, (uint8_t *)&val,
                          REGISTER_LENGTH);
  
  if (success)
  {
    val = SWAP(val);
    success = val & CONV_RDY_BIT;
  }
  
  if (success)
  {
    // Read the sensor registers
    success = sensorReadReg(TMP007_REG_ADDR_LOCAL_TEMP, &buf[0], 
                            REGISTER_LENGTH);
    if (success)
    {
      success = sensorReadReg(TMP007_REG_ADDR_OBJ_TEMP, &buf[2], 
                              REGISTER_LENGTH);
    }
  }
  
  if (!success)
  {
    sensorSetErrorData(buf,4);
  }
  
  // Swap bytes
  *rawTemp = buf[0]<<8 | buf[1];
  *rawObjTemp = buf[2]<<8 | buf[3];
  
  SENSOR_DESELECT();

  return success;
}


/*******************************************************************************
 * @fn          sensorTmp007Test
 *
 * @brief       Run a sensor self-test
 *
 * @return      TRUE if passed, FALSE if failed
 ******************************************************************************/
bool sensorTmp007Test(void)
{
  // Select this sensor on the I2C bus
  SENSOR_SELECT();

  // Check product ID
  ST_ASSERT(sensorReadReg(TMP007_REG_PROD_ID, (uint8_t *)&val, REGISTER_LENGTH));
  val = SWAP(val);
  ST_ASSERT(val == TMP007_VAL_PROD_ID);

  // Turn sensor on
  val = SWAP(TMP007_VAL_CONFIG_ON);
  ST_ASSERT(sensorWriteReg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val,
                           REGISTER_LENGTH));

  // Check config register (on)
  ST_ASSERT(sensorReadReg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val, 
                          REGISTER_LENGTH));
  val = SWAP(val);
  ST_ASSERT(val == TMP007_VAL_CONFIG_ON);

  // Turn sensor off
  val = SWAP(TMP007_VAL_CONFIG_OFF);
  ST_ASSERT(sensorWriteReg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val, 
                           REGISTER_LENGTH));

  // Check config register (off)
  ST_ASSERT(sensorReadReg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val, 
                          REGISTER_LENGTH));
  val = SWAP(val);
  ST_ASSERT(val == TMP007_VAL_CONFIG_OFF);

  SENSOR_DESELECT();

  return true;
}

/*******************************************************************************
 * @fn          sensorTmp007Convert
 *
 * @brief       Convert raw data to object and ambience temperature
 *
 * @param       rawTemp - raw temperature from sensor
 *
 * @param       rawObjTemp - raw temperature from sensor
 *
 * @param       tObj - converted object temperature
 *
 * @param       tAmb - converted ambience temperature
 *
 * @return      none
 ******************************************************************************/
void sensorTmp007Convert(uint16_t rawTemp, uint16_t rawObjTemp, float *tObj, 
                         float *tTgt)
{
  const float SCALE_LSB = 0.03125;
  float t;
  int it;
  
  it = (int)((rawObjTemp) >> 2);
  t = ((float)(it)) * SCALE_LSB;
  *tObj = t;
  
  it = (int)((rawTemp) >> 2);
  t = (float)it;
  *tTgt = t * SCALE_LSB;
}

/*******************************************************************************
*******************************************************************************/

