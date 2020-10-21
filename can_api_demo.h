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
* File Name    : can_demo.h
* Description  : CAN API demo. Main application program. See operation below under DEMO DESCRIPTION.
************************************************************************************************************************
*         : 02.20.2015 2.01    For 64M, 71M. Originates from RX63N.
*         : 30.10.2015 2.02    FIT update, new R_CAN_Create interface.
*         : 3.3.2016   2.10    65N added.
*         : 1.30.2017  2.11    - Test run with 65N-2MB.
*                              - Function names changed_to_this_style().
*                              - Some GSCE coding guidelines implemented. Mulitple lines changed. (Plugin was used.)
************************************************************************************************************************/

#ifndef API_DEMO_H
#define API_DEMO_H

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "r_can_rx_config.h"
#include "r_can_rx_if.h"
#include "rskrx64m.h"

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/* Application error codes */
enum app_err_enum {    APP_NO_ERR 			= 0x00,
                       APP_ERR_CAN_PERIPH 	= 0x01,
					   APP_ERR_CAN_INIT 	= 0x02,
					   APP_TABLE_ERR 		= 0x04,
                       APP_STATE_ERR 		= 0x08
					   /* Add your application errors/alarms here. */
                  };

/* CAN channel numbers */				  
enum CAN_channel_num {	CH_0 = 0,
						CH_1 = 1,
						CH_2 = 2
					 };
						
/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* Mailboxes used for demo. Keep mailboxes 4 apart if you want masks 
independent - not affecting neighboring mailboxes. */
#define 	CANBOX_TX		    (0x00)      /* First mailbox */
#define 	CANBOX(MB_NUM)		(MB_NUM)      /* mailbox number*/
#define 	CANBOX_RX 		    (0x04)
#define 	CANBOX_REMOTE_RX	(0x08)
#define 	CANBOX_REMOTE_TX	(0x0C)	    /* Mailbox #12 */


bool CANBUS_Init(void);
bool CANBUS_Read();
bool CANBUS_Send();

/**********************************************************************************************************************
Global variables and functions exported
***********************************************************************************************************************/
/* Data */
extern can_frame_t		tx_dataframe, rx_dataframe, remote_frame;

#if !USE_CAN_POLL
extern uint32_t         CAN1_tx_sentdata_flag;
extern uint32_t			CAN1_tx_err_sentdata_flag;
extern uint32_t			CAN1_rx_newdata_flag;
extern uint32_t			CAN1_rx_test_newdata_flag;
#endif

extern enum app_err_enum	app_err_nr;

/* Functions */
uint32_t reset_all_errors(uint8_t g_can_channel);

#endif //API_DEMO_H
/* file end */
