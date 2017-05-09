/**
 * @file  hal_assert.c
 * @brief Abort implementation for service implementation environment.
 */
#include <ICall.h>
#include "OSAL.h"

void halAssertHandler(void)
{
  ICall_abort();
}
