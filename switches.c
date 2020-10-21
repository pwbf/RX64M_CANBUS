/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : can_demo.c
* Description  : CAN API demo. Main application program. See operation below under DEMO DESCRIPTION.
************************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 30.10.2015 2.02    Updated for FIT.
*         : 3.3.2016   2.10    65N added. Function names changed_to_this_style().
*         : 1.30.2017  2.11    - Test run with 65N-2MB.
*                              - Added ERROR_DIAG macro. Use for bus error diagnostics only.
*                              - Removed all USE_LCD code. Using debug console (printf) instead.
*                                Added corresponding trace code to demo.
*                              - Function names changed_to_this_style().
*                              - Some GSCE coding guidelines implemented. Mulitple lines changed. (Plugin was used.)
***********************************************************************************************************************/
/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include <machine.h>
#include "platform.h"
#include "r_can_rx_config.h"
#include "r_can_rx_if.h"
#include "can_api_demo.h"
#include <stdio.h>

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* SWITCHES. Low level = pressed.
For 64M, 71M:
SW1 is on PORT1, BIT5
SW2 is on PORT1, BIT2
SW3 is on PORT0, BIT7   */

/* logical switch combination byte patterns */
#define     SW1_MASK        (0x20)    /* 00100000. Switch 1 mask */
#define     SW2_MASK        (0x04)    /* 00000100. Switch 2 mask */
#define     SW3_MASK        (0x80)    /* 10000000. Switch 3 mask */
#define     SW12_MASK       (0x24)    /* 00100100. Switch 1 & 2 mask */
#define     SW23_MASK       (0x84)    /* 10000100. Switch 2 & 3 mask */
#define     SW123_MASK      (0x00)    /* 00000000. All Switches mask */
#define     SW_NONE         (0xA4)    /* 10100100. No Switch */

#define     SW1_PRESSED     (0x84)    /* 10000100. Switch 1 pressed only */
#define     SW2_PRESSED     (0xA0)    /* 10100000. Switch 2 pressed only */
#define     SW3_PRESSED     (0x24)    /* 00100100. Switch 3 pressed only */
#define     SW23_PRESSED    (0x20)    /* 00100000. Switch 3 & 2 pressed only */

/* Nr times switch-poll must match */
#define     SW_COUNT        (0x10)
/* Switch func. table */
#define     SW_TBL_SIZE     (6)

/***********************************************************************************************************************
Exported global variables
***************************************************************************************************/
/* Data */
/* Functions */

/***********************************************************************************************************************
Global variables and functions imported (externs)
**************************************************************************************************/
/* Data */
extern uint32_t g_can_channel;
/* Functions */

/***********************************************************************************************************************
Global variables and functions private to the file
***************************************************************************************************/
/* Data */
/* Switch */
uint8_t last_sw_data;         /* Last switch data */
uint8_t fix_sw_data;          /* Fixed switch data */
uint8_t prev_fix_sw_data;     /* Last fixed switch data */
uint8_t sw_count;             /* Switch counter */

/* Functions */
static uint8_t  are_switches_stable(uint8_t);
static uint8_t  switch_decode(void);
static void     sw1_func(void);
static void     sw2_func(void);
static void     sw3_func(void);
static void     sw23_func(void);
static void     sw32_func(void);
static void     sw_none_func(void);

/* An array of constant pointers to the switch functions */
void (* const switch_func_table[SW_TBL_SIZE])(void) = {   sw_none_func,
                                                        sw1_func,
                                                        sw2_func,
                                                        sw3_func,
                                                        sw32_func,
                                                        sw23_func   };

/***********************************************************************************************************************
Name:          read_switches
Parameters:    -
Returns:       -
Description:   Switch function
**************************************************************************************************/
void read_switches(void)
{
		//printf("Read switch\n");
    uint8_t i, current_sw_pos; 

    /* Build a switch byte pattern */
    current_sw_pos = SW_NONE;               /* initialize all switch bits to 1 */

    if (SW1 == SW_ACTIVE)
    {
        current_sw_pos &= SW1_PRESSED;    /* SW1 is pressed. Clear SW1 bit */
		//printf("SW1\n");
    }

    if (SW2 == SW_ACTIVE)
    {
        current_sw_pos &= SW2_PRESSED;    /* SW2 is pressed. Clear SW2 bit */
		//printf("SW2\n");
    }

    if (SW3 == SW_ACTIVE)
    {
        current_sw_pos &= SW3_PRESSED;    /* SW3 is pressed. Clear SW3 bit */
		//printf("SW3\n");
    }

    if (are_switches_stable(current_sw_pos))
    {
        /* Switch pressed, find out which */
        i = switch_decode();
        if (i < SW_TBL_SIZE)
        {   /* Switch function table. Call the corresponding switch func */
            switch_func_table[i]();
        }
        else
        {
            app_err_nr |= APP_TABLE_ERR;
        }
    }
}

/***********************************************************************************************************************
Name:          sw_func
Parameters:    Current switch 1-3 positions.
Returns:       switches_stable. 1 = stable change. 0 - no stable change.
Description:   Fix switch function, if read_switches are fix (stable), copy value
***********************************************************************************************************************/
uint8_t are_switches_stable(uint8_t current_sw_pos)
{
    uint8_t switches_stable = 0;
    
    /* If switches have changed, time the change. */
    if ((fix_sw_data != current_sw_pos) && (last_sw_data == current_sw_pos))
    {
        sw_count++;
        /* If read_switches has been constant for SW_COUNT, set fix_sw_data. */
        if (sw_count > SW_COUNT)
        {
            fix_sw_data = current_sw_pos;
            sw_count = 0;
            switches_stable = 1;
        }
    }
    else
    {
        sw_count = 0;
    }
    
    last_sw_data = current_sw_pos;
    return (switches_stable);
}/* end are_switches_stable() */

/***********************************************************************************************************************
Name:          switch_decode
Parameters:    -
Returns:       -
Description:   Decodes which switch has been pressed
***********************************************************************************************************************/
uint8_t switch_decode(void)
{
    uint8_t switch_func_nr = 0;
    
    switch (fix_sw_data)
    {
        /* No switch */
        case SW_NONE:
            switch_func_nr = 0;
            break;

        /* SW1 */
        case SW1_PRESSED:
            if (prev_fix_sw_data == SW_NONE)
                switch_func_nr = 1;
            else
                switch_func_nr = 0;
            break;

        /* SW2 */
        case SW2_PRESSED:
            /* Don't call Switch 2 again after '23' until user has realeased SW2 */
            if (prev_fix_sw_data == SW_NONE)
                switch_func_nr = 2;
            else if (prev_fix_sw_data == SW23_PRESSED)
                switch_func_nr = 0; /* SwDummyFunc */
            else
                switch_func_nr = 0;
            break;
        /* SW3 */
        case SW3_PRESSED:
            /* Don't call Switch 3 after '32' again until user has realeased SW3 */
            if (prev_fix_sw_data == SW_NONE)
                switch_func_nr = 3;
            else if (prev_fix_sw_data == SW23_PRESSED)
                switch_func_nr = 0; /* SwDummyFunc */
            else
                switch_func_nr = 0;
            break;
        /* SW1 & SW2 */
        case SW23_PRESSED:
            /* SW1 --> SW2 */
            if (prev_fix_sw_data == SW3_PRESSED)
                switch_func_nr = 4;
            /* SW2 --> SW1 */
            else if (prev_fix_sw_data == SW2_PRESSED)
                switch_func_nr = 5;
            else
                switch_func_nr = 0;
            break;
        default:
            switch_func_nr = 0;
            break;
    }
    
    prev_fix_sw_data = fix_sw_data;
    return (switch_func_nr);
}/* end switch_decode() */

/***********************************************************************************************************************
Name:          sw1_func
Parameters:    -
Returns:       -
Description:   Switch 1. Sends a test CAN frame.
               A-D Demo.
***********************************************************************************************************************/
void sw1_func(void)
{
    /* Normal demo */
    /* Inc. 1st databyte just to show next press to user. */
    tx_dataframe.data[0]++;
    if (FRAME_ID_MODE == STD_ID_MODE )
    {
        R_CAN_TxSet(g_can_channel, CANBOX_TX, &tx_dataframe, DATA_FRAME);
    }
    else
    {
        R_CAN_TxSetXid(g_can_channel, CANBOX_TX, &tx_dataframe, DATA_FRAME);
    }
}/* end sw1_func() */

/***********************************************************************************************************************
Name:          sw2_func
Parameters:    -
Returns:       -
Description:   Shows test frame CAN TxID.
***********************************************************************************************************************/
void sw2_func(void)
{
#if BSP_CFG_IO_LIB_ENABLE
    printf("Tx ID = 0x%X.\n", tx_dataframe.id);
#endif
}/* end sw2_func() */

/***********************************************************************************************************************
Name:               sw3_func
Parameters:         -
Returns:            -
Description:        Shows test frame CAN RxID. If you are using a CAN
                    mask, the incoming data ID may have overwritten what was set
                    by the user with SW2 & 3.

                    Also RESETS any user ERRORS.
***********************************************************************************************************************/
void sw3_func(void)
{
    #if BSP_CFG_IO_LIB_ENABLE
        printf("Rx ID = 0x%X.\n", rx_dataframe.id);
    #endif
    
    reset_all_errors(g_can_channel);
}/* end sw3_func() */

/***********************************************************************************************************************
Name:              sw23_func
Parameters:        -
Returns:           -
Description:       Switch 2 (hold) then 3 pressed. Increments CAN TX-ID.
***********************************************************************************************************************/
void sw23_func(void)
{
    /* Show user command accepted. */
    LED2 = LED_ON;

    /* Inc. transmit ID */
    tx_dataframe.id++;
    #if BSP_CFG_IO_LIB_ENABLE
        printf("Tx ID changed to 0x%X.\n", tx_dataframe.id);
    #endif
    if (FRAME_ID_MODE == STD_ID_MODE)
    {
        if (tx_dataframe.id > 0x7FF)
        {
            tx_dataframe.id = 0x01;
        }
    }
    else
    {   /* Extended or Mixed ID mode. */
        if (tx_dataframe.id > 0x1FFFFFFF)
        {
            tx_dataframe.id = 0x01;
        }            
    }    
    sw2_func();
}/* end sw23_func() */

/***********************************************************************************************************************
Name:              sw32_func
Parameters:        -
Returns:           -
Description:       Switch 3 (hold) then 2 pressed. Increments CAN RX-ID.
***********************************************************************************************************************/
void sw32_func(void)
{
    /* Show user command accepted. */
    LED3 = LED_ON;

    /* When adjusting rx id, stop all transmission. */
    R_CAN_TxStopMsg(g_can_channel, CANBOX_RX);

    /* If InvalData is flagged we are still receiving. Skip and come back. */
    if (CAN0.MCTL[CANBOX_RX].BIT.RX.INVALDATA == 0)
    {
        /* Stop receiving */
        CAN0.MCTL[CANBOX_RX].BYTE = 0;

        /* Increment receive ID */
        rx_dataframe.id += 1;
        #if BSP_CFG_IO_LIB_ENABLE
            printf("Rx ID changed to 0x%X.\n", rx_dataframe.id);
        #endif

        if (FRAME_ID_MODE == STD_ID_MODE)
        {
            if (rx_dataframe.id > 0x7FF)
            {
                rx_dataframe.id = 0x01;
            }
        }
        else
        {   /* Extended or Mixed ID mode. */
            if (rx_dataframe.id > 0x1FFFFFFF)
            {
                rx_dataframe.id = 0x01;
            }            
        } 

        /* Set new ID and start receiving */
        if (FRAME_ID_MODE == STD_ID_MODE)
        {
            R_CAN_RxSet(g_can_channel, CANBOX_RX, rx_dataframe.id, DATA_FRAME);
        }
        else
        {   /* Extended or Mixed ID mode. */
            R_CAN_RxSetXid(g_can_channel, CANBOX_RX, rx_dataframe.id, DATA_FRAME);            
        } 
        sw3_func();
    }
}/* end sw32_func() */

/***********************************************************************************************************************
Name:          sw_none_func
Parameters:    -
Returns:       -
Description:   This function will run if no switch is pressed, or there is 
               no desirable switchfunction to run. Does nothing.
***********************************************************************************************************************/
void sw_none_func(void)
{
    R_BSP_NOP();
}
/* file end */
