/**************************************************************************************************
  Filename:       broadcaster.c
  Revised:        $Date: 2015-05-06 15:14:31 -0700 (Wed, 06 May 2015) $
  Revision:       $Revision: 43701 $

  Description:    GAP Broadcaster Role for RTOS Applications


  Copyright 2011 - 2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <xdc/std.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>

#include <driverlib/ioc.h>

#include "gap.h"
#include "gatt.h"
#include "linkdb.h"
#include "util.h"

#include "gattservapp.h"
#include "broadcaster.h"
#include "gapbondmgr.h"

#include "osal_snv.h"
#include "ICallBleAPIMSG.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
// Profile Events
#define START_ADVERTISING_EVT         0x0001

#define DEFAULT_ADVERT_OFF_TIME       30000   // 30 seconds

// Task configuration
#define GAPROLE_TASK_PRIORITY         3

#ifndef GAPROLE_TASK_STACK_SIZE
#define GAPROLE_TASK_STACK_SIZE       400
#endif

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Entity ID globally used to check for source and/or destination of messages
static ICall_EntityID selfEntity;

// Semaphore globally used to post events to the application thread
static ICall_Semaphore sem;

// Clock object used to signal timeout
static Clock_Struct startAdvClock;

// Task pending events
static uint16_t events = 0;

// Task setup
Task_Struct gapRoleTask;
Char gapRoleTaskStack[GAPROLE_TASK_STACK_SIZE];

static gaprole_States_t gapRole_state;

/*********************************************************************
 * Profile Parameters - reference GAPROLE_PROFILE_PARAMETERS for
 * descriptions
 */

static uint8_t  gapRole_profileRole;
static uint8_t  gapRole_bdAddr[B_ADDR_LEN];
static uint8_t  gapRole_AdvEnabled = TRUE;
static uint16_t gapRole_AdvertOffTime = DEFAULT_ADVERT_OFF_TIME;
static uint8_t  gapRole_AdvertDataLen = 3;
static uint8_t  gapRole_AdvertData[B_MAX_ADV_LEN] =
{
  0x02,             // length of this data
  GAP_ADTYPE_FLAGS, // AD Type = Flags
  // BR/EDR not supported
  GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static uint8_t  gapRole_ScanRspDataLen = 0;
static uint8_t  gapRole_ScanRspData[B_MAX_ADV_LEN] = {0};
static uint8_t  gapRole_AdvEventType;
static uint8_t  gapRole_AdvDirectType;
static uint8_t  gapRole_AdvDirectAddr[B_ADDR_LEN] = {0};
static uint8_t  gapRole_AdvChanMap;
static uint8_t  gapRole_AdvFilterPolicy;

// Application callbacks
static gapRolesCBs_t *pGapRoles_AppCGs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

/*********************************************************************
 * Profile Attributes - Table
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void gapRole_init(void);
static void gapRole_taskFxn(UArg a0, UArg a1);

static void gapRole_processStackMsg(ICall_Hdr *pMsg);
static void gapRole_processGAPMsg(gapEventHdr_t *pMsg);
static void gapRole_SetupGAP(void);

static void gapRole_setEvent(uint32_t event);

/*********************************************************************
 * CALLBACKS
 */
void gapRole_clockHandler(UArg a0);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @brief   Set a GAP Role parameter.
 *
 * Public function defined in broadcaster.h.
 */
bStatus_t GAPRole_SetParameter(uint16_t param, uint8_t len, void *pValue)
{
  bStatus_t ret = SUCCESS;
  switch (param)
  {
    case GAPROLE_ADVERT_ENABLED:
      if (len == sizeof(uint8_t))
      {
        uint8_t oldAdvEnabled = gapRole_AdvEnabled;
        gapRole_AdvEnabled = *((uint8_t*)pValue);

        if ((oldAdvEnabled) && (gapRole_AdvEnabled == FALSE))
        {
          // Turn off Advertising
          if (gapRole_state == GAPROLE_ADVERTISING)
          {
            VOID GAP_EndDiscoverable(selfEntity);
          }
        }
        else if ((oldAdvEnabled == FALSE) && (gapRole_AdvEnabled))
        {
          // Turn on Advertising
          if ((gapRole_state == GAPROLE_STARTED)
              || (gapRole_state == GAPROLE_WAITING))
          {
            gapRole_setEvent(START_ADVERTISING_EVT);
          }
        }
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADVERT_OFF_TIME:
      if (len == sizeof (uint16_t))
      {
        gapRole_AdvertOffTime = *((uint16_t*)pValue);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADVERT_DATA:
      if (len <= B_MAX_ADV_LEN)
      {
        VOID memset(gapRole_AdvertData, 0, B_MAX_ADV_LEN);
        VOID memcpy(gapRole_AdvertData, pValue, len);
        gapRole_AdvertDataLen = len;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_SCAN_RSP_DATA:
      if (len <= B_MAX_ADV_LEN)
      {
        VOID memset(gapRole_ScanRspData, 0, B_MAX_ADV_LEN);
        VOID memcpy(gapRole_ScanRspData, pValue, len);
        gapRole_ScanRspDataLen = len;
        
        // Update the advertising data
        ret = GAP_UpdateAdvertisingData(selfEntity,
                              TRUE, gapRole_AdvertDataLen, gapRole_AdvertData);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADV_EVENT_TYPE:
      if ((len == sizeof (uint8_t)) && 
          (*((uint8_t*)pValue) <= GAP_ADTYPE_ADV_LDC_DIRECT_IND))
      {
        gapRole_AdvEventType = *((uint8_t*)pValue);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADV_DIRECT_TYPE:
      if ((len == sizeof (uint8_t)) && 
          (*((uint8_t*)pValue) <= ADDRTYPE_PRIVATE_RESOLVE))
      {
        gapRole_AdvDirectType = *((uint8_t*)pValue);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADV_DIRECT_ADDR:
      if (len == B_ADDR_LEN)
      {
        VOID memcpy(gapRole_AdvDirectAddr, pValue, B_ADDR_LEN) ;
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADV_CHANNEL_MAP:
      if ((len == sizeof (uint8_t)) && (*((uint8_t*)pValue) <= 0x07))
      {
        gapRole_AdvChanMap = *((uint8_t*)pValue);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case GAPROLE_ADV_FILTER_POLICY:
      if ((len == sizeof (uint8_t)) && 
          (*((uint8_t*)pValue) <= GAP_FILTER_POLICY_WHITE))
      {
        gapRole_AdvFilterPolicy = *((uint8_t*)pValue);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      // The param value isn't part of this profile, try the GAP.
      if ((param < TGAP_PARAMID_MAX) && (len == sizeof (uint16_t)))
      {
        ret = GAP_SetParamValue(param, *((uint16_t*)pValue));
      }
      else
      {
        ret = INVALIDPARAMETER;
      }
      break;
  }

  return (ret);
}

/*********************************************************************
 * @brief   Get a GAP Role parameter.
 *
 * Public function defined in broadcaster.h.
 */
bStatus_t GAPRole_GetParameter(uint16_t param, void *pValue)
{
  bStatus_t ret = SUCCESS;
  switch (param)
  {
    case GAPROLE_PROFILEROLE:
      *((uint8_t*)pValue) = gapRole_profileRole;
      break;

    case GAPROLE_BD_ADDR:
      VOID memcpy(pValue, gapRole_bdAddr, B_ADDR_LEN);
      break;

    case GAPROLE_ADVERT_ENABLED:
      *((uint8_t*)pValue) = gapRole_AdvEnabled;
      break;

    case GAPROLE_ADVERT_OFF_TIME:
      *((uint16_t*)pValue) = gapRole_AdvertOffTime;
      break;

    case GAPROLE_ADVERT_DATA:
      VOID memcpy(pValue , gapRole_AdvertData, gapRole_AdvertDataLen);
      break;

    case GAPROLE_SCAN_RSP_DATA:
      VOID memcpy(pValue, gapRole_ScanRspData, gapRole_ScanRspDataLen) ;
      break;

    case GAPROLE_ADV_EVENT_TYPE:
      *((uint8_t*)pValue) = gapRole_AdvEventType;
      break;

    case GAPROLE_ADV_DIRECT_TYPE:
      *((uint8_t*)pValue) = gapRole_AdvDirectType;
      break;

    case GAPROLE_ADV_DIRECT_ADDR:
      VOID memcpy(pValue, gapRole_AdvDirectAddr, B_ADDR_LEN) ;
      break;

    case GAPROLE_ADV_CHANNEL_MAP:
      *((uint8_t*)pValue) = gapRole_AdvChanMap;
      break;

    case GAPROLE_ADV_FILTER_POLICY:
      *((uint8_t*)pValue) = gapRole_AdvFilterPolicy;
      break;

    default:
      // The param value isn't part of this profile, try the GAP.
      if (param < TGAP_PARAMID_MAX)
      {
        *((uint16_t*)pValue) = GAP_GetParamValue(param);
      }
      else
      {
        ret = INVALIDPARAMETER;
      }
      break;
  }

  return (ret);
}

/*********************************************************************
 * @brief   Does the device initialization.
 *
 * Public function defined in broadcaster.h.
 */
bStatus_t GAPRole_StartDevice(gapRolesCBs_t *pAppCallbacks)
{
  if (gapRole_state == GAPROLE_INIT)
  {
    // Clear all of the Application callbacks
    if (pAppCallbacks)
    {
      pGapRoles_AppCGs = pAppCallbacks;
    }

    // Start the GAP
    gapRole_SetupGAP();

    return (SUCCESS);
  }
  else
  {
    return (bleAlreadyInRequestedMode);
  }
}

/*********************************************************************
 * @fn      GAPRole_createTask
 *
 * @brief   Task creation function for the GAP Broadcaster Role.
 *
 * @param   none
 *
 * @return  none
 */
void GAPRole_createTask(void)
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = gapRoleTaskStack;
  taskParams.stackSize = GAPROLE_TASK_STACK_SIZE;
  taskParams.priority = GAPROLE_TASK_PRIORITY;
  
  Task_construct(&gapRoleTask, gapRole_taskFxn, &taskParams, NULL);
}

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

/*********************************************************************
 * @fn      gapRole_init
 *
 * @brief   Initialization function for the GAP Role Task.
 *
 * @param   none
 *
 * @return  none
 */
static void gapRole_init(void)
{ 
  // Register the current thread as an ICall dispatcher application
  // so that the application can send and receive messages.
  ICall_registerApp(&selfEntity, &sem);

  gapRole_state = GAPROLE_INIT;

  // Setup timers as one-shot timers
  Util_constructClock(&startAdvClock, gapRole_clockHandler,
                      0, 0, false, START_ADVERTISING_EVT);

  // Initialize the Profile Advertising and Connection Parameters
  gapRole_profileRole = GAP_PROFILE_BROADCASTER;

  gapRole_AdvEventType = GAP_ADTYPE_ADV_NONCONN_IND;
  gapRole_AdvDirectType = ADDRTYPE_PUBLIC;
  gapRole_AdvChanMap = GAP_ADVCHAN_ALL;
  gapRole_AdvFilterPolicy = GAP_FILTER_POLICY_ALL;
}

/*********************************************************************
 * @fn      gapRole_taskFxn
 *
 * @brief   Task entry point for the GAP Peripheral Role.
 *
 * @param   a0 - first argument
 * @param   a1 - second argument
 *
 * @return  none
 */
static void gapRole_taskFxn(UArg a0, UArg a1)
{  
  // Initialize profile
  gapRole_init();
  
  // Profile main loop
  for (;;)
  {
    // Waits for a signal to the semaphore associated with the calling thread.
    // Note that the semaphore associated with a thread is signaled when a
    // message is queued to the message receive queue of the thread or when
    // ICall_signal() function is called onto the semaphore.
    ICall_Errno errno = ICall_wait(ICALL_TIMEOUT_FOREVER);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      ICall_EntityID dest;
      ICall_ServiceEnum src;
      ICall_HciExtEvt *pMsg = NULL;
      
      if (ICall_fetchServiceMsg(&src, &dest, 
                                (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
      {
        if ((src == ICALL_SERVICE_CLASS_BLE) && (dest == selfEntity))
        {
          // Process inter-task message
          gapRole_processStackMsg((ICall_Hdr *)pMsg);
        }

        if (pMsg)
        {
          ICall_freeMsg(pMsg);
        }
      }
    }
    
    if (events & START_ADVERTISING_EVT)
    {
      events &= ~START_ADVERTISING_EVT;
      
      if (gapRole_AdvEnabled)
      {
        gapAdvertisingParams_t params;

        // Setup advertisement parameters
        params.eventType = gapRole_AdvEventType;
        params.initiatorAddrType = gapRole_AdvDirectType;
        VOID memcpy(params.initiatorAddr, gapRole_AdvDirectAddr, B_ADDR_LEN);
        params.channelMap = gapRole_AdvChanMap;
        params.filterPolicy = gapRole_AdvFilterPolicy;

        if (GAP_MakeDiscoverable(selfEntity, &params) != SUCCESS)
        {
          gapRole_state = GAPROLE_ERROR;
          
          // Notify the application with the new state change
          if (pGapRoles_AppCGs && pGapRoles_AppCGs->pfnStateChange)
          {
            pGapRoles_AppCGs->pfnStateChange(gapRole_state);
          }
        }
      }
    }
  }
}

/*********************************************************************
 * @fn      gapRole_processStackMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void gapRole_processStackMsg(ICall_Hdr *pMsg)
{
  switch (pMsg->event)
  {
    case GAP_MSG_EVENT:
      gapRole_processGAPMsg((gapEventHdr_t *)pMsg);
      break;

    default:
      break;
  }
}

/*********************************************************************
 * @fn      gapRole_processGAPMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void gapRole_processGAPMsg(gapEventHdr_t *pMsg)
{
  uint8_t notify = FALSE;   // State changed notify the app? (default no)

  switch (pMsg->opcode)
  {
    case GAP_DEVICE_INIT_DONE_EVENT:
      {
        gapDeviceInitDoneEvent_t *pPkt = (gapDeviceInitDoneEvent_t *)pMsg;
        bStatus_t stat = pPkt->hdr.status;

        if (stat == SUCCESS)
        {
          // Save off the information
          VOID memcpy(gapRole_bdAddr, pPkt->devAddr, B_ADDR_LEN);

          gapRole_state = GAPROLE_STARTED;

          // Update the advertising data
          stat = GAP_UpdateAdvertisingData(selfEntity, TRUE,
                                            gapRole_AdvertDataLen,
                                            gapRole_AdvertData);
        }

        if (stat != SUCCESS)
        {
          gapRole_state = GAPROLE_ERROR;
        }

        notify = TRUE;
      }
      break;

    case GAP_ADV_DATA_UPDATE_DONE_EVENT:
      {
        gapAdvDataUpdateEvent_t *pPkt = (gapAdvDataUpdateEvent_t *)pMsg;

        if (pPkt->hdr.status == SUCCESS)
        {
          if (pPkt->adType)
          {
            // Setup the Response Data
            pPkt->hdr.status = GAP_UpdateAdvertisingData(selfEntity,
                              FALSE, gapRole_ScanRspDataLen, gapRole_ScanRspData);
          }
          else if ((gapRole_state != GAPROLE_ADVERTISING) &&
                    (Util_isActive(&startAdvClock) == FALSE))
          {
            // Start advertising
            gapRole_setEvent(START_ADVERTISING_EVT);
          }
        }

        if (pPkt->hdr.status != SUCCESS)
        {
          // Set into Error state
          gapRole_state = GAPROLE_ERROR;
          notify = TRUE;
        }
      }
      break;

    case GAP_MAKE_DISCOVERABLE_DONE_EVENT:
    case GAP_END_DISCOVERABLE_DONE_EVENT:
      {
        gapMakeDiscoverableRspEvent_t *pPkt = (gapMakeDiscoverableRspEvent_t *)pMsg;

        if (pPkt->hdr.status == SUCCESS)
        {
          if (pMsg->opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
          {
            gapRole_state = GAPROLE_ADVERTISING;
          }
          else // GAP_END_DISCOVERABLE_DONE_EVENT
          {

            if (gapRole_AdvertOffTime != 0)
            {
              if ((gapRole_AdvEnabled))
              {
                Util_restartClock(&startAdvClock, gapRole_AdvertOffTime);
              }
            }
            else
            {
              // Since gapRole_AdvertOffTime is set to 0, the device should not
              // automatically become discoverable again after a period of time.
              // Set enabler to FALSE; device will become discoverable again when
              // this value gets set to TRUE
              gapRole_AdvEnabled = FALSE;
            }

            // In the Advertising Off period
            gapRole_state = GAPROLE_WAITING;
          }
        }
        else
        {
          gapRole_state = GAPROLE_ERROR;
        }
        notify = TRUE;
      }
      break;

    default:
      break;
  }

  if (notify == TRUE)
  {
    // Notify the application
    if (pGapRoles_AppCGs && pGapRoles_AppCGs->pfnStateChange)
    {
      pGapRoles_AppCGs->pfnStateChange(gapRole_state);
    }
  }
}

/*********************************************************************
 * @fn      gapRole_SetupGAP
 *
 * @brief   Call the GAP Device Initialization function using the
 *          Profile Parameters.
 *
 * @param   none
 *
 * @return  none
 */
static void gapRole_SetupGAP(void)
{
  VOID GAP_DeviceInit(selfEntity, gapRole_profileRole, 0, NULL, NULL, NULL);
}

/*********************************************************************
 * @fn      gapRole_setEvent
 *
 * @brief   Set an event
 *
 * @param   event - event to be set
 *
 * @return  none
 */
static void gapRole_setEvent(uint32_t event)
{
  events |= event;
      
  // Wake up the application thread when it waits for clock event
  Semaphore_post(sem);
}

/*********************************************************************
 * @fn      gapRole_clockHandler
 *
 * @brief   Clock handler function
 *
 * @param   a0 - event
 *
 * @return  none
 */
void gapRole_clockHandler(UArg a0)
{
  gapRole_setEvent(a0);
}

/*********************************************************************
*********************************************************************/
