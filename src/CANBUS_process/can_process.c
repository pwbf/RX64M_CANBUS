#include "can_process.h"


bool CANBUS_Init(void){
   uint32_t  api_status = R_CAN_OK;

    #if BSP_CFG_IO_LIB_ENABLE
        printf("\n\nStarting Tx-Rx Demo...\n");
        printf("R_CAN_Create() for channel %d, and provided user callback funcs.\n", g_can_channel);
    #endif


    /* Init CAN. */
        api_status = R_CAN_Create(g_can_channel, NULL, NULL, NULL);
   	//api_status = R_CAN_Create(g_can_channel, my_can_tx0_callback, my_can_rx0_callback, my_can_err0_callback);
   

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
	//api_status |= init_can_app();
    /* Initialize CAN mailboxes, and setup the demo receive and transmit dataframe variables. */
    //api_status |= init_can_app();

    /* Is all OK after all CAN initialization? */
    if (api_status != R_CAN_OK)
    {
        api_status = R_CAN_OK;
        app_err_nr = APP_ERR_CAN_INIT;
    }

}

bool CANBOX_SET(void){
	printf(">> RxSetXid MAILBOX=%d CANID=0x%X\n", MAILBOX_ID_SPEED_INFO, CANID_WHEEL_SPEED_INFO);
	CAN_SET_STATUS[0] = R_CAN_RxSetXid(g_can_channel, MAILBOX_ID_SPEED_INFO, CANID_WHEEL_SPEED_INFO, DATA_FRAME);
	R_CAN_RxSetMask(g_can_channel, MAILBOX_ID_SPEED_INFO, 0x7FF);
	R_BSP_SoftwareDelay(10, BSP_DELAY_MILLISECS);
	
	printf(">> RxSetXid MAILBOX=%d CANID=0x%X\n", MAILBOX_ID_TRANS_CTRL, CANID_TRANSMISSION_CTRL);
	CAN_SET_STATUS[1] = R_CAN_RxSetXid(g_can_channel, MAILBOX_ID_TRANS_CTRL, CANID_TRANSMISSION_CTRL, DATA_FRAME);
	R_CAN_RxSetMask(g_can_channel, MAILBOX_ID_SPEED_INFO, 0x7FF);
	R_BSP_SoftwareDelay(10, BSP_DELAY_MILLISECS);
	
#if SHOW_DEBUG_MESSAGE == CONFIG_ENABLE	
	printf("======================\n");
	SHOW_SET_STATUS(MAILBOX_ID_SPEED_INFO, CAN_SET_STATUS[0]);
	SHOW_SET_STATUS(MAILBOX_ID_TRANS_CTRL, CAN_SET_STATUS[1]);
	printf("======================\n");
#endif
}


void SHOW_RX_STATUS(uint8_t mailbox_code, uint32_t status_code){
	printf("MailBox[%d]>> ", mailbox_code);
	switch(status_code){
	   	case R_CAN_OK:
			printf("R_CAN_OK\n");
			PORTE.PODR.BIT.B4 = 0;
			break;
	   	case R_CAN_NOT_OK:
			//printf("R_CAN_NOT_OK\n");
			PORTE.PODR.BIT.B3 = 0;
			break;
	   	case R_CAN_RXPOLL_TMO:
			printf("R_CAN_RXPOLL_TMO\n");
			PORTE.PODR.BIT.B2 = 0;
			break;
	   	case R_CAN_SW_BAD_MBX:
			printf("R_CAN_SW_BAD_MBX\n");
			break;
	   	case R_CAN_BAD_CH_NR:
			printf("R_CAN_BAD_CH_NR\n");
			break;
	   }
}

void SHOW_SET_STATUS(uint8_t mailbox_code, uint32_t status_code){
	printf("Setup MB[%d]>> ", mailbox_code);
	switch(status_code){
	   	case R_CAN_OK:
			printf("R_CAN_OK\n");
			PORTE.PODR.BIT.B4 = 0;
			break;
	   	case R_CAN_SW_BAD_MBX:
			printf("R_CAN_SW_BAD_MBX\n");
			PORTE.PODR.BIT.B3 = 0;
			break;
	   	case R_CAN_BAD_CH_NR:
			printf("R_CAN_BAD_CH_NR\n");
			PORTE.PODR.BIT.B2 = 0;
			break;
	   	case R_CAN_SW_SET_TX_TMO:
			printf("R_CAN_SW_SET_TX_TMO\n");
			break;
	   	case R_CAN_SW_SET_RX_TMO:
			printf("R_CAN_SW_SET_RX_TMO\n");
			break;
	   }
}

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

    }
}

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
}

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
}

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
}

static void my_can_rx0_callback(void)
{
    uint32_t api_status = R_CAN_OK;

    if (api_status == R_CAN_OK) /* So compiler doesn't complain when not using remote frames. */
    {
        // CAN0_rx_newdata_flag = 1;
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
        // CAN0_rx_newdata_flag = 0;
    }
    #endif

    /* Use mailbox search reg. Should be faster if a lot of mailboxes to check.
    Not verified. */
}/* end my_can_rx0_callback() */

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