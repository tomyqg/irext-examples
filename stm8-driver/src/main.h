/*******************************************************************************
 *
 *  Filename:      main.h
 *
 *  Description:   Main functions for Irext STM8 module driver
 *
 *  Created by strawmanbobi 2017-12-31
 *  Copyright (c) 2017 Irext. All rights reserved.
 *
 *******************************************************************************/


#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ir_decode.h"


// #define UART_INT

#define IR_IO_PORT            (GPIOC)
#define IR_PIN                (GPIO_PIN_1)

// UART associated definitions
#define HEADER_SR  0x30
#define HEADER_BT  0x31
#define HEADER_CMD 0x32

#define CATEGORY_LENGTH_SIZE 1
#define SUMMARY_LENGTH_SIZE   4

#define UART_BUFFER_SIZE 128


// IR associated definitions
#define BINARY_SOURCE_SIZE_MAX      1024
#define BINARY_PIECE_SIZE           16

#define IR_KEY_POWER                0
#define IR_KEY_MUTE                 1
#define IR_KEY_VOL_UP               7
#define IR_KEY_VOL_DOWN             8


typedef enum
{
    IR_TYPE_NONE = 0,
    IR_TYPE_TV,
    IR_TYPE_AC,
    IR_TYPE_MAX
} ir_type_t;


typedef enum
{
    IR_STATE_STANDBY = 0,
    IR_STATE_NONE,
    IR_STATE_READY,
    IR_STATE_OPENED,
    IR_STATE_MAX
} ir_state_t;


typedef struct
{
    ir_type_t ir_type;
    ir_state_t ir_state;
    uint16_t recv_index;
    uint8_t source_code[BINARY_SOURCE_SIZE_MAX];
    uint16_t source_code_length;
    uint16_t ir_decoded[USER_DATA_SIZE];
    uint16_t decoded_length;
} decode_control_block_t;


// initialization
void init_GPIO();
void init_UART();
void init_Timer4();

// interrupt handlers
void timer4_callback();

// logic functions
void TransportDataToUart(uint8_t* data, uint16_t len);
void PrintString(uint8_t *str);
void PrintValue(char *content, uint32_t value, uint8_t format);
void WriteBytes(uint8_t *data, uint16_t len);
void WriteValue(char *content, uint32_t value, uint8_t format);


// utilities
void delay(u16 count);


#endif /* __MAIN_H */

#ifdef __cplusplus
}
#endif

