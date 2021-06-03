#ifndef CAN_PROCESS_H
#define CAN_PROCESS_H

#include "platform.h"
#include "r_can_rx_config.h"
#include "r_can_rx_if.h"
#include "CAN_REC.h"
#include <machine.h>

/* TEST CAN ID */
#if (FRAME_ID_MODE == STD_ID_MODE)
    #define     TX_CANID_DEMO_INIT  	(0x0A0)    // Max 0x7FF if Standard ID.
    #define     RX_CANID_DEMO_INIT     	(0x0A1)    // "-
#else
    #define     TX_CANID_DEMO_INIT     	(uint32_t)(0x000A0000)    ///Max 0x20000000 if Extended CAN ID.
    #define     RX_CANID_DEMO_INIT     	(uint32_t)(0x000A0001)    // "-
#endif

#ifdef CAN2 // Three CAN channels exist on device.
    #define     NR_CAN_CHANNELS             (3)
#else
#ifdef CAN1 // Two CAN channels exist.
    #define     NR_CAN_CHANNELS             (2)
#else       // One CAN channel.
    #define     NR_CAN_CHANNELS             (1)
#endif
#endif

#define     NR_STARTUP_TEST_FRAMES      (99)
#define     NR_LOOPS_RESET_LEDS         (0x00020000)
#define     LED_DELAY                   (0x00200000)

#define     REMOTE_DEMO_ENABLE          (0)   /* 1 on, 0 off. */
#define     REMOTE_TEST_ID              (0x050)

/* Use this to aid in diagnose of problematic CAN bus.
 * Do *NOT* use in production unless error diagnostics is desired in end product.
 * Observe that CAN recovers by itself if things return to mormal, and
 * CAN bus states Error Passive and Bus Off are handled by handle_can_bus_state(). */
#define     ERROR_DIAG                  (0)   /* 1 on, 0 off. USE_CAN_POLL must be 0 if on. */

/* Application error codes */
enum app_err_enum {	APP_NO_ERR 		= 0x00,
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
						
/* Mailboxes used for demo. Keep mailboxes 4 apart if you want masks 
independent - not affecting neighboring mailboxes. */
#define 	CANBOX_TX		(0x00)      /* First mailbox */
#define 	CANBOX(MB_NUM)		(MB_NUM)      /* mailbox number*/
#define 	CANBOX_RX		(0x04)
#define 	CANBOX_REMOTE_RX	(0x08)
#define 	CANBOX_REMOTE_TX	(0x0C)	    /* Mailbox #12 */


bool CANBUS_Init(void);
bool CANBUS_Read();
bool CANBUS_Send();

bool CANBOX_SET(void);

static void     check_can_errors(void);
static void     handle_can_bus_state(uint8_t ch_nr);

static void my_can_tx0_callback(void);
static void my_can_rx0_callback(void);
static void my_can_err0_callback(void);

/* Data */
can_frame_t  tx_dataframe, tx_data, rx_dataframe, remote_frame;

/* RSK+ 64M uses CAN0 (channel 0). */
uint32_t    g_can_channel = CH_0;
uint32_t    CAN0_tx_sentdata_flag = 0;
uint32_t    CAN0_rx_newdata_flag = 0;
uint32_t    CAN0_tx_remote_sentdata_flag = 0;
uint32_t    CAN0_rx_remote_frame_flag = 0;


/* Errors. Peripheral and bus errors. Space for each channel. */
static uint32_t error_bus_status[NR_CAN_CHANNELS];
static uint32_t error_bus_status_prev[NR_CAN_CHANNELS];
static uint32_t can_state[NR_CAN_CHANNELS];
static uint32_t nr_times_reached_busoff[NR_CAN_CHANNELS];

/* Counts demo frames sent and received. */
static uint32_t nr_frames_rx = 0, nr_frames_tx = 0;

enum app_err_enum   app_err_nr;

/* Functions */
uint32_t reset_all_errors(uint8_t g_can_channel);

uint32_t CAN_SET_STATUS[2] = {0x00};
uint32_t CAN_MB_STATUS[2] = {0x00};


void SHOW_SET_STATUS(uint8_t mailbox_code, uint32_t status_code);
void SHOW_RX_STATUS(uint8_t mailbox_code, uint32_t status_code);


#endif