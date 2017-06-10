/*******************************************************************************
  Filename:       ll_scheduler.h
  Revised:        $Date: 2011-12-15 13:45:50 -0800 (Thu, 15 Dec 2011) $
  Revision:       $Revision: 28688 $

  Description:    This file contains the types, constants, API's etc. for the
                  Link Layer (LL) task scheduler.

  Copyright 2009-2015 Texas Instruments Incorporated. All rights reserved.

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
*******************************************************************************/

#ifndef LL_SCHEDULER_H
#define LL_SCHEDULER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * INCLUDES
 */

/*******************************************************************************
 * MACROS
 */


/*******************************************************************************
 * CONSTANTS
 */

// Task ID
#define LL_TASK_ID_ADVERTISER                    0x01
#define LL_TASK_ID_SCANNER                       0x02
#define LL_TASK_ID_INITIATOR                     0x04
#define LL_TASK_ID_SLAVE                         0x40
#define LL_TASK_ID_MASTER                        0x80
#define LL_TASK_ID_NONE                          0xFF

// Task ID Masks
#define LL_TASK_ID_SECONDARY_TASKS               (LL_TASK_ID_ADVERTISER      | \
                                                  LL_TASK_ID_SCANNER         | \
                                                  LL_TASK_ID_INITIATOR)

// Task State
#define LL_TASK_STATE_INACTIVE                   0
#define LL_TASK_STATE_ACTIVE                     1
#define LL_TASK_STATE_UNDEFINED                  0xFF

// Save State Data
#define ACCESS_ADDR_LEN                          4
#define CRC_INIT_LEN                             3

// The number of slots (i.e. 625us timer ticks) is the amount of time
// each master has to execute including post-processing and scheduling.
// It determines the relative offset of each additional connection
// interval, and thus limits the number of allowed concurrent connections.
// For example, if the value is four, then three connections can fit
// without affecting each other or the minimum connection interval
// (i.e. 7.5ms). But if the value is 12, then the minimum connection
// interval for three connections is 22.5ms. For five is 37.5ms. Etc.
#define NUM_SLOTS_PER_MASTER                     8

// The number of task blocks needed depends on the build configuration. The
// Non-connectable Adv and Scan would each require a task block if supported.
// If a Slave connection is supported, then a task is needed for the connectable
// Adv. This task is then reused when a Slave connection is formed. If a Master
// connection is supported, then a task is needed for Init. This task is then
// reused when a Master connection is formed. Thus, the total number of needed
// tasks is at most two plus the number of connections.
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_NCONN_CFG)
#define NUM_TASK_BLOCKS_ADV_NCONN_CFG            1
#else
#define NUM_TASK_BLOCKS_ADV_NCONN_CFG            0
#endif // CTRL_CONFIG=ADV_NCONN_CFG

#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
#define NUM_TASK_BLOCKS_SCAN_CFG                 1
#else
#define NUM_TASK_BLOCKS_SCAN_CFG                 0
#endif // CTRL_CONFIG=SCAN_CFG

// collect the total number of non-connection related task blocks based config
// Note: The number of connections is defined in the llConfigTable.
#define LL_NUM_TASK_BLOCKS                    (NUM_TASK_BLOCKS_ADV_NCONN_CFG + \
                                               NUM_TASK_BLOCKS_SCAN_CFG + 1)

/*******************************************************************************
 * TYPEDEFS
 */

// Task Information
typedef struct
{
  uint8       taskID;            // type of LL task
  uint8       taskState;         // whether task is active or inactive
  uint32      command;           // address to radio operation
  uint32      startTime;         // initial RF command start time; used by PM
  uint32      anchorPoint;       // anchor point
  uint32      lastStartTime;     // previous start time
  void        (*setup)(void);    // function used for specific setup operations
} taskInfo_t;

// Task List
typedef struct
{
  uint8        numTasks;         // number of active tasks
  uint8        activeTasks;      // bits to indicate which tasks are active
  uint8        lastSecTask;      // the last secondary task; used for combo states
  taskInfo_t  *curTask;          // currently active task
  taskInfo_t  *llTasks;          // array of tasks
} taskList_t;

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * Functions
 */

extern taskList_t  llTaskList;
//
extern void        llSchedulerInit( void );
extern void        llScheduler( void );
extern void        llScheduleTask( taskInfo_t *llTask );
extern uint8       llFindStartType( taskInfo_t *secTask, taskInfo_t *primTask );
extern taskInfo_t *llFindNextSecTask( uint8 secTaskID );
extern taskInfo_t *llAllocTask( uint8 llTaskID );
extern void        llFreeTask( taskInfo_t **llTask );
extern taskInfo_t *llGetCurrentTask( void );
extern taskInfo_t *llGetTask( uint8 llTaskID );
extern uint8       llGetTaskState( uint8 llTaskID );
extern uint8       llActiveTask( uint8 llTaskID );
extern uint8       llGetActiveTasks( void );
extern uint8       llGetNumTasks( void );

#ifdef __cplusplus
}
#endif

#endif /* LL_SCHEDULER_H */
