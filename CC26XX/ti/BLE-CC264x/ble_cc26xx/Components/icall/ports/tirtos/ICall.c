/**
  @file  ICall.c
  @brief Indirect function Call dispatcher implementation on top of TI-RTOS.

  This implementation uses heapmgr.h to implement a simple heap with low
  memory overhead but large processing overhead.<br>
  The size of the heap is determined with HEAPMGR_SIZE macro, which can
  be overridden with a compile option.

  <!--
  Copyright 2013 - 2015 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED ``AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
  -->
*/
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/gates/GateHwi.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>

#include <stdint.h>

#include "ICall.h"
#include "ICallPlatform.h"

#ifndef ICALL_FEATURE_SEPARATE_IMGINFO
#include <ICallAddrs.h>
#endif /* ICALL_FEATURE_SEPARATE_IMGINFO */

#ifndef ICALL_MAX_NUM_ENTITIES
/**
 * Maximum number of entities that use ICall, including service entities
 * and application entities.
 * The value may be overridden by a compile option.
 * Note that there are at least,
 * Primitive service, Stack services along with potentially generic
 * framework service for the stack thread.
 */
#define ICALL_MAX_NUM_ENTITIES     5
#endif

#ifndef ICALL_MAX_NUM_TASKS
/**
 * Maximum number of threads which include entities.
 * The value may be overridden by a compile option.
 */
#define ICALL_MAX_NUM_TASKS        2
#endif

/**
 * @internal
 * Service class value used to indicate an invalid (unused) entry
 */
#define ICALL_SERVICE_CLASS_INVALID_ENTRY  0x0000

/**
 * @internal
 * Service class value used to indicate an entry for an application entity
 */
#define ICALL_SERVICE_CLASS_APPLICATION    ICALL_SERVICE_CLASS_MASK

/**
 * @internal
 * Primitive service entity ID
 */
#define ICALL_PRIMITIVE_ENTITY_ID          0

/**
 * @internal
 * Accessor macro to get a header field (next) from a message pointer
 */
#define ICALL_MSG_NEXT(_p) (((ICall_MsgHdr *)(_p) - 1)->next)

/**
 * @internal
 * Accessor macro to get a header field (dest_id) from a message pointer
 */
#define ICALL_MSG_DEST_ID(_p) (((ICall_MsgHdr *)(_p) - 1)->dest_id)

/**
 * @internal
 * Data structure used to access critical section
 * state variable.
 * Without this data structure, C code will violate
 * C89 or C99 strict aliasing rule.
 */
typedef union _icall_cs_state_union_t
{
  /** critical section variable as declared in the interface */
  ICall_CSState  state;
  /** @internal field used to access internal data */
  struct _icall_cs_state_aggr_t
  {
    /* Note that even if TIRTOS declares each key to be
     * UInt which will, in case of 64 bit compiler, be greater
     * than 16 bits, but it was confirmed from owner
     * that only 16 bits will be used.
     */
    /** field to store Task_disable() return value */
    uint_least16_t taskkey;
    /** field to store Hwi_disable() return value */
    uint_least16_t hwikey;
  } each;
} ICall_CSStateUnion;

/**
 * @internal Primitive service handler function type
 */
typedef ICall_Errno (*ICall_PrimSvcFunc)(ICall_FuncArgsHdr *);

#ifdef ICALL_FEATURE_SEPARATE_IMGINFO
/* Image information shall be in separate module */

/**
 * Array of entry function addresses of external images.
 *
 * Note that function address must be odd number for Thumb mode functions.
 */
extern const ICall_RemoteTaskEntry ICall_imgEntries[];
/**
 * Array of task priorities of external images.
 * One task is created per image to start off the image entry function.
 * Each element of this array correspond to the task priority of
 * each entry function defined in @ref ICall_imgEntries.
 */
extern const Int ICall_imgTaskPriorities[];

/**
 * Array of task stack sizes of external images.
 * One task is created per image to start off the image entry function.
 * Each element of this array correspond to the task stack size of
 * each entry function defined in @ref ICall_imgEntries.
 */
extern const SizeT ICall_imgTaskStackSizes[];

/**
 * Array of custom initialization parameters (pointers).
 * Each initialization parameter (pointer) is passed to each corresponding
 * image entry function defined in @ref ICall_imgEntries;
 */
extern const void *ICall_imgInitParams[];

/**
 * Number of external images.
 */
extern const uint_least8_t ICall_numImages;

#define icall_threadEntries ICall_imgEntries
#define ICall_threadPriorities ICall_imgTaskPriorities
#define ICall_threadStackSizes ICall_imgTaskStackSizes
#define ICall_getInitParams(_i) (ICall_imgInitParams[i])
#define ICALL_REMOTE_THREAD_COUNT ICall_numImages
#else /* ICALL_FEATURE_SEPARATE_IMGINFO */
/**
 * @internal
 * Array of entry function of external images.
 */
static const ICall_RemoteTaskEntry icall_threadEntries[] = ICALL_ADDR_MAPS;

/** @internal external image count */
#define ICALL_REMOTE_THREAD_COUNT \
  (sizeof(icall_threadEntries)/sizeof(icall_threadEntries[0]))

/** @internal thread priorities to be assigned to each remote thread */
static const Int ICall_threadPriorities[] = ICALL_TASK_PRIORITIES;

/** @internal thread stack max depth for each remote thread */
static const SizeT ICall_threadStackSizes[] = ICALL_TASK_STACK_SIZES;

/** @internal initialization parameter (pointer) for each remote thread */
#ifdef ICALL_CUSTOM_INIT_PARAMS
static const void *ICall_initParams[] = ICALL_CUSTOM_INIT_PARAMS;
#define ICall_getInitParams(_i) (ICall_initParams[i])
#else /* ICALL_CUSTOM_INIT_PARAMS */
#define ICall_getInitParams(_i) NULL
#endif /* ICALL_CUSTOM_INIT_PARAMS */

#endif /* ICALL_FEATURE_SEPARATE_IMGINFO */

/** @internal message queue */
typedef void *ICall_MsgQueue;

/** @internal data structure about a task using ICall module */
typedef struct _icall_task_entry_t
{
  Task_Handle task;
  Semaphore_Handle sem;
  ICall_MsgQueue queue;
} ICall_TaskEntry;

/** @internal data structure about an entity using ICall module */
typedef struct _icall_entity_entry_t
{
  ICall_ServiceEnum service;
  ICall_TaskEntry *task;
  ICall_ServiceFunc fn;
} ICall_entityEntry;

/** @internal storage to track all tasks using ICall module */
static ICall_TaskEntry ICall_tasks[ICALL_MAX_NUM_TASKS];

/** @internal storage to track all entities using ICall module */
static ICall_entityEntry ICall_entities[ICALL_MAX_NUM_ENTITIES];

/**
 * @internal
 * Wakeup schedule data structure definition
 */
typedef struct _icall_schedule_t
{
  Clock_Handle  clock;
  ICall_TimerCback cback;
  void *arg;
} ICall_ScheduleEntry;

/* For now critical sections completely disable hardware interrupts
 * because they are used from ISRs in MAC layer implementation.
 * If MAC layer implementation changes, critical section
 * implementation may change to reduce overall interrupt latency.
 */
/* Enter critical section implementation. See header file for comment. */
ICall_CSState ICall_enterCSImpl(void)
{
  ICall_CSStateUnion cu;
  cu.each.taskkey = (uint_least16_t) Task_disable();
  cu.each.hwikey = (uint_least16_t) Hwi_disable();
  return cu.state;
}

/* See header file for comment */
ICall_EnterCS ICall_enterCriticalSection = ICall_enterCSImpl;

/* leave critical section implementation. See header file for comment */
void ICall_leaveCSImpl(ICall_CSState key)
{
  ICall_CSStateUnion *cu = (ICall_CSStateUnion *) &key;
  Hwi_restore((UInt) cu->each.hwikey);
  Task_restore((UInt) cu->each.taskkey);
}

/* See header file for comment */
ICall_LeaveCS ICall_leaveCriticalSection = ICall_leaveCSImpl;

/* Implementing a simple heap using heapmgr.h template.
 * This simple heap depends on critical section implementation
 * and hence the template is used after critical section definition. */
void *ICall_heapMalloc(uint16_t size);
void *ICall_heapRealloc(void *blk, uint16_t size);
void ICall_heapFree(void *blk);
#define HEAPMGR_INIT       ICall_heapInit
#define HEAPMGR_MALLOC     ICall_heapMalloc
#define HEAPMGR_FREE       ICall_heapFree
#define HEAPMGR_REALLOC    ICall_heapRealloc
#define HEAPMGR_GETMETRICS ICall_heapGetMetrics
#define HEAPMGR_LOCK()                                       \
  do { ICall_heapCSState = ICall_enterCSImpl(); } while (0)
#define HEAPMGR_UNLOCK()                                     \
  do { ICall_leaveCSImpl(ICall_heapCSState); } while (0)
#define HEAPMGR_IMPL_INIT()
/* Note that a static variable can be used to contain critical section
 * state since heapmgr.h template ensures that there is no nested
 * lock call. */
static ICall_CSState ICall_heapCSState;
#include <heapmgr.h>

/**
 * @internal Searches for a task entry within @ref ICall_tasks.
 * @param taskhandle  TI-RTOS task handle
 * @return Pointer to task entry when found, or NULL.
 */
static ICall_TaskEntry *ICall_searchTask(Task_Handle taskhandle)
{
  size_t i;
  ICall_CSState key;

  key = ICall_enterCSImpl();
  for (i = 0; i < ICALL_MAX_NUM_TASKS; i++)
  {
    if (!ICall_tasks[i].task)
    {
      /* Empty slot */
      break;
    }
    if (taskhandle == ICall_tasks[i].task)
    {
      ICall_leaveCSImpl(key);
      return &ICall_tasks[i];
    }
  }
  ICall_leaveCSImpl(key);
  return NULL;
}

/**
 * @internal Searches for a task entry within @ref ICall_tasks or
 *           build an entry if the entry table is empty.
 * @param  taskhandle  TI-RTOS task handle
 * @return Pointer to task entry when found, or NULL.
 */
static ICall_TaskEntry *ICall_newTask(Task_Handle taskhandle)
{
  size_t i;
  ICall_CSState key;

  key = ICall_enterCSImpl();
  for (i = 0; i < ICALL_MAX_NUM_TASKS; i++)
  {
    if (!ICall_tasks[i].task)
    {
      /* Empty slot */
      ICall_TaskEntry *taskentry = &ICall_tasks[i];
      taskentry->task = taskhandle;
      taskentry->queue = NULL;
      taskentry->sem = Semaphore_create(0, NULL, NULL);
      if (taskentry->sem == NULL)
      {
        /* abort */
        ICALL_HOOK_ABORT_FUNC();
      }
      ICall_leaveCSImpl(key);
      return taskentry;
    }
    if (taskhandle == ICall_tasks[i].task)
    {
      ICall_leaveCSImpl(key);
      return &ICall_tasks[i];
    }
  }
  ICall_leaveCSImpl(key);
  return NULL;
}

/**
 * @internal Searches for a service entity entry.
 * @param service  service id
 * @return entity id of the service or
 *         @ref ICALL_INVALID_ENTITY_ID when none found.
 */
static ICall_EntityID
ICall_searchServiceEntity(ICall_ServiceEnum service)
{
  size_t i;
  ICall_CSState key;

  key = ICall_enterCSImpl();
  for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
  {
    if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
    {
      /* Empty slot */
      break;
    }
    if (service == ICall_entities[i].service)
    {
      ICall_leaveCSImpl(key);
      return (ICall_EntityID) i;
    }
  }
  ICall_leaveCSImpl(key);
  return ICALL_INVALID_ENTITY_ID;
}

/**
 * @internal Searches for a service entity entry.
 * @param service  service id
 * @return Pointer to entity entry of the service or
 *         NULL when none found.
 */
static ICall_entityEntry *
ICall_searchService(ICall_ServiceEnum service)
{
  ICall_EntityID entity = ICall_searchServiceEntity(service);
  if (entity == ICALL_INVALID_ENTITY_ID)
  {
    return NULL;
  }
  return &ICall_entities[entity];
}

/* Dispatcher implementation. See ICall_dispatcher declaration
 * for comment. */
static ICall_Errno ICall_dispatch(ICall_FuncArgsHdr *args)
{
  ICall_entityEntry *entity;

  entity =  ICall_searchService(args->service);
  if (!entity)
  {
    return ICALL_ERRNO_INVALID_SERVICE;
  }
  if (!entity->fn)
  {
    return ICALL_ERRNO_INVALID_FUNCTION;
  }

  return entity->fn(args);
}

/* See header file for comments */
ICall_Dispatcher ICall_dispatcher = ICall_dispatch;

/* Static instance of ICall_RemoteTaskArg to pass to
 * remote task entry function.
 * See header file for comments */
static const ICall_RemoteTaskArg ICall_taskEntryFuncs =
{
  ICall_dispatch,
  ICall_enterCSImpl,
  ICall_leaveCSImpl
};

/**
 * @internal Thread entry function wrapper that complies with
 *           TI-RTOS.
 * @param arg0  actual entry function
 * @param arg1  ignored
 */
static Void ICall_taskEntry(UArg arg0, UArg arg1)
{
  ICall_RemoteTaskEntry entryfn = (ICall_RemoteTaskEntry) arg0;

  entryfn(&ICall_taskEntryFuncs, (void *) arg1);
}

/* forward reference */
static void ICall_initPrim(void);

/* See header file for comments. */
void ICall_init(void)
{
  size_t i;

  for (i = 0; i < ICALL_MAX_NUM_TASKS; i++)
  {
    ICall_tasks[i].task = NULL;
    ICall_tasks[i].queue = NULL;
  }
  for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
  {
    ICall_entities[i].service = ICALL_SERVICE_CLASS_INVALID_ENTRY;
  }

  /* Initialize primitive service */
  ICall_initPrim();
}

/* See header file for comments */
void ICall_createRemoteTasks(void)
{
  size_t i;
  UInt keytask;

  /* Cheap locking mechanism to lock tasks
   * which may attempt to access the service call dispatcher
   * till all services are registered.
   */
  keytask = Task_disable();

  for (i = 0; i < ICALL_REMOTE_THREAD_COUNT; i++)
  {
    Task_Params params;
    Task_Handle task;
    ICall_CSState key;

    Task_Params_init(&params);
    params.priority = ICall_threadPriorities[i];
    params.stackSize = ICall_threadStackSizes[i];
    params.arg0 = (UArg) icall_threadEntries[i];
    params.arg1 = (UArg) ICall_getInitParams(i);

    task = Task_create(ICall_taskEntry, &params, NULL);
    if (task == NULL)
    {
      /* abort */
      ICALL_HOOK_ABORT_FUNC();
    }
    key = ICall_enterCSImpl();
    if (ICall_newTask(task) == NULL)
    {
      /* abort */
      ICALL_HOOK_ABORT_FUNC();
    }
    ICall_leaveCSImpl(key);
  }

  Task_restore(keytask);
}

/* Primitive service implementation follows */

/**
 * @internal Enrolls a service
 * @param args arguments
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when service id is already
 *         registered by another entity.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when maximum number of services
 *         are already registered.
 */
static ICall_Errno ICall_primEnroll(ICall_EnrollServiceArgs *args)
{
  size_t i;
  ICall_TaskEntry *taskentry = ICall_newTask(Task_self());
  ICall_CSState key;

  /* Note that certain service does not handle a message
   * and hence, taskentry might be NULL.
   */

  key = ICall_enterCSImpl();
  for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
  {
    if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
    {
      /* Use this entry */
      ICall_entities[i].service = args->service;
      ICall_entities[i].task = taskentry;
      ICall_entities[i].fn = args->fn;
      args->entity = (ICall_EntityID) i;
      args->msgsem = taskentry->sem;
      ICall_leaveCSImpl(key);
      return ICALL_ERRNO_SUCCESS;
    }
    else if (args->service == ICall_entities[i].service)
    {
      /* Duplicate service enrollment */
      ICall_leaveCSImpl(key);
      return ICALL_ERRNO_INVALID_PARAMETER;
    }
  }
  /* abort */
  ICALL_HOOK_ABORT_FUNC();
  ICall_leaveCSImpl(key);
  return ICALL_ERRNO_NO_RESOURCE;
}

/**
 * @internal Registers an application
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 */
static ICall_Errno ICall_primRegisterApp(ICall_RegisterAppArgs *args)
{
  size_t i;
  ICall_TaskEntry *taskentry = ICall_newTask(Task_self());
  ICall_CSState key;

  if (!taskentry)
  {
    /* abort */
    ICALL_HOOK_ABORT_FUNC();
    return ICALL_ERRNO_NO_RESOURCE;
  }

  key = ICall_enterCSImpl();
  for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
  {
    if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
    {
      /* Use this entry */
      ICall_entities[i].service = ICALL_SERVICE_CLASS_APPLICATION;
      ICall_entities[i].task = taskentry;
      ICall_entities[i].fn = NULL;
      args->entity = (ICall_EntityID) i;
      args->msgsem = taskentry->sem;
      ICall_leaveCSImpl(key);
      return ICALL_ERRNO_SUCCESS;
    }
  }
  /* abort */
  ICALL_HOOK_ABORT_FUNC();
  ICall_leaveCSImpl(key);
  return ICALL_ERRNO_NO_RESOURCE;
}

/**
 * @internal Allocates memory block for a message.
 * @param args   arguments
 */
static ICall_Errno ICall_primAllocMsg(ICall_AllocArgs *args)
{
  ICall_MsgHdr *hdr =
      (ICall_MsgHdr *) ICall_heapMalloc(sizeof(ICall_MsgHdr) + args->size);

  if (!hdr)
  {
    return ICALL_ERRNO_NO_RESOURCE;
  }
  hdr->len = args->size;
  hdr->next = NULL;
  hdr->dest_id = ICALL_UNDEF_DEST_ID;
  args->ptr = (void *) (hdr + 1);
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Frees the memory block allocated for a message.
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS
 */
static ICall_Errno ICall_primFreeMsg(ICall_FreeArgs *args)
{
  ICall_MsgHdr *hdr = (ICall_MsgHdr *) args->ptr - 1;
  ICall_heapFree(hdr);
  return ICALL_ERRNO_SUCCESS;
}

/**
 * Allocates a memory block.
 * Note that this function is for use by ICall implementation.
 *
 * @param size   size in bytes
 * @return pointer to the allocated memory block or NULL
 */
void *ICall_mallocImpl(uint_fast16_t size)
{
  return ICall_heapMalloc(size);
}

/**
 * Frees a memory block.
 * Note that this function is for use by ICall implementation.
 *
 * @param ptr   pointer to the memory block
 */
void ICall_freeImpl(void *ptr)
{
  ICall_heapFree(ptr);
}

/**
 * @internal Allocates a memory block
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when memory block cannot
 *         be allocated.
 */
static ICall_Errno ICall_primMalloc(ICall_AllocArgs *args)
{
  args->ptr = ICall_heapMalloc(args->size);
  if (args->ptr == NULL)
  {
    return ICALL_ERRNO_NO_RESOURCE;
  }
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Frees a memory block
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS
 */
static ICall_Errno ICall_primFree(ICall_FreeArgs *args)
{
  ICall_heapFree(args->ptr);
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Queues a message to a message queue.
 * @param q_ptr    message queue
 * @param msg_ptr  message pointer
 */
static void ICall_msgEnqueue( ICall_MsgQueue *q_ptr, void *msg_ptr )
{
  void *list;
  ICall_CSState key;

  // Hold off interrupts
  key = ICall_enterCSImpl();

  ICALL_MSG_NEXT( msg_ptr ) = NULL;
  // If first message in queue
  if ( *q_ptr == NULL )
  {
    *q_ptr = msg_ptr;
  }
  else
  {
    // Find end of queue
    for ( list = *q_ptr; ICALL_MSG_NEXT( list ) != NULL;
          list = ICALL_MSG_NEXT( list ) );

    // Add message to end of queue
    ICALL_MSG_NEXT( list ) = msg_ptr;
  }

  // Re-enable interrupts
  ICall_leaveCSImpl(key);
}

/**
 * @internal Dequeues a message from a message queue
 * @param q_ptr  message queue pointer
 * @return Dequeued message pointer or NULL if none.
 */
static void *ICall_msgDequeue( ICall_MsgQueue *q_ptr )
{
  void *msg_ptr = NULL;
  ICall_CSState key;

  // Hold off interrupts
  key = ICall_enterCSImpl();

  if ( *q_ptr != NULL )
  {
    // Dequeue message
    msg_ptr = *q_ptr;
    *q_ptr = ICALL_MSG_NEXT( msg_ptr );
    ICALL_MSG_NEXT( msg_ptr ) = NULL;
    ICALL_MSG_DEST_ID( msg_ptr ) = ICALL_UNDEF_DEST_ID;
  }

  // Re-enable interrupts
  ICall_leaveCSImpl(key);

  return msg_ptr;
}

/**
 * @internal Prepends a list of messages to a message queue
 * @param q_ptr  message queue pointer
 * @param head   message list to prepend
 */
static void ICall_msgPrepend( ICall_MsgQueue *q_ptr, ICall_MsgQueue head )
{
  void *msg_ptr = NULL;
  ICall_CSState key;

  // Hold off interrupts
  key = ICall_enterCSImpl();

  if ( head != NULL )
  {
    /* Find the end of the queue */
    msg_ptr = head;
    while (ICALL_MSG_NEXT( msg_ptr ) != NULL)
    {
      msg_ptr = ICALL_MSG_NEXT( msg_ptr );
    }
    ICALL_MSG_NEXT(msg_ptr) = *q_ptr;
    *q_ptr = head;
  }

  // Re-enable interrupts
  ICall_leaveCSImpl(key);
}

/**
 * @internal Sends a message to an entity.
 * @param args    arguments
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when either src
 *              or dest is not a valid entity id or when
 *              dest is an entity id of an entity that does
 *              not receive a message
 *              (e.g., ICall primitive service entity).
 */
static ICall_Errno ICall_primSend(ICall_SendArgs *args)
{
  ICall_CSState key;
  ICall_MsgHdr *hdr = (ICall_MsgHdr *) args->msg - 1;

  if (args->dest.entityId >= ICALL_MAX_NUM_ENTITIES ||
      args->src >= ICALL_MAX_NUM_ENTITIES)
  {
    return ICALL_ERRNO_INVALID_PARAMETER;
  }
  key = ICall_enterCSImpl();
  if (!ICall_entities[args->dest.entityId].task)
  {
    ICall_leaveCSImpl(key);
    return ICALL_ERRNO_INVALID_PARAMETER;
  }
  ICall_leaveCSImpl(key);
  /* Note that once the entry is valid,
   * the value does not change and hence it is OK
   * to leave the critical section.
   */

  hdr->srcentity = args->src;
  hdr->dstentity = args->dest.entityId;
  hdr->format = args->format;
  ICall_msgEnqueue(&ICall_entities[args->dest.entityId].task->queue, args->msg);
  Semaphore_post(ICall_entities[args->dest.entityId].task->sem);
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Retrieves a message, queued to receive queue of the calling thread.
 *
 * @param args  arguments
 * @return @ref ICALL_ERRNO_SUCCESS when a message was successfully
 *         retrieved.<br>
 *         @ref ICALL_ERRNO_NOMSG when no message was queued to
 *         the receive queue at the moment.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when the calling thread
 *         does not have a received queue associated with it.
 *         This happens when neither ICall_enrollService() nor
 *         ICall_registerApp() was ever called from the calling
 *         thread.
 */
static ICall_Errno ICall_primFetchMsg(ICall_FetchMsgArgs *args)
{
  Task_Handle taskhandle = Task_self();
  ICall_TaskEntry *taskentry = ICall_searchTask(taskhandle);
  ICall_MsgHdr *hdr;

  if (!taskentry)
  {
    return ICALL_ERRNO_UNKNOWN_THREAD;
  }
  /* Successful */
  args->msg = ICall_msgDequeue(&taskentry->queue);
  hdr = (ICall_MsgHdr *) args->msg - 1;
  if (args->msg == NULL)
  {
    return ICALL_ERRNO_NOMSG;
  }
  args->src.entityId = hdr->srcentity;
  args->dest = hdr->dstentity;
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Transforms and entityId into a serviceId.
 * @param entityId   entity id
 * @param servId     pointer to a variable to store
 *                   the resultant service id
 * @return @ICALL_ERRNO_SUCCESS if the transformation was successful.
 *         @ICALL_ERRNO_INVALID_SERVICE if no matching service
 *         is found for the entity id.
 */
static ICall_Errno ICall_primEntityId2ServiceId(ICall_EntityID entityId,
                                                ICall_ServiceEnum *servId)
{
  if (entityId >= ICALL_MAX_NUM_ENTITIES ||
      ICall_entities[entityId].service ==
          ICALL_SERVICE_CLASS_INVALID_ENTRY ||
      ICall_entities[entityId].service ==
          ICALL_SERVICE_CLASS_APPLICATION)
  {
    return ICALL_ERRNO_INVALID_SERVICE;
  }
  *servId = ICall_entities[entityId].service;
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Transforms and entityId into a serviceId.
 * @param args  arguments
 * @return return values corresponding to those of ICall_entityId2ServiceId()
 */
static ICall_Errno ICall_primE2S(ICall_EntityId2ServiceIdArgs *args)
{
  return ICall_primEntityId2ServiceId(args->entityId, &args->servId);
}

/**
 * @internal Sends a message to a registered server.
 * @param args   arguments corresponding to those of ICall_sendServiceMsg().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_INVALID_SERVICE when the 'dest'
 *         is unregistered service.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when the 'src'
 *         is not a valid entity id or when 'dest' is
 *         is a service that does not receive a message
 *         (such as ICall primitive service).
 */
static ICall_Errno ICall_primSendServiceMsg(ICall_SendArgs *args)
{
  ICall_EntityID dstentity = ICall_searchServiceEntity(args->dest.servId);

  if (dstentity == ICALL_INVALID_ENTITY_ID)
  {
    return ICALL_ERRNO_INVALID_SERVICE;
  }
  args->dest.entityId = dstentity;
  return ICall_primSend(args);
}

/**
 * @internal Retrieves a message received at the message queue
 * associated with the calling thread.
 *
 * Note that this function should be used by an application
 * which does not expect any message from non-server entity.
 *
 * @param args   arguments corresponding to those of ICall_fetchServiceMsg()
 * @return @ref ICALL_ERRNO_SUCCESS when the operation was successful
 *         and a message was retrieved.<br>
 *         @ref ICALL_ERRNO_NOMSG when there is no queued message
 *         at the moment.<br>
 *         @ref ICALL_ERRNO_CORRUPT_MSG when a message queued in
 *         front of the thread's receive queue was not sent by
 *         a server. Note that in this case, the message is
 *         not retrieved but thrown away.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when this function is
 *         called from a thread which has not registered
 *         an entity, either through ICall_enrollService()
 *         or through ICall_registerApp().
 */
static ICall_Errno ICall_primFetchServiceMsg(ICall_FetchMsgArgs *args)
{
  ICall_ServiceEnum servId;
  ICall_Errno errno = ICall_primFetchMsg(args);
  if (errno == ICALL_ERRNO_SUCCESS)
  {
    if (ICall_primEntityId2ServiceId(args->src.entityId, &servId) !=
        ICALL_ERRNO_SUCCESS)
    {
      /* Source entity ID cannot be translated to service id */
      ICall_freeMsg(args->msg);
      return ICALL_ERRNO_CORRUPT_MSG;
    }
    args->src.servId = servId;
  }
  return errno;
}

/**
 * @internal
 * Converts milliseconds to number of ticks.
 * @param msecs milliseconds
 * @param ticks pointer to a variable to store the resultant number of ticks
 * @return @ref ICALL_ERRNO_SUCCESS when successful<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when conversion failed
 *         as the input goes out of range for the output data type.
 */
static ICall_Errno ICall_msecs2Ticks(uint_fast32_t msecs, UInt *ticks)
{
  uint_fast64_t intermediate = msecs;
  intermediate *= 1000;
  intermediate /= Clock_tickPeriod;
  if (intermediate >= ((uint_fast64_t) 1 << (sizeof(UInt)*8 - 1)))
  {
    /* Out of range.
     * Note that we use only half of the range so that client can
     * determine whether the time has passed or time has yet to come.
     */
    return ICALL_ERRNO_INVALID_PARAMETER;
  }
  *ticks = (UInt) intermediate;
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Waits for a signal to the semaphore associated with the calling thread.
 *
 * Note that the semaphore associated with a thread is signaled
 * when a message is queued to the message receive queue of the thread
 * or when ICall_signal() function is called onto the semaphore.
 *
 * @param args  arguments corresponding to those of ICall_wait().
 * @return @ref ICALL_ERRNO_SUCCESS when the semaphore is signaled.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when designated timeout period
 *         has passed since the call of the function without
 *         the semaphore being signaled.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when the milliseconds
 *         is greater than the value of ICall_getMaxMSecs().
 */
static ICall_Errno ICall_primWait(ICall_WaitArgs *args)
{
  Task_Handle taskhandle = Task_self();
  ICall_TaskEntry *taskentry = ICall_searchTask(taskhandle);
  UInt timeout;

  {
    BIOS_ThreadType threadtype = BIOS_getThreadType();

    if (threadtype == BIOS_ThreadType_Hwi ||
        threadtype == BIOS_ThreadType_Swi)
    {
      /* Blocking call is not allowed from Hwi or Swi.
       * Note that though theoretically, Swi or lower priority Hwi may block
       * on an event to be generated by a higher priority Hwi, it is not a
       * safe practice and hence it is disabled.
       */
      return ICALL_ERRNO_UNKNOWN_THREAD;
    }
  }

  if (!taskentry)
  {
    return ICALL_ERRNO_UNKNOWN_THREAD;
  }
  /* Successful */
  if (args->milliseconds == 0)
  {
    timeout = BIOS_NO_WAIT;
  }
  else if (args->milliseconds == ICALL_TIMEOUT_FOREVER)
  {
    timeout = BIOS_WAIT_FOREVER;
  }
  else
  {
    /* Convert milliseconds to number of ticks */
    ICall_Errno errno = ICall_msecs2Ticks(args->milliseconds, &timeout);
    if (errno != ICALL_ERRNO_SUCCESS)
    {
      return errno;
    }
  }

  if (Semaphore_pend(taskentry->sem, timeout))
  {
    return ICALL_ERRNO_SUCCESS;
  }
  return ICALL_ERRNO_TIMEOUT;
}

/**
 * @internal signals a semaphore
 * @param args  arguments corresponding to those of ICall_signal()
 * @return return value corresponding to those of ICall_signal()
 */
static ICall_Errno ICall_primSignal(ICall_SignalArgs *args)
{
  Semaphore_post(args->sem);
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal aborts program
 * @param args  arguments corresponding to those of ICall_abort()
 * @return return value corresponding to those of ICall_abort()
 */
static ICall_Errno ICall_primAbort(ICall_FuncArgsHdr *args)
{
  ICALL_HOOK_ABORT_FUNC();
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Enables an interrupt.
 * @param args arguments corresponding to those of ICall_enableInt()
 * @return return values corresponding to those of ICall_enableInt()
 */
static ICall_Errno ICall_primEnableInt(ICall_IntNumArgs *args)
{
  Hwi_enableInterrupt(args->intnum);
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Disables an interrupt.
 * @param args arguments corresponding to those of ICall_disableInt()
 * @return return values corresponding to those of ICall_disableInt()
 */
static ICall_Errno ICall_primDisableInt(ICall_IntNumArgs *args)
{
  Hwi_disableInterrupt(args->intnum);
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Enables master interrupt and context switching.
 * @param args arguments corresponding to those of ICall_enableMInt()
 * @return return values corresponding to those of ICall_enableMInt()
 */
static ICall_Errno ICall_primEnableMInt(ICall_FuncArgsHdr *args)
{
  Hwi_enable();
  Task_enable();
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Disables master interrupt and context switching.
 * @param args arguments corresponding to those of ICall_disableMInt()
 * @return return values corresponding to those of ICall_disableMInt()
 */
static ICall_Errno ICall_primDisableMInt(ICall_FuncArgsHdr *args)
{
  Task_disable();
  Hwi_disable();
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal registers an interrupt service routine
 * @param args  arguments corresponding to those of ICall_registerISR()
 * @return return values corresponding to those of ICall_registerISR()
 */
static ICall_Errno ICall_primRegisterISR(ICall_RegisterISRArgs *args)
{
  Hwi_Params hwiParams;

  Hwi_Params_init(&hwiParams);
  hwiParams.priority = 0xE0; // default all registered ints to lowest priority

  if (Hwi_create( args->intnum,
                 (void (*)(UArg))args->isrfunc,
                 &hwiParams,
                 NULL) == NULL)
  {
    ICALL_HOOK_ABORT_FUNC();
    return ICALL_ERRNO_NO_RESOURCE;
  }
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal registers an interrupt service routine
 * @param args  arguments corresponding to those of ICall_registerISR_Ext()
 * @return return values corresponding to those of ICall_registerISR_ext()
 */
static ICall_Errno ICall_primRegisterISR_Ext(ICall_RegisterISRArgs_Ext *args)
{
  Hwi_Params hwiParams;

  Hwi_Params_init(&hwiParams);
  hwiParams.priority = args->intPriority;

  if (Hwi_create( args->intnum,
                  (void (*)(UArg))args->isrfunc,
                  &hwiParams,
                  NULL) == NULL)
  {
    ICALL_HOOK_ABORT_FUNC();
    return ICALL_ERRNO_NO_RESOURCE;
  }
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Gets tick counter value
 * @param args  arguments corresponding to those of ICall_getTicks()
 * @return return values corresponding to those of ICall_getTicks()
 */
static ICall_Errno ICall_primGetTicks(ICall_GetUint32Args *args)
{
  args->value = Clock_getTicks();
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Clock event handler function.
 * This function is used to implement the wakeup scheduler.
 *
 * @param arg  an @ref ICall_ScheduleEntry
 */
static Void ICall_clockFunc(UArg arg)
{
  ICall_ScheduleEntry *entry = (ICall_ScheduleEntry *) arg;

  entry->cback(entry->arg);
}

/**
 * @internal
 * Set up or restart a timer.
 *
 * @param args arguments corresponding to those of ICall_setTimer()
 * @return @ref ICALL_ERRNO_SUCCESS when successful;<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER if timer designated by the
 *              timer ID value was not set up before.
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 *         Check ICall heap size and OS heap size if this happens.
 */
static ICall_Errno ICall_primSetTimer(ICall_SetTimerArgs *args)
{
  ICall_ScheduleEntry *entry;

  if (args->timerid == ICALL_INVALID_TIMER_ID)
  {
    Clock_Params params;

    /* Create a new timer */
    entry = ICall_heapMalloc(sizeof(ICall_ScheduleEntry));
    if (entry == NULL)
    {
      /* allocation failed */
      return ICALL_ERRNO_NO_RESOURCE;
    }
    Clock_Params_init(&params);
    params.startFlag = FALSE;
    params.period = 0;
    params.arg = (UArg) entry;
    entry->clock = Clock_create(ICall_clockFunc,
                                args->timeout,
                                &params, NULL);
    if (!entry->clock)
    {
      /* abort */
      ICall_abort();
      ICall_heapFree(entry);
      return ICALL_ERRNO_NO_RESOURCE;
    }
    entry->cback = args->cback;
    entry->arg = args->arg;
    args->timerid = (ICall_TimerID) entry;
  }
  else
  {
    ICall_CSState key;

    entry = (ICall_ScheduleEntry *) args->timerid;

    /* Critical section is entered to disable interrupts that might cause call
     * to callback due to race condition */
    key = ICall_enterCriticalSection();
    Clock_stop(entry->clock);
    entry->arg = args->arg;
    ICall_leaveCriticalSection(key);
  }

  Clock_setTimeout(entry->clock, args->timeout);
  Clock_start(entry->clock);

  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Set up or restart a timer.
 *
 * @param args arguments corresponding to those of ICall_setTimerMSecs()
 * @return @ref ICALL_ERRNO_SUCCESS when successful;<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when msecs is greater than
 *              maximum value supported.
 *         @ref ICALL_ERRNO_NO_RESOURCE when ran out of resource.
 *         Check ICall heap size and OS heap size if this happens.
 */
static ICall_Errno ICall_primSetTimerMSecs(ICall_SetTimerArgs *args)
{
  UInt ticks;
  /* Convert to tick time */
  ICall_Errno errno  = ICall_msecs2Ticks(args->timeout, &ticks);

  if (errno != ICALL_ERRNO_SUCCESS)
  {
    return errno;
  }
  args->timeout = ticks;
  return ICall_primSetTimer(args);
}

/**
 * @internal
 * Stops a timer.
 *
 * @param args arguments corresponding to those of ICall_stopTimer()
 *
 * @return @ref ICALL_ERRNO_SUCCESS when successful;<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER
 *              if id is @ref ICALL_INVALID_TIMER_ID.
 */
static ICall_Errno ICall_primStopTimer(ICall_StopTimerArgs *args)
{
  ICall_ScheduleEntry *entry = (ICall_ScheduleEntry *) args->timerid;

  if (args->timerid == ICALL_INVALID_TIMER_ID)
  {
    return ICALL_ERRNO_INVALID_PARAMETER;
  }

  Clock_stop(entry->clock);
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Gets tick period
 * @param args  arguments corresponding to those of ICall_getTickPeriod()
 * @return return values corresponding to those of ICall_getTickPeriod()
 */
static ICall_Errno ICall_primGetTickPeriod(ICall_GetUint32Args *args)
{
  args->value = Clock_tickPeriod;
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal Gets maximum period supported
 * @param args  arguments corresponding to those of ICall_getMaxMSecs()
 * @return return values corresponding to those of ICall_getMaxMSecs()
 */
static ICall_Errno ICall_primGetMaxMSecs(ICall_GetUint32Args *args)
{
  uint_fast64_t tmp = ((uint_fast64_t) 0x7ffffffful) * Clock_tickPeriod;
  tmp /= 1000;
  if (tmp >= 0x80000000ul)
  {
    tmp = 0x7ffffffful;
  }
  args->value  = (uint_least32_t) tmp;
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Waits for a message that matches comparison
 *
 * @param args  arguments corresponding to those of ICall_waitMatch().
 * @return @ref ICALL_ERRNO_SUCCESS when the semaphore is signaled.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when designated timeout period
 *         has passed since the call of the function without
 *         the semaphore being signaled.<br>
 *         @ref ICALL_ERRNO_INVALID_PARAMETER when the milliseconds
 *         is greater than the value of ICall_getMaxMSecs().
 */
static ICall_Errno ICall_primWaitMatch(ICall_WaitMatchArgs *args)
{
  Task_Handle taskhandle = Task_self();
  ICall_TaskEntry *taskentry = ICall_searchTask(taskhandle);
  ICall_MsgQueue prependQueue = NULL;
  uint_fast16_t consumedCount = 0;
  UInt timeout;
  uint_fast32_t timeoutStamp;
  ICall_Errno errno;

  {
    BIOS_ThreadType threadtype = BIOS_getThreadType();

    if (threadtype == BIOS_ThreadType_Hwi ||
        threadtype == BIOS_ThreadType_Swi)
    {
      /* Blocking call is not allowed from Hwi or Swi.
       * Note that though theoretically, Swi or lower priority Hwi may block
       * on an event to be generated by a higher priority Hwi, it is not a
       * safe practice and hence it is disabled.
       */
      return ICALL_ERRNO_UNKNOWN_THREAD;
    }
  }

  if (!taskentry)
  {
    return ICALL_ERRNO_UNKNOWN_THREAD;
  }
  /* Successful */
  if (args->milliseconds == 0)
  {
    timeout = BIOS_NO_WAIT;
  }
  else if (args->milliseconds == ICALL_TIMEOUT_FOREVER)
  {
    timeout = BIOS_WAIT_FOREVER;
  }
  else
  {
    /* Convert milliseconds to number of ticks */
    errno = ICall_msecs2Ticks(args->milliseconds, &timeout);
    if (errno != ICALL_ERRNO_SUCCESS)
    {
      return errno;
    }
  }

  errno = ICALL_ERRNO_TIMEOUT;
  timeoutStamp = Clock_getTicks() + timeout;
  while (Semaphore_pend(taskentry->sem, timeout))
  {
    ICall_FetchMsgArgs fetchArgs;
    ICall_ServiceEnum servId;
    errno = ICall_primFetchMsg(&fetchArgs);
    if (errno == ICALL_ERRNO_SUCCESS)
    {
      if (ICall_primEntityId2ServiceId(fetchArgs.src.entityId, &servId) ==
            ICALL_ERRNO_SUCCESS)
      {
        if (args->matchFn(servId, fetchArgs.dest, fetchArgs.msg))
        {
          /* Matching message found*/
          args->servId = servId;
          args->dest = fetchArgs.dest;
          args->msg = fetchArgs.msg;
          errno = ICALL_ERRNO_SUCCESS;
          break;
        }
      }
      /* Message was received but it wasn't expected one.
       * Add to the prepend queue */
      ICall_msgEnqueue(&prependQueue, fetchArgs.msg);
    }

    /* Prepare for timeout exit */
    errno = ICALL_ERRNO_TIMEOUT;

    /* Keep the decremented semaphore count */
    consumedCount++;
    if (timeout != BIOS_WAIT_FOREVER &&
        timeout != BIOS_NO_WAIT)
    {
      /* Readjust timeout */
      UInt newTimeout = timeoutStamp - Clock_getTicks();
      if (newTimeout == 0 || newTimeout > timeout)
      {
        break;
      }
      timeout = newTimeout;
    }
  }

  /* Prepend retrieved irrelevant messages */
  ICall_msgPrepend(&taskentry->queue, prependQueue);
  /* Re-increment the consumed semaphores */
  for (; consumedCount > 0; consumedCount--)
  {
    Semaphore_post(taskentry->sem);
  }
  return errno;
}

/**
 * @internal
 * Retrieves an entity ID of an entity associated with the calling thread.
 *
 * @param args  arguments corresponding to those of ICall_getEntityId().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when no entity was associated
 *         with the calling thread.
 */
static ICall_Errno ICall_primGetEntityId(ICall_GetEntityIdArgs *args)
{
  Task_Handle taskhandle = Task_self();
  ICall_CSState key;
  size_t i;

  {
    BIOS_ThreadType threadtype = BIOS_getThreadType();

    if (threadtype == BIOS_ThreadType_Hwi ||
        threadtype == BIOS_ThreadType_Swi)
    {
      return ICALL_ERRNO_UNKNOWN_THREAD;
    }
  }

  key = ICall_enterCSImpl();
  for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
  {
    if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
    {
      /* Not found */
      break;
    }
    if (ICall_entities[i].task->task == taskhandle)
    {
      /* Found */
      args->entity = i;
      ICall_leaveCSImpl(key);
      return ICALL_ERRNO_SUCCESS;
    }
  }
  ICall_leaveCSImpl(key);
  return ICALL_ERRNO_UNKNOWN_THREAD;
}

/**
 * @internal
 * Checks whether the calling thread provides the designated service.
 *
 * @param args  arguments corresponding to those of ICall_threadServes().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_UNKNOWN_THREAD when the calling thread is
 *         unrecognized.
 *         @ref ICALL_ERRNO_INVALID_SERVICE if the service id is not enrolled
 *         by any thread.
 */
static ICall_Errno ICall_primThreadServes(ICall_ThreadServesArgs *args)
{
  Task_Handle taskhandle;
  ICall_CSState key;
  size_t i;

  {
    BIOS_ThreadType threadtype = BIOS_getThreadType();

    if (threadtype == BIOS_ThreadType_Hwi ||
        threadtype == BIOS_ThreadType_Swi)
    {
      return ICALL_ERRNO_UNKNOWN_THREAD;
    }
  }

  taskhandle = Task_self();

  key = ICall_enterCSImpl();
  for (i = 0; i < ICALL_MAX_NUM_ENTITIES; i++)
  {
    if (ICall_entities[i].service == ICALL_SERVICE_CLASS_INVALID_ENTRY)
    {
      /* Not found */
      break;
    }
    else if (ICall_entities[i].service == args->servId)
    {
      args->result = (uint_fast8_t)
        (ICall_entities[i].task->task == taskhandle);
      ICall_leaveCSImpl(key);
      return ICALL_ERRNO_SUCCESS;
    }
  }
  ICall_leaveCSImpl(key);
  return ICALL_ERRNO_INVALID_SERVICE;
}

/**
 * @internal
 * Creates an RTOS task.
 *
 * @param args  arguments corresponding to those of ICall_createTask().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when task creation failed.
 */
static ICall_Errno ICall_primCreateTask(ICall_CreateTaskArgs *args)
{
  /* Task_Params is a huge structure.
   * To reduce stack usage, heap is used instead.
   * This implies that ICall_createTask() must be called before heap
   * space may be exhausted.
   */
  Task_Params *params = (Task_Params *) ICall_heapMalloc(sizeof(Task_Params));
  Task_Handle task;

  if (params == NULL)
  {
    return ICALL_ERRNO_NO_RESOURCE;
  }

  Task_Params_init(params);
  params->priority = args->priority;
  params->stackSize = args->stacksize;
  params->arg0 = args->arg;

  task = Task_create((Task_FuncPtr) args->entryfn, params, NULL);
  ICall_heapFree(params);

  if (task == NULL)
  {
    return ICALL_ERRNO_NO_RESOURCE;
  }
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Creates a semaphore.
 *
 * @param args  arguments corresponding to those of ICall_createSemaphore().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_NO_RESOURCE when task creation failed.
 */
static ICall_Errno ICall_primCreateSemaphore(ICall_CreateSemaphoreArgs *args)
{
  /* Semaphore_Params is a huge structure.
   * To reduce stack usage, heap is used instead.
   * This implies that ICall_createSemaphore() must be called before heap
   * space may be exhausted.
   */
  Semaphore_Params *semParams =
    (Semaphore_Params *) ICall_heapMalloc(sizeof(Semaphore_Params));

  if (semParams == NULL)
  {
    return ICALL_ERRNO_NO_RESOURCE;
  }

  Semaphore_Params_init(semParams);
  if (args->mode == ICALL_SEMAPHORE_MODE_BINARY)
  {
    semParams->mode = Semaphore_Mode_BINARY;
  }

  args->sem = Semaphore_create(args->initcount, semParams, NULL);
  ICall_heapFree(semParams);

  if (args->sem == NULL)
  {
    return ICALL_ERRNO_NO_RESOURCE;
  }
  return ICALL_ERRNO_SUCCESS;
}

/**
 * @internal
 * Waits on a semaphore.
 *
 * @param args  arguments corresponding to those of ICall_waitSemaphore().
 * @return @ref ICALL_ERRNO_SUCCESS when successful.<br>
 *         @ref ICALL_ERRNO_TIMEOUT when timeout occurred.
 */
static ICall_Errno ICall_primWaitSemaphore(ICall_WaitSemaphoreArgs *args)
{
  UInt timeout;

  if (args->milliseconds == 0)
  {
    timeout = BIOS_NO_WAIT;
  }
  else if (args->milliseconds == ICALL_TIMEOUT_FOREVER)
  {
    timeout = BIOS_WAIT_FOREVER;
  }
  else
  {
    ICall_Errno errno = ICall_msecs2Ticks(args->milliseconds, &timeout);
    if (errno != ICALL_ERRNO_SUCCESS)
    {
      return errno;
    }
  }
  if (Semaphore_pend(args->sem, timeout))
  {
    return ICALL_ERRNO_SUCCESS;
  }
  return ICALL_ERRNO_TIMEOUT;
}

/**
 * @internal Primitive service function ID to handler function map
 */
static const struct _icall_primsvcfunc_map_entry_t
{
#ifdef COVERAGE_TEST
  size_t id;
#endif /* COVERAGE_TEST */
  ICall_PrimSvcFunc func;
} ICall_primSvcFuncs[] =
{
  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_ENROLL,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primEnroll
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_REGISTER_APP,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primRegisterApp
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_MSG_ALLOC,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primAllocMsg
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_MSG_FREE,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primFreeMsg
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_MALLOC,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primMalloc
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_FREE,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primFree
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_SEND_MSG,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primSend
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_FETCH_MSG,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primFetchMsg
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_SEND_SERV_MSG,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primSendServiceMsg
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_FETCH_SERV_MSG,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primFetchServiceMsg
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_WAIT,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primWait
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_SIGNAL,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primSignal
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_ABORT,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primAbort
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_ENABLE_INT,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primEnableInt
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_DISABLE_INT,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primDisableInt
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_ENABLE_MINT,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primEnableMInt
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_DISABLE_MINT,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primDisableMInt
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_REGISTER_ISR,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primRegisterISR
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_GET_TICKS,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primGetTicks
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_SET_TIMER_MSECS,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primSetTimerMSecs
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_GET_TICK_PERIOD,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primGetTickPeriod
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_GET_MAX_MILLISECONDS,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primGetMaxMSecs
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_ENTITY2SERVICE,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primE2S
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_PWR_UPD_ACTIVITY_COUNTER,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrUpdActivityCounter
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_PWR_REGISTER_NOTIFY,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrRegisterNotify
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_WAIT_MATCH,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primWaitMatch
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_GET_ENTITY_ID,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primGetEntityId
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_SET_TIMER,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primSetTimer
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_STOP_TIMER,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primStopTimer
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_PWR_CONFIG_AC_ACTION,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrConfigACAction
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_PWR_REQUIRE,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrRequire
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_PWR_DISPENSE,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrDispense
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_THREAD_SERVES,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primThreadServes
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_PWR_IS_STABLE_XOSC_HF,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrIsStableXOSCHF
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_PWR_GET_TRANSITION_STATE,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrGetTransitionState
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_CREATE_TASK,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primCreateTask
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_CREATE_SEMAPHORE,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primCreateSemaphore
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_WAIT_SEMAPHORE,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primWaitSemaphore
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_SWITCH_XOSC_HF,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrSwitchXOSCHF
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_PWR_GET_XOSC_STARTUP_TIME,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICallPlatform_pwrGetXOSCStartupTime
  },

  {
#ifdef COVERAGE_TEST
    ICALL_PRIMITIVE_FUNC_REGISTER_ISR_EXT,
#endif /* COVERAGE_TEST */
    (ICall_PrimSvcFunc) ICall_primRegisterISR_Ext
  },

};

/**
 * @internal
 * Primitive service registered handler function
 * @param args   arguments
 * @return error code
 */
static ICall_Errno ICall_primService(ICall_FuncArgsHdr *args)
{
  if (args->func >= sizeof(ICall_primSvcFuncs)/sizeof(ICall_primSvcFuncs[0]))
  {
    return ICALL_ERRNO_INVALID_FUNCTION;
  }
  return ICall_primSvcFuncs[args->func].func(args);
}

/**
 * @internal Enrolls primitive service
 */
static void ICall_initPrim(void)
{
  ICall_entities[0].service = ICALL_SERVICE_CLASS_PRIMITIVE;
  ICall_entities[0].fn = ICall_primService;

  /* Initialize heap */
  ICall_heapInit();

  /* TODO: Think about freezing permanently allocated memory blocks
   * for optimization.
   * Now that multiple stack images may share the same heap.
   * kick cannot be triggered by a single stack image.
   * Hence, maybe there should be an alternative API to
   * permanently allocate memory blocks, such as
   * by allocating the blocks at the end of the heap space. */
}

#ifdef COVERAGE_TEST
/**
 * @internal
 * Verification function for ICall implementation
 */
void ICall_verify(void)
{
  size_t i;
  for (i = 0; i < sizeof(ICall_primSvcFuncs)/sizeof(ICall_primSvcFuncs[0]); i++)
  {
    if (i != ICall_primSvcFuncs[i].id)
    {
      ICall_abort();
    }
  }
}
#endif /* COVERAGE_TEST */
