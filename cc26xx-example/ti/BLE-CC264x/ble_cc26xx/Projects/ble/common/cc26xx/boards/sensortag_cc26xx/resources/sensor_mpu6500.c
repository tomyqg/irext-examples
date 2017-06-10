/**************************************************************************************************
*  Filename:       sensor_mpu6500.c
*  Revised:        $Date: 2014-02-05 10:47:02 +0100 (on, 05 feb 2014) $
*  Revision:       $Revision: 12066 $
*
*  Description:    Driver for the Invensys MPU6500 Motion Processing Unit
*
*  Copyright (C) 2014 - 2015 Texas Instruments Incorporated - http://www.ti.com/
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
*************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
*                                          Includes
* ------------------------------------------------------------------------------------------------
*/

#include "sensor_mpu6500.h"
#include "sensor.h"
#include "bsp_i2c.h"

/* ------------------------------------------------------------------------------------------------
*                                           Constants
* ------------------------------------------------------------------------------------------------
*/
// Sensor I2C address
#define SENSOR_I2C_ADDRESS            0x68

// Registers
#define SELF_TEST_X_GYRO              0x00 // R/W
#define SELF_TEST_Y_GYRO              0x01 // R/W
#define SELF_TEST_Z_GYRO              0x02 // R/W
#define SELF_TEST_X_ACCEL             0x0D // R/W
#define SELF_TEST_Z_ACCEL             0x0E // R/W
#define SELF_TEST_Y_ACCEL             0x0F // R/W

#define XG_OFFSET_H                   0x13 // R/W
#define XG_OFFSET_L                   0x14 // R/W
#define YG_OFFSET_H                   0x15 // R/W
#define YG_OFFSET_L                   0x16 // R/W
#define ZG_OFFSET_H                   0x17 // R/W
#define ZG_OFFSET_L                   0x18 // R/W

#define SMPLRT_DIV                    0x19 // R/W
#define CONFIG                        0x1A // R/W
#define GYRO_CONFIG                   0x1B // R/W
#define ACCEL_CONFIG                  0x1C // R/W
#define ACCEL_CONFIG_2                0x1D // R/W
#define LP_ACCEL_ODR                  0x1E // R/W
#define WOM_THR                       0x1F // R/W
#define FIFO_EN                       0x23 // R/W

// .. registers 0x24 - 0x36 are not applicable to the SensorTag HW configuration (IC2 Master)

#define INT_PIN_CFG                   0x37 // R/W
#define INT_ENABLE                    0x38 // R/W
#define INT_STATUS                    0x3A // R
#define ACCEL_XOUT_H                  0x3B // R
#define ACCEL_XOUT_L                  0x3C // R
#define ACCEL_YOUT_H                  0x3D // R
#define ACCEL_YOUT_L                  0x3E // R
#define ACCEL_ZOUT_H                  0x3F // R
#define ACCEL_ZOUT_L                  0x40 // R
#define TEMP_OUT_H                    0x41 // R
#define TEMP_OUT_L                    0x42 // R
#define GYRO_XOUT_H                   0x43 // R
#define GYRO_XOUT_L                   0x44 // R
#define GYRO_YOUT_H                   0x45 // R
#define GYRO_YOUT_L                   0x46 // R
#define GYRO_ZOUT_H                   0x47 // R
#define GYRO_ZOUT_L                   0x48 // R

// .. registers 0x49 - 0x60 are not applicable to the SensorTag HW configuration (external sensor data)
// .. registers 0x63 - 0x67 are not applicable to the SensorTag HW configuration (I2C master)

#define SIGNAL_PATH_RESET             0x68 // R/W
#define ACCEL_INTEL_CTRL              0x69 // R/W
#define USER_CTRL                     0x6A // R/W
#define PWR_MGMT_1                    0x6B // R/W
#define PWR_MGMT_2                    0x6C // R/W
#define FIFO_COUNT_H                  0x72 // R/W
#define FIFO_COUNT_L                  0x73 // R/W
#define FIFO_R_W                      0x74 // R/W
#define WHO_AM_I                      0x75 // R/W           

// Acceleromter ranges
#define ACC_REG_CTRL_2G               0x00
#define ACC_REG_CTRL_4G               0x08
#define ACC_REG_CTRL_8G               0x10
#define ACC_REG_CTRL_16G              0x18
#define ACC_REG_CTRL_INVALID          0xFF

// Masks is mpuConfig valiable
#define ACC_CONFIG_MASK               0x38
#define GYRO_CONFIG_MASK              0x07

// Values PWR_MGMT_1
#define MPU_SLEEP                     0x4F  // Sleep + stop all clocks
#define MPU_WAKE_UP                   0x09  // Disable temp. + intern osc

// Values PWR_MGMT_2                  
#define ALL_AXES                      0x3F
#define GYRO_AXES                     0x07
#define ACC_AXES                      0x38

// Data sizes
#define DATA_SIZE                     6

// Output data rates
#define INV_LPA_0_3125HZ              0
#define INV_LPA_0_625HZ               1
#define INV_LPA_1_25HZ                2
#define INV_LPA_2_5HZ                 3
#define INV_LPA_5HZ                   4
#define INV_LPA_10HZ                  5
#define INV_LPA_20HZ                  6
#define INV_LPA_40HZ                  7
#define INV_LPA_80HZ                  8
#define INV_LPA_160HZ                 9
#define INV_LPA_320HZ                 10
#define INV_LPA_640HZ                 11
#define INV_LPA_STOPPED               255

// Bit values
#define BIT_ACTL                      0x80
#define BIT_LATCH_EN                  0x20
#define BIT_ANY_RD_CLR                0x10
#define BIT_RAW_RDY_EN                0x01
#define BIT_WOM_EN                    0x40
#define BIT_LPA_CYCLE                 0x20
#define BIT_STBY_XA                   0x20
#define BIT_STBY_YA                   0x10
#define BIT_STBY_ZA                   0x08
#define BIT_STBY_XG                   0x04
#define BIT_STBY_YG                   0x02
#define BIT_STBY_ZG                   0x01
#define BIT_STBY_XYZA                 (BIT_STBY_XA | BIT_STBY_YA | BIT_STBY_ZA)
#define BIT_STBY_XYZG                 (BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)

// Sensor selection/deselection
#define SENSOR_SELECT()                     bspI2cSelect(BSP_I2C_INTERFACE_1,SENSOR_I2C_ADDRESS)
#define SENSOR_DESELECT()                   bspI2cDeselect()

/* ------------------------------------------------------------------------------------------------
*                                           Typedefs
* ------------------------------------------------------------------------------------------------
*/

/* ------------------------------------------------------------------------------------------------
*                                           Macros
* ------------------------------------------------------------------------------------------------
*/

/* ------------------------------------------------------------------------------------------------
*                                           Local Functions
* ------------------------------------------------------------------------------------------------
*/
static void sensorMpuSleep(void);
static void sensorMpu6500WakeUp(void);
static void sensorMpu6500SelectAxes(void);

/* ------------------------------------------------------------------------------------------------
*                                           Local Variables
* ------------------------------------------------------------------------------------------------
*/
static uint8_t mpuConfig;
static uint8_t accRange = ACC_RANGE_INVALID;
static uint8_t accRangeReg;
static uint8_t val;
static uint8_t intStatus;
static bool isWomEnabled;

/**************************************************************************************************
* @fn          sensorMpu6500Init
*
* @brief       This function initializes the HAL Accelerometer abstraction layer.
*
* @return      True if success
*/
bool sensorMpu6500Init(void)
{
  bool ret;
  
  isWomEnabled = false;
  
  // Select this sensor
  SENSOR_SELECT();

  // Device reset
  val = 0x80;
  ret = sensorWriteReg(PWR_MGMT_1, &val, 1);
  if (ret)
  {
    delay_ms(100);
    
    // Release reset
    val = 0x00;
    ret = sensorWriteReg(PWR_MGMT_1, &val, 1);
    
    if (ret)
    {      
      delay_ms(100);
      
      // Initial configuration
      mpuConfig = 0;   // All axes off
      sensorMpu6500AccSetRange(ACC_RANGE_8G);
      
      // Power save
      sensorMpuSleep();
    }
  }
  
  SENSOR_DESELECT();
  
  return ret;
}


/**************************************************************************************************
* @fn          sensorMpu6500IntEnable
*
* @brief       This function initializes the Accelerometer abstraction layer.
*
* @param       enable - true if interrupt is to be generated
*
* @return      True if success
*/
bool sensorMpu6500IntEnable(bool enable)
{
  bool ret;
  
  SENSOR_SELECT();
  
  // Configure INT pin
  val = enable ? BIT_RAW_RDY_EN : 0;
  ret = sensorWriteReg(INT_ENABLE, &val, 1);

  SENSOR_DESELECT();
  
  return ret;
}

/**************************************************************************************************
* @fn          sensorMpu6500WomEnable
*
* @brief       Enable Wake On Motion functionality
*
* @return      True if success
*/
bool sensorMpu6500WomEnable(void)
{
  SENSOR_SELECT();
  
  // Make sure accelerometer is running
  val = 0x09;
  ST_ASSERT(sensorWriteReg(PWR_MGMT_1, &val, 1));

  // Enable accelerometer, disable gyro
  val = 0x07;
  ST_ASSERT(sensorWriteReg(PWR_MGMT_2, &val, 1));
  
  // Set Accel LPF setting to 184 Hz Bandwidth
  val = 0x01;
  ST_ASSERT(sensorWriteReg(ACCEL_CONFIG_2, &val, 1));
  
  // Enable Motion Interrupt
  val = BIT_WOM_EN;
  ST_ASSERT(sensorWriteReg(INT_ENABLE, &val, 1));
    
  // Enable Accel Hardware Intelligence
  val = 0xC0;
  ST_ASSERT(sensorWriteReg(ACCEL_INTEL_CTRL, &val, 1));
  
  // Set Motion Threshold
  val = 3;
  ST_ASSERT(sensorWriteReg(WOM_THR, &val, 1));
  
  // Set Frequency of Wake-up
  val = INV_LPA_5HZ;
  ST_ASSERT(sensorWriteReg(LP_ACCEL_ODR, &val, 1));
  
  // Enable Cycle Mode (Accel Low Power Mode)
  val = 0x29;
  ST_ASSERT(sensorWriteReg(PWR_MGMT_1, &val, 1));
  
  // Select the current range
  ST_ASSERT(sensorWriteReg(ACCEL_CONFIG, &accRangeReg, 1));

  SENSOR_DESELECT();
  isWomEnabled = true;
  mpuConfig = BIT_STBY_XYZA;
  
  return true;
}


/**************************************************************************************************
* @fn          sensorMpu6500IntStatus
*
* @brief       Check whether a data or wake on motion interrupt has occurred
*
* @return      Return interrupt status
*/
uint8_t sensorMpu6500IntStatus(void)
{
  SENSOR_SELECT();
  sensorReadReg(INT_STATUS,&intStatus,1);
  SENSOR_DESELECT();
  
  return intStatus;
}

/**************************************************************************************************
* @fn          sensorMpu6500AccEnable
*
* @brief       Enable acceleromter readout
*
* @param       Axes: bitmap [0..2], X = 1, Y = 2, Z = 4. 0 = accelerometer off
*
* @return      None
*/
void sensorMpu6500AccEnable(uint8_t axes)
{
  // Select this sensor
  SENSOR_SELECT();

  if (mpuConfig == 0 && axes != 0)
  {
    // Wake up the sensor if it was off
    sensorMpu6500WakeUp();
  }

  mpuConfig &= ~ACC_CONFIG_MASK;
  mpuConfig |= (axes << 3);
  
  if (mpuConfig != 0)
  {
    // Enable accelerometer readout
    sensorMpu6500SelectAxes();
    delay_ms(10);
  }
  else if (mpuConfig == 0)
  {
    sensorMpuSleep();
  }
  
  SENSOR_DESELECT();
}

/**************************************************************************************************
* @fn          sensorMpu6500GyroEnable
*
* @brief       Enable gyroscope readout
*
* @param       Axes: bitmap [0..2], X = 1, Y = 2, Z = 4. 0 = gyroscope off
*
* @return      None
*/
void sensorMpu6500GyroEnable(uint8_t axes)
{
  // Select this sensor
  SENSOR_SELECT();

  if (mpuConfig == 0 && axes != 0 && !isWomEnabled)
  {
    // Wake up the sensor if it was off
    sensorMpu6500WakeUp();
  }
  
  mpuConfig &= ~GYRO_CONFIG_MASK;
  mpuConfig |= axes;
  
  if (mpuConfig != 0)
  {
    // Enable gyro readout
    sensorMpu6500SelectAxes();
    // Gyro seems to cause false interrupts during startup, so delay processing slightly
    delay_ms(10);
  }
  else if (mpuConfig == 0 && !isWomEnabled)
  {
    sensorMpuSleep();
  }
  
  SENSOR_DESELECT();
}

/**************************************************************************************************
* @fn          sensorMpu6500AccSetRange
*
* @brief       Set the range of the accelerometer
*
* @param       newRange: ACC_RANGE_2G, ACC_RANGE_4G, ACC_RANGE_8G, ACC_RANGE_16G
*
* @return      true if write succeeded
*/
bool sensorMpu6500AccSetRange(uint8_t newRange)
{
  bool success;
  
  if (newRange == accRange)
    return true;
  
  success = false;
  
  switch (newRange)
  {
  case ACC_RANGE_2G:
    accRangeReg = ACC_REG_CTRL_2G;
    break;
  case ACC_RANGE_4G:
    accRangeReg = ACC_REG_CTRL_4G;
    break;
  case ACC_RANGE_8G:
    accRangeReg = ACC_REG_CTRL_8G;
    break;
  case ACC_RANGE_16G:
    accRangeReg = ACC_REG_CTRL_16G;
    break;
  default:
    accRangeReg = ACC_REG_CTRL_INVALID;
    // Should not get here
    break;
  }
  
  if (accRangeReg != ACC_REG_CTRL_INVALID)
  {  
    // Apply the range
    success = sensorWriteReg(ACCEL_CONFIG, &accRangeReg, 1);
    if (success)
      accRange = newRange;
  }
  
  return success;
}

/**************************************************************************************************
* @fn          sensorMpu6500AccRead
*
* @brief       Read data from the accelerometer - X, Y, Z - 3 words
*
* @return      TRUE if valid data, FALSE if not
*/
bool sensorMpu6500AccRead(uint16_t *data )
{
  bool success;

  // Select this sensor
  SENSOR_SELECT();

  if (intStatus & BIT_RAW_RDY_EN)
  {
    // Burst read of all accelerometer values
    success = sensorReadReg(ACCEL_XOUT_H, (uint8_t*)data, DATA_SIZE);
    if (!success)
    {
      sensorSetErrorData((uint8_t*)data,DATA_SIZE);
    }
  }
  else
  {
    success = false; // Data not ready
  }
  
  SENSOR_DESELECT();

  return success;
}

/**************************************************************************************************
* @fn          sensorMpu6500GyroRead
*
* @brief       Read data from the gyroscope - X, Y, Z - 3 words
*
* @return      TRUE if valid data, FALSE if not
*/
bool sensorMpu6500GyroRead(uint16_t *data )
{
  bool success;
  
  // Select this sensor
  SENSOR_SELECT();
  
  if (intStatus & BIT_RAW_RDY_EN)
  {
    // Burst read of all gyroscope values
    success = sensorReadReg(GYRO_XOUT_H, (uint8_t*)data, DATA_SIZE);
    
    if (!success)
    {
      sensorSetErrorData((uint8_t*)data,DATA_SIZE);
    }
  }
  else
  {
    success = false;
  }

  SENSOR_DESELECT();
  
  return success;
}

/**************************************************************************************************
 * @fn          sensorMpu6500Test
 *
 * @brief       Run a sensor self-test
 *
 * @return      TRUE if passed, FALSE if failed
 */
bool sensorMpu6500Test(void)
{
  // Select this sensor on the I2C bus
  SENSOR_SELECT();
  
  // Check the WHO AM I register
  ST_ASSERT(sensorReadReg(WHO_AM_I, &val, 1));
  ST_ASSERT(val == 0x70);

  SENSOR_DESELECT();
  
  return true;
}

/**************************************************************************************************
 * @fn          sensorMpu6500AccelConvert
 *
 * @brief       Convert raw data to G units
 *
 * @param       rawData - raw data from sensor
 *
 * @return      Converted value
 **************************************************************************************************/
float sensorMpu6500AccelConvert(int16_t rawData)
{
  float v;
  
  switch (accRange)
  {
  case ACC_RANGE_2G:
    //-- calculate acceleration, unit G, range -2, +2
    v = (rawData * 1.0) / (32768/2);
    break;
    
  case ACC_RANGE_4G:
    //-- calculate acceleration, unit G, range -4, +4
    v = (rawData * 1.0) / (32768/4);
    break;
    
  case ACC_RANGE_8G:
    //-- calculate acceleration, unit G, range -8, +8
    v = (rawData * 1.0) / (32768/8);
    break;    
    
  case ACC_RANGE_16G:
    //-- calculate acceleration, unit G, range -16, +16
    v = (rawData * 1.0) / (32768/16);
    break;    
  }
  
  return v;
}

/**************************************************************************************************
 * @fn          sensorMpu6500GyroConvert
 *
 * @brief       Convert raw data to deg/sec units
 *
 * @param       data - raw data from sensor
 *
 * @return      none
 **************************************************************************************************/
float sensorMpu6500GyroConvert(int16_t data)
{           
  //-- calculate rotation, unit deg/s, range -250, +250
  return (data * 1.0) / (65536 / 500);
}

/* ------------------------------------------------------------------------------------------------
*                                           Private functions
* -------------------------------------------------------------------------------------------------
*/

/**************************************************************************************************
* @fn          sensorMpuSleep
*
* @brief       Place the MPU in low power mode
*
* @return
*/
static void sensorMpuSleep(void)
{
  val = ALL_AXES;
  sensorWriteReg(PWR_MGMT_2, &val, 1);

  val = MPU_SLEEP;
  sensorWriteReg(PWR_MGMT_1, &val, 1);
}


/**************************************************************************************************
* @fn          sensorMpu6500WakeUp
*
* @brief       Exit low power mode
*
* @return      none
*/
static void sensorMpu6500WakeUp(void)
{
  val = MPU_WAKE_UP;
  sensorWriteReg(PWR_MGMT_1, &val, 1);
  
  // All axis initially disabled
  val = ALL_AXES;
  sensorWriteReg(PWR_MGMT_2, &val, 1);
  mpuConfig = 0;
  
  // Restore the range
  sensorWriteReg(ACCEL_CONFIG, &accRangeReg, 1);

  // Clear interrupts
  sensorReadReg(INT_STATUS,&val,1);
}


/**************************************************************************************************
* @fn          sensorMpu6500SelectAxes
*
* @brief       MPU in sleep
*
* @return      none
*/
static void sensorMpu6500SelectAxes(void)
{
  val = ~mpuConfig;
  sensorWriteReg(PWR_MGMT_2, &val, 1);
}

// Temporary test function to read all registers
uint8_t sensorMpu6500ReadRegs(uint8_t *data, uint8_t start, uint8_t nRegs)
{
  uint8_t reg;
  SENSOR_SELECT();

  for (reg = start; reg < nRegs; reg++)
  {
    if (sensorReadReg(reg, data+reg, 1) == FALSE)
      break;
  }
  SENSOR_DESELECT();
  
  return reg;
}

/*********************************************************************
*********************************************************************/
