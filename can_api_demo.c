
#include "can_api_demo.h"
#include "platform.h"
#include <machine.h>
#include "r_can_rx_if.h"
#include "r_can_rx_config.h"
#include <stdio.h>


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

/***********************************************************************************************************************
 * Exported global variables
************************************************************************************************************************/
/* Demo data */
can_frame_t  tx_dataframe, tx_data, rx_dataframe, remote_frame;

/* RSK+ 64M uses CAN0 (channel 0). */
uint32_t    g_can_channel = CH_0;

/* Flags set by the CAN ISRs, so the user app knows it has data tp process.
 * Polled mode will poll the mailboxes instead. */
#if !USE_CAN_POLL
uint32_t    CAN0_tx_sentdata_flag = 0;
uint32_t    CAN0_rx_newdata_flag = 0;
uint32_t    CAN0_tx_remote_sentdata_flag = 0;
uint32_t    CAN0_rx_remote_frame_flag = 0;
#endif

/* Application layer logic error container. Use as desired to record issues. */
enum app_err_enum   app_err_nr;

/***********************************************************************************************************************
Global variables and functions imported (externs)
***********************************************************************************************************************/
/* Data */
/* Functions */

extern void   read_switches(void);

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
/* Errors. Peripheral and bus errors. Space for each channel. */
static uint32_t error_bus_status[NR_CAN_CHANNELS];
static uint32_t error_bus_status_prev[NR_CAN_CHANNELS];
static uint32_t can_state[NR_CAN_CHANNELS];
static uint32_t nr_times_reached_busoff[NR_CAN_CHANNELS];

/* Counts demo frames sent and received. */
static uint32_t nr_frames_rx = 0, nr_frames_tx = 0;

/* Functions */
static uint32_t init_can_app(void);
static void     check_can_errors(void);
static void     handle_can_bus_state(uint8_t ch_nr);

#if USE_CAN_POLL
 static void    can_poll_demo(void);
#else
 static void can_int_demo(void);
 static void my_can_tx0_callback(void);
 static void my_can_rx0_callback(void);
 static void my_can_err0_callback(void);
#endif

/***********************************************************************************************************************
Function name:  main
Parameters:     -
Returns:        -
Description:    Main Can API demo program. See top of file.
***********************************************************************************************************************/

uint8_t rtn = 0;
void main(void)
{
    uint32_t  api_status = R_CAN_OK;

    #if BSP_CFG_IO_LIB_ENABLE
        printf("\n\nStarting Tx-Rx Demo...\n");
        printf("R_CAN_Create() for channel %d, and provided user callback funcs.\n", g_can_channel);
    #endif


    /* Init CAN. */
    #if USE_CAN_POLL
        api_status = R_CAN_Create(g_can_channel, NULL, NULL, NULL);
    #else
        api_status = R_CAN_Create(g_can_channel, my_can_tx0_callback, my_can_rx0_callback, my_can_err0_callback);
    #endif

    if (api_status != R_CAN_OK)
    {
        /* An error at this stage is fatal to demo, so stop here. */
        #if BSP_CFG_IO_LIB_ENABLE
            printf("Demo init error.\n\n");
        #endif
        while (1)
        {
			return 1;
            /* Stop here and leave error displayed. */
        }
    }
	
    /****************************************
    * Pick ONE R_CAN_PortSet call below!    *
    *****************************************/
    /* Normal CAN bus usage. */
    //api_status = R_CAN_PortSet(g_can_channel, CAN_OPERATION);
    api_status = R_CAN_PortSet(g_can_channel, CAN_RESET);
	api_status |= init_can_app();
    /* Initialize CAN mailboxes, and setup the demo receive and transmit dataframe variables. */
    //api_status |= init_can_app();

    /* Is all OK after all CAN initialization? */
    if (api_status != R_CAN_OK)
    {
        api_status = R_CAN_OK;
        app_err_nr = APP_ERR_CAN_INIT;
    }

	printf("Before RxSet\n");
	/*R_CAN_RxSet(g_can_channel, CANBOX(1), 0x0001, DATA_FRAME);
	R_BSP_SoftwareDelay(100, BSP_DELAY_MILLISECS);*/
	rtn = R_CAN_RxSet(g_can_channel, CANBOX(2), 0x0002, DATA_FRAME);
	//rtn = R_CAN_RxSet(g_can_channel, CANBOX(2), 0x0002, REMOTE_FRAME);
	switch(rtn){
	   	case R_CAN_OK:
			printf("R_CAN_RxSet>> R_CAN_OK\n");
			break;
	   	case R_CAN_SW_BAD_MBX:
			printf("R_CAN_RxSet>> R_CAN_SW_BAD_MBX\n");
			break;
	   	case R_CAN_BAD_CH_NR:
			printf("R_CAN_RxSet>> R_CAN_BAD_CH_NR\n");
			break;
	   	case R_CAN_SW_SET_TX_TMO:
			printf("R_CAN_RxSet>> R_CAN_SW_SET_TX_TMO\n");
			break;
	   	case R_CAN_SW_SET_RX_TMO:
			printf("R_CAN_RxSet>> R_CAN_SW_SET_RX_TMO\n");
			break;
	   }
	
	/*R_BSP_SoftwareDelay(100, BSP_DELAY_MILLISECS);
	R_CAN_RxSetMask(g_can_channel, CANBOX(2), 0x000);
	R_BSP_SoftwareDelay(100, BSP_DELAY_MILLISECS);*/
	//can_int_demo();
	printf("After RxSet");
	printf("\n");

    /*  M A I N  L O O P  * * * * * * * * * * * * * * * * * * * * * * * */
    PORT2.PDR.BIT.B6 = 1;
    
    while(1)
    {
	   /*
	   PORT2.PODR.BIT.B6 = 0;
	   R_BSP_SoftwareDelay(100, BSP_DELAY_MILLISECS);
	   PORT2.PODR.BIT.B6 = 1;
	   R_BSP_SoftwareDelay(100, BSP_DELAY_MILLISECS);
	   */
	   
	   rtn = R_CAN_RxPoll(g_can_channel, CANBOX(2));
	   PORTE.PODR.BIT.B1 = 0;
	   switch(rtn){
	   	case R_CAN_OK:
			printf("R_CAN_RxPoll>> R_CAN_OK\n");
			PORTE.PODR.BIT.B4 = 0;
			break;
	   	case R_CAN_NOT_OK:
			printf("R_CAN_RxPoll>> R_CAN_NOT_OK\n");
			PORTE.PODR.BIT.B3 = 0;
			break;
	   	case R_CAN_RXPOLL_TMO:
			printf("R_CAN_RxPoll>> R_CAN_RXPOLL_TMO\n");
			PORTE.PODR.BIT.B2 = 0;
			break;
	   	case R_CAN_SW_BAD_MBX:
			printf("R_CAN_RxPoll>> R_CAN_SW_BAD_MBX\n");
			//PORTE.PODR.BIT.B1 = 0;
			break;
	   	case R_CAN_BAD_CH_NR:
			printf("R_CAN_RxPoll>> R_CAN_BAD_CH_NR\n");
			PORTE.PODR.BIT.B1 = 0;
			PORTE.PODR.BIT.B2 = 0;
			PORTE.PODR.BIT.B3 = 0;
			PORTE.PODR.BIT.B4 = 0;
			break;
	   }
	   R_BSP_SoftwareDelay(50, BSP_DELAY_MILLISECS);
			PORTE.PODR.BIT.B2 = 1;
			PORTE.PODR.BIT.B3 = 1;
			PORTE.PODR.BIT.B4 = 1;
	   R_BSP_SoftwareDelay(50, BSP_DELAY_MILLISECS);
	   
    }
}/* end main() */


#if USE_CAN_POLL
/*****************************************************************************
Function name:  can_poll_demo
Parameters:     -
Returns:        -
Description:    POLLED CAN demo version
*****************************************************************************/
static void can_poll_demo(void)
{
    uint32_t    api_status = R_CAN_OK;

    /*** TRANSMITTED any frames? */
    api_status = R_CAN_TxCheck(g_can_channel, CANBOX_TX);
    if (api_status == R_CAN_OK)
    {
        nr_frames_tx++;
        LED1 = LED_ON;
        #if BSP_CFG_IO_LIB_ENABLE
            printf("TxChk OK\n");
            printf("Sent frame. nr_frames_tx = %d.\n", nr_frames_tx);
        #endif
    }

    /* Since we are always polling for transmits, api_status for R_CAN_TxCheck
      will most often be other than R_CAN_OK, don't show that in this demo.*/

    /*** RECEIVED any frames? */
    api_status = R_CAN_RxPoll(g_can_channel, CANBOX_RX);
    if (api_status == R_CAN_OK)
    {
        nr_frames_rx++;
        LED0 = LED_ON;

        /* Read CAN data and show. */
        api_status = R_CAN_RxRead(g_can_channel, CANBOX_RX, &rx_dataframe);

        /* You can set BP here and check the received data in debugger. */
        R_BSP_NOP();

        #if BSP_CFG_IO_LIB_ENABLE
            printf("Received CAN ID 0x%X, nr_frames_rx = %d.\n", rx_dataframe.id, nr_frames_rx);
        #endif
        if (api_status == R_CAN_MSGLOST)
        {
            #if BSP_CFG_IO_LIB_ENABLE
                printf( "MSGLOST\n");
            #endif
            R_BSP_NOP();
        }
    }
}/* end can_poll_demo() */

#else

/*****************************************************************************
Function name:      can_int_demo
Parameters:         -
Returns:            -
Description:        INTERRUPT driven CAN demo version.
*****************************************************************************/
static void can_int_demo(void)
{
    uint32_t    api_status = R_CAN_OK;

    /*** TRANSMITTED any frames? Only need to check if flag is set by CAN Tx ISR. */
    if (CAN0_tx_sentdata_flag)
    {
        CAN0_tx_sentdata_flag = 0;
        LED1 = LED_ON;
        nr_frames_tx++;
        #if BSP_CFG_IO_LIB_ENABLE
            printf("Sent frame. nr_frames_tx = %d.\n", nr_frames_tx);
        #endif
    }

    /*** RECEIVED any frames? Only need to check if flag is set by CAN Rx ISR.
     Will only receive own frames in CAN port test modes 0 and 1. */
    if (CAN0_rx_newdata_flag)
    {
        CAN0_rx_newdata_flag = 0;
        LED0 = LED_ON;
        nr_frames_rx++;

        /* Read CAN data and show. */
        api_status = R_CAN_RxRead(g_can_channel, CANBOX_RX, &rx_dataframe);

        /* You can set BP here and check the received data in debugger. */
        R_BSP_NOP();

        #if BSP_CFG_IO_LIB_ENABLE
            printf("Received CAN ID 0x%X, nr_frames_rx = %d.\n", rx_dataframe.id, nr_frames_rx);
        #endif

        if (api_status == R_CAN_MSGLOST)
        {
            #if BSP_CFG_IO_LIB_ENABLE
                printf( "MSGLOST\n\n");
            #endif
            R_BSP_NOP();
        }
    }

    #if REMOTE_DEMO_ENABLE
    /* Previous remote reply succeeded? */
    if (CAN0_tx_remote_sentdata_flag)
    {
        CAN0_tx_remote_sentdata_flag = 0;
        #if BSP_CFG_IO_LIB_ENABLE
            printf( "Remote trig.\n");
        #endif
    }

    /* Set up remote reply if remote request came in. */
    if (CAN0_rx_remote_frame_flag == 1)
    {
        CAN0_rx_remote_frame_flag = 0;
        remote_frame.data[0]++;

        if (FRAME_ID_MODE == STD_ID_MODE )
        {
        	api_status |= R_CAN_TxSet(g_can_channel, CANBOX_REMOTE_TX, &remote_frame, DATA_FRAME);
        }
        else
        {
        	api_status |= R_CAN_TxSetXid(g_can_channel, CANBOX_REMOTE_TX, &remote_frame, DATA_FRAME);
        }
    }
    #endif
}/* end can_int_demo() */
#endif

/*****************************************************************************
Function name:  init_can_app
Parameters:     -
Returns:        -
Description:    Initialize CAN mailboxes, and setup the demo receive and trans-
                mit dataframe variables. See top of file for demo description.
                After startup, user calls to transmit demo frame done by SW1Func()
                in file switches.c.
*****************************************************************************/
static uint32_t init_can_app(void)
{
    uint32_t    api_status = R_CAN_OK;
    uint8_t     i;

    can_state[0] = R_CAN_STATUS_ERROR_ACTIVE;

    /* Initialize status for all channels. */
    for (i = 0; i < NR_CAN_CHANNELS; i++)
    {
        error_bus_status[i] = R_CAN_STATUS_ERROR_ACTIVE;
        error_bus_status_prev[i] = R_CAN_STATUS_ERROR_ACTIVE;
    }

    /******** Init demo to receive data ********/
    /* Use API to set one CAN mailbox for demo receive. */
    /* Standard id. Choose value 0-0x07FF (2047). */
    if (FRAME_ID_MODE == STD_ID_MODE)
    {
        api_status |= R_CAN_RxSet(g_can_channel, CANBOX_RX, RX_CANID_DEMO_INIT, DATA_FRAME);

        /* Mask for receive box.
         * 0x7FF = no mask. 0x7FD = mask bit 1, for example; If receive ID is set to 1, both ID 1 and 3 should be received.
         * 0x00 = MASK ALL = receive all frames. */
        R_CAN_RxSetMask(g_can_channel, CANBOX_RX, 0x7FF);
    }
    else
    {
        api_status |= R_CAN_RxSetXid(g_can_channel, CANBOX_RX, RX_CANID_DEMO_INIT, DATA_FRAME);

        /* Mask for receive box.
         * 0x1FFFFFFF = no mask. 0x1FFFFFFD = mask bit 1, for example; If receive ID is set to 1, both
           ID 1 and 3 should be received. */
        R_CAN_RxSetMask( g_can_channel, CANBOX_RX, 0x1FFFFFFF);
    }

    /********Init. demo Tx dataframe RAM structure********
    Standard ID; choose value 0-0x07FF.
    Extended ID; choose value 0-0x1FFFFFFD. */
    tx_dataframe.id         = TX_CANID_DEMO_INIT;
    tx_dataframe.dlc        = 8;
    for (i = 0; i < 8; i++)
    {
        tx_dataframe.data[i]    = i;
    }

    #if REMOTE_DEMO_ENABLE
    /*************** Init. remote dataframe response **********************/
    remote_frame.id = REMOTE_TEST_ID;

    /* Length is specified by the remote request. */

    /* Some data.. */
    for (i = 0; i < 8; i++)
    {
        remote_frame.data[i] = i;
    }

    /* Prepare mailbox for Tx. */
    if (FRAME_ID_MODE == STD_ID_MODE)
    {
        R_CAN_RxSet(g_can_channel, CANBOX_REMOTE_RX, REMOTE_TEST_ID, REMOTE_FRAME);
    }
    else
    {
        R_CAN_RxSetXid(g_can_channel, CANBOX_REMOTE_RX, REMOTE_TEST_ID, REMOTE_FRAME);
    }
    #endif
    /***********************************************************************/

    /* Set frame buffer id so trace shows correct receive ID from start. */
    rx_dataframe.id = RX_CANID_DEMO_INIT;

    app_err_nr |= APP_NO_ERR;

    return (api_status);
} /* end init_can_app */

/*****************************************************************************
Function Name:  check_can_errors
Parameters:     -
Returns:        -
Description:    Check for all possible errors, in app and peripheral. Add
                checking for your app here.
*****************************************************************************/
static void check_can_errors(void)
{
    /* Error passive or more? */
    handle_can_bus_state(g_can_channel);

    if (app_err_nr)
    {
        #if BSP_CFG_IO_LIB_ENABLE
            printf("Check CAN errors. app_err_nr = %d\n", app_err_nr);
        #endif

        app_err_nr = APP_NO_ERR;

        /* Show error to user */
        /* RESET ERRORs with SW1. */
        LED3 = LED_ON;  //RED LED
    }
}/* end check_can_errors() */

/*****************************************************************************
Function name:  handle_can_bus_state()
Parameters:     Bus number
Returns:        -
Description:    Check CAN peripheral bus state.
*****************************************************************************/
static void handle_can_bus_state(uint8_t ch_nr)
{
    /* Has the status register reached error passive or more? */
    if (ch_nr < NR_CAN_CHANNELS)
    {
        error_bus_status[ch_nr] = R_CAN_CheckErr(ch_nr);
    }
    else
    {
        return;
    }

    /* Tell user if CAN bus status changed.
       All Status bits are read only. */
    if (error_bus_status[ch_nr] != error_bus_status_prev[ch_nr])
    {
        switch (error_bus_status[ch_nr])
        {
            /* Error Active. */
            case R_CAN_STATUS_ERROR_ACTIVE:
                /* Only report if we recovered from Error Passive or Bus Off. */
                if (error_bus_status_prev[ch_nr] > R_CAN_STATUS_ERROR_ACTIVE)
                {
                    #if BSP_CFG_IO_LIB_ENABLE
                        printf("CAN channel %d R_CAN_STATUS_ERROR_ACTIVE\n\n", ch_nr);
                    #endif
                }
                /* Restart if returned from Bus Off. */
                if (error_bus_status_prev[ch_nr] == R_CAN_STATUS_BUSOFF)
                {
                    #if BSP_CFG_IO_LIB_ENABLE
                        printf("CAN channel %d returned from Bus Off. REINIT CAN PERIPHERAL.\n\n", ch_nr);
                    #endif

                    /* Restart CAN */
                    #if USE_CAN_POLL
                        if (R_CAN_Create(g_can_channel, NULL, NULL, NULL))
                    #else
                        if (R_CAN_Create(g_can_channel, my_can_tx0_callback, my_can_rx0_callback, my_can_err0_callback) != R_CAN_OK)
                    #endif

                    /* Restart CAN demos even if only one channel failed. */
                    init_can_app();
                }
                break;

            /* Error Passive. */
            case R_CAN_STATUS_ERROR_PASSIVE:
                #if BSP_CFG_IO_LIB_ENABLE
                    printf("CAN channel %d R_CAN_STATUS_ERROR_PASSIVE\n\n", ch_nr);
                #endif
                break;

            case R_CAN_STATUS_BUSOFF:
            default:
                nr_times_reached_busoff[ch_nr]++;
                #if BSP_CFG_IO_LIB_ENABLE
                    printf("CAN channel %d R_CAN_STATUS_BUSOFF\n\n", ch_nr);
                #endif
                break;
        }
        error_bus_status_prev[ch_nr] = error_bus_status[ch_nr];
    }
}/* end handle_can_bus_state() */

/*******************************************************************************
Function name:  reset_all_errors()
Description:    Reset all types of errors, application and CAN peripeheral errors.
Parameters:     g_can_channel   Channel number to reset.
Return value:   CAN API code
*******************************************************************************/
uint32_t reset_all_errors(uint8_t g_can_channel)
{
    uint32_t status = 0;

    /* Reset errors */
    app_err_nr = APP_NO_ERR;

    error_bus_status[0] = 0;
    error_bus_status[1] = 0;

    /* You can chooose to not reset error_bus_status_prev; if there was an error,
    keep info to signal recovery */
    error_bus_status_prev[0] = 0;
    error_bus_status_prev[1] = 0;

    nr_times_reached_busoff[0] = 0;
    nr_times_reached_busoff[1] = 0;

    /* Reset Error Judge Factor and Error Code registers */
    CAN0.EIFR.BYTE = 0;

    /* Reset CAN0 Error Code Store Register (ECSR). */
    CAN0.ECSR.BYTE = 0;

    /* Reset CAN0 Error Counters. */
    CAN0.RECR = 0;
    CAN0.TECR = 0;

    return (status);
}/* end reset_all_errors() */


/*******************************************************************************

CAN INTERRRUPTS
Interrupts are duplicated for each CAN channel used except for the Error
interrupt which handles all channels in a group.
Vectors are set according to the channel.

*******************************************************************************/
#if !USE_CAN_POLL
/*****************************************************************************
Name:CAN ISRs
Parameters:-
Returns:-
Description:CAN interrupt routine examples.
*****************************************************************************/
/*****************************************************************************
Function Name:  my_can_tx0_callback
Parameters:     -
Returns:        -
Description:    CAN0 Transmit interrupt.
                Check which mailbox transmitted data and process it.
*****************************************************************************/
static void my_can_tx0_callback(void)
{
    uint32_t api_status = R_CAN_OK;

    api_status = R_CAN_TxCheck(CH_0, CANBOX_TX);
    if (api_status == R_CAN_OK)
    {
        CAN0_tx_sentdata_flag = 1;
    }

    #if REMOTE_DEMO_ENABLE
    api_status = R_CAN_TxCheck(CH_0, CANBOX_REMOTE_TX);
    if (api_status == R_CAN_OK)
        CAN0_tx_remote_sentdata_flag = 1;
    #endif

    /* Use mailbox search reg. Should be faster than above if a lot of mailboxes to check.
    Not verified. */
}/* end can_tx0_callback() */

/*****************************************************************************
Function Name:  my_can_rx0_callback
Parameters:     -
Returns:        -
Description:    CAN0 Receive interrupt. Check which mailbox received data and process it.
*****************************************************************************/
static void my_can_rx0_callback(void)
{
    uint32_t api_status = R_CAN_OK;

    if (api_status == R_CAN_OK) /* So compiler doesn't complain when not using remote frames. */
    {
        CAN0_rx_newdata_flag = 1;
    }

    #if REMOTE_DEMO_ENABLE
    api_status = R_CAN_RxPoll(CH_0, CANBOX_REMOTE_RX);
    if (api_status == R_CAN_OK)
    {
        /* REMOTE_FRAME FRAME REQUEST RECEIVED */
        /* Do not set BP on the next line to check for Remote frame. By the time you
        continue, the recsucc flag will already have changed to be a trmsucc flag in
        the CAN status reg. */

        /* Reset of the receive/transmit flag in the MCTL register will be done by
        set_remote_reply_std_CAN0(). */

        /* Tell application. */
        CAN0_rx_remote_frame_flag = 1;

        remote_frame.dlc = (uint8_t)(CAN0.MB[CANBOX_REMOTE_RX].DLC);

        /* Reset NEWDATA flag since we won't be reading the mailbox. */
        CAN0.MCTL[CANBOX_REMOTE_RX].BIT.RX.NEWDATA = 0;

        /* Confirmation of receival of remote flag is done by remote frame being sent,
        so no need flag application we received remote request. */
        CAN0_rx_newdata_flag = 0;
    }
    #endif

    /* Use mailbox search reg. Should be faster if a lot of mailboxes to check.
    Not verified. */
}/* end my_can_rx0_callback() */

/*****************************************************************************
Function Name:  my_can_err0_callback
Parameters:     -
Returns:        -
Description:    CAN0 Error interrupt.
*****************************************************************************/
static void my_can_err0_callback(void)
{
    /* Error interrupt can have multiple sources. Check interrupt flags to id source. */
    if (IS(CAN0, ERS0))
    {
        #if ERROR_DIAG
            uint8_t         err_int_factor, errcode_store;
            static uint8_t  err_int_factor_accumulate, errcode_store_accumulate;

            LED3 = LED_ON;

            /* Cause identification. */
            err_int_factor = CAN0.EIFR.BYTE;
            err_int_factor_accumulate |= CAN0.EIFR.BYTE;
            errcode_store = CAN0.ECSR.BYTE;
            errcode_store_accumulate |= CAN0.ECSR.BYTE;

            /* Clear EIFR and ECSR. Do a byte-write to avoid read-modify-write with HW writing another bit inbetween. (HW manual says
             * "When a single bit is set to 0 by a program, do not use the logic operation instruction (AND) ??use the transfer
             * instruction (MOV) to ensure that only the specified bit is set to 0 and the other bits are set to 1. Writing 1
             * has no effect to these bit values.)" */
            CAN0.EIFR.BYTE = 0;
            CAN0.ECSR.BYTE = 0;

            #if BSP_CFG_IO_LIB_ENABLE
                printf("CAN0 error diag: \nerr_int_factor 0x%X, accumulated value 0x%X\n", err_int_factor, err_int_factor_accumulate);
                printf("errcode_store 0x%X, accumulated value 0x%X\n", errcode_store, errcode_store_accumulate);
            #endif
        #endif

        /* Set BP here to catch cause. */
        R_BSP_NOP();

        /* Clear interrupt. */
        CLR(CAN0, ERS0) = 1;
    }
}/* end my_can_err0_callback() */

#ifdef CAN1
/*****************************************************************************
Function Name:  my_can_err1_callback
Parameters:     -
Returns:        -
Description:    CAN1 Error interrupt.
*****************************************************************************/
void my_can_err1_callback(void)
{
    /* Error interrupt can have multiple sources. Check interrupt flags to id source. */
    if (IS(CAN1, ERS1))
    {
        #if ERROR_DIAG
            uint8_t         err_int_factor, errcode_store;
            static uint8_t  err_int_factor_accumulate, errcode_store_accumulate;

            LED3 = LED_ON;

            /* Cause identification. */
            err_int_factor = CAN1.EIFR.BYTE;
            err_int_factor_accumulate |= CAN1.EIFR.BYTE;
            errcode_store = CAN1.ECSR.BYTE;
            errcode_store_accumulate |= CAN1.ECSR.BYTE;

            /* Clear EIFR and ECSR. Do a byte-write to avoid read-modify-write with HW writing another bit inbetween. (HW manual says
             * "When a single bit is set to 0 by a program, do not use the logic operation instruction (AND) ??use the transfer
             * instruction (MOV) to ensure that only the specified bit is set to 0 and the other bits are set to 1. Writing 1
             * has no effect to these bit values.)" */
            CAN1.EIFR.BYTE = 0;
            CAN1.ECSR.BYTE = 0;

            #if BSP_CFG_IO_LIB_ENABLE
                printf("CAN1 error diag: \nerr_int_factor 0x%X, accumulated value 0x%X\n", err_int_factor, err_int_factor_accumulate);
                printf("errcode_store 0x%X, accumulated value 0x%X\n", errcode_store, errcode_store_accumulate);
            #endif
        #endif //ERROR_DIAG

        /* Set BP here to catch cause. */
        R_BSP_NOP();

        /* Clear interrupt. */
        CLR(CAN1, ERS1) = 1;
    }
}/* end my_can_err1_callback() */
#endif

#if(0)
#ifdef CAN2
/*****************************************************************************
Function Name:  my_can_err2_callback
Parameters:     -
Returns:        -
Description:    CAN2 Error interrupt.
*****************************************************************************/
static void my_can_err2_callback(void)
{
    /* Error interrupt can have multiple sources. Check interrupt flags to id source. */
    if (IS(CAN2, ERS2))
    {
        #if ERROR_DIAG
            uint8_t         err_int_factor, errcode_store;
            static uint8_t  err_int_factor_accumulate, errcode_store_accumulate;

            LED3 = LED_ON;

            /* Cause identification. */
            err_int_factor = CAN2.EIFR.BYTE;
            err_int_factor_accumulate |= CAN2.EIFR.BYTE;
            errcode_store = CAN2.ECSR.BYTE;
            errcode_store_accumulate |= CAN2.ECSR.BYTE;

            /* Clear EIFR and ECSR. Do a byte-write to avoid read-modify-write with HW writing another bit inbetween. (HW manual says
             * "When a single bit is set to 0 by a program, do not use the logic operation instruction (AND) ??use the transfer
             * instruction (MOV) to ensure that only the specified bit is set to 0 and the other bits are set to 1. Writing 1
             * has no effect to these bit values.)" */
            CAN2.EIFR.BYTE = 0;
            CAN2.ECSR.BYTE = 0;

            #if BSP_CFG_IO_LIB_ENABLE
                printf("CAN2 error diag: \nerr_int_factor 0x%X, accumulated value 0x%X\n", err_int_factor, err_int_factor_accumulate);
                printf("errcode_store 0x%X, accumulated value 0x%X\n", errcode_store, errcode_store_accumulate);
            #endif
        #endif //ERROR_DIAG

        /* Set BP here to catch cause. */
        R_BSP_NOP();

        /* Clear interrupt. */
        CLR(CAN2, ERS2) = 1;
    }
}/* end my_can_err2_callback() */
#endif
#endif
#endif  //USE_CAN_POLL

