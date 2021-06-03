
#include "platform.h"
#include "CAN_REC.h"
#include "r_can_rx_if.h"
#include "segmentDisplay.h"
// #include "can_process.h"


extern uint32_t CAN_SET_STATUS[2];
extern uint32_t CAN_MB_STATUS[2];

can_frame_t SPEED_DATA;
can_frame_t TRANS_DATA;

void main(void)
{
 	CANBUS_Init();
	CANBOX_SET();
	GPIO_init();
	//while(1)
	DisplayControl(1,2,3,4);
    while(1)
    {
	//CHK_ALL_MB();
	if(STATUS_WHEEL_SPEED_INFO == R_CAN_OK){
		R_CAN_RxRead(CAN_CHANNEL_SET, MAILBOX_ID_SPEED_INFO ,&SPEED_DATA);
		//SHOW_DATA();
	}
	if(STATUS_TRANSMISSION_INFO == R_CAN_OK){
		R_CAN_RxRead(CAN_CHANNEL_SET, MAILBOX_ID_TRANS_CTRL ,&TRANS_DATA);
		//SHOW_DATA();
	}
	SHOW_DATA();
	R_BSP_SoftwareDelay(5, BSP_DELAY_MILLISECS);
	   
    }
}

  
void CHK_ALL_MB(void){
	CAN_MB_STATUS[0] = R_CAN_RxPoll(CAN_CHANNEL_SET, MAILBOX_ID_SPEED_INFO);
	CAN_MB_STATUS[1] = R_CAN_RxPoll(CAN_CHANNEL_SET, MAILBOX_ID_TRANS_CTRL);
	
#if SHOW_DEBUG_MESSAGE == CONFIG_ENABLE
	//printf("======================\n");
	SHOW_RX_STATUS(MAILBOX_ID_SPEED_INFO, CAN_MB_STATUS[0]);
	SHOW_RX_STATUS(MAILBOX_ID_TRANS_CTRL, CAN_MB_STATUS[1]);
	//printf("======================\n");
#endif
}

void SHOW_DATA(void){
	uint32_t speedNow = SPEED_DATA.data[0];
	uint32_t speedDigit[3] = {0,0,0};
	
	if(speedNow < 1000){
		speedDigit[2] = speedNow % 10;
		speedNow /= 10;
		speedDigit[1] = speedNow % 10;
		speedNow /= 10;
		speedDigit[0] = speedNow % 10;
	}
	else{
		speedDigit[2] = 9;
		speedDigit[1] = 9;
		speedDigit[0] = 9;
	}
	printf("Speed Data: 0x%04X(%d)\n", speedNow, speedNow);
	/*printf("Data[0] = 0x%02X(%d)\n", SPEED_DATA.data[0]);
	printf("Data[1] = 0x%02X(%d)\n", SPEED_DATA.data[1]);
	printf("Data[2] = 0x%02X(%d)\n", SPEED_DATA.data[2]);
	printf("Data[3] = 0x%02X(%d)\n", SPEED_DATA.data[3]);
	printf("Data[4] = 0x%02X(%d)\n", SPEED_DATA.data[4]);
	printf("Data[5] = 0x%02X(%d)\n", SPEED_DATA.data[5]);
	printf("Data[6] = 0x%02X(%d)\n", SPEED_DATA.data[6]);
	printf("Data[7] = 0x%02X(%d)\n", SPEED_DATA.data[7]);
	printf("Data[8] = 0x%02X(%d)\n\n", SPEED_DATA.data[8]);*/
	printf("Transmission Data: (%d)\n", TRANS_DATA.data[0]);
	/*printf("Data[0] = 0x%02X(%d)\n", TRANS_DATA.data[0]);
	printf("Data[1] = 0x%02X(%d)\n", TRANS_DATA.data[1]);
	printf("Data[2] = 0x%02X(%d)\n", TRANS_DATA.data[2]);
	printf("Data[3] = 0x%02X(%d)\n", TRANS_DATA.data[3]);
	printf("Data[4] = 0x%02X(%d)\n", TRANS_DATA.data[4]);
	printf("Data[5] = 0x%02X(%d)\n", TRANS_DATA.data[5]);
	printf("Data[6] = 0x%02X(%d)\n", TRANS_DATA.data[6]);
	printf("Data[7] = 0x%02X(%d)\n", TRANS_DATA.data[7]);
	printf("Data[8] = 0x%02X(%d)\n\n", TRANS_DATA.data[8]);*/
	printf("Display: %d %d %d %d\n", TRANS_DATA.data[0],speedDigit[0],speedDigit[1],speedDigit[2]);
	DisplayControl(TRANS_DATA.data[0],speedDigit[0],speedDigit[1],speedDigit[2]);
	printf("\n");
}

void DisplayControl(uint8_t digit3, uint8_t digit2, uint8_t digit1, uint8_t digit0){
	//Display Digit 0
	DIGIT_CTRL_0 = GPIO_LOW;
	DIGIT_CTRL_1 = GPIO_HIGH;
	DIGIT_CTRL_2 = GPIO_HIGH;
	DIGIT_CTRL_3 = GPIO_HIGH;
	SEG_LED_A = chBin[digit0][0];
	SEG_LED_B = chBin[digit0][1];
	SEG_LED_C = chBin[digit0][2];
	SEG_LED_D = chBin[digit0][3];
	SEG_LED_E = chBin[digit0][4];
	SEG_LED_F = chBin[digit0][5];
	SEG_LED_G = chBin[digit0][6];
	SEG_LED_DP = chBin[digit0][7];
	R_BSP_SoftwareDelay(5, BSP_DELAY_MILLISECS);
	
	//Display Digit 1
	DIGIT_CTRL_0 = GPIO_HIGH;
	DIGIT_CTRL_1 = GPIO_LOW;
	DIGIT_CTRL_2 = GPIO_HIGH;
	DIGIT_CTRL_3 = GPIO_HIGH;
	SEG_LED_A = chBin[digit1][0];
	SEG_LED_B = chBin[digit1][1];
	SEG_LED_C = chBin[digit1][2];
	SEG_LED_D = chBin[digit1][3];
	SEG_LED_E = chBin[digit1][4];
	SEG_LED_F = chBin[digit1][5];
	SEG_LED_G = chBin[digit1][6];
	SEG_LED_DP = chBin[digit1][7];
	R_BSP_SoftwareDelay(5, BSP_DELAY_MILLISECS);
	
	//Display Digit 2
	DIGIT_CTRL_0 = GPIO_HIGH;
	DIGIT_CTRL_1 = GPIO_HIGH;
	DIGIT_CTRL_2 = GPIO_LOW;
	DIGIT_CTRL_3 = GPIO_HIGH;
	SEG_LED_A = chBin[digit2][0];
	SEG_LED_B = chBin[digit2][1];
	SEG_LED_C = chBin[digit2][2];
	SEG_LED_D = chBin[digit2][3];
	SEG_LED_E = chBin[digit2][4];
	SEG_LED_F = chBin[digit2][5];
	SEG_LED_G = chBin[digit2][6];
	SEG_LED_DP = chBin[digit2][7];
	R_BSP_SoftwareDelay(5, BSP_DELAY_MILLISECS);
	
	//Display Digit 3
	DIGIT_CTRL_0 = GPIO_HIGH;
	DIGIT_CTRL_1 = GPIO_HIGH;
	DIGIT_CTRL_2 = GPIO_HIGH;
	DIGIT_CTRL_3 = GPIO_LOW;
	SEG_LED_A = chBin[digit3][0];
	SEG_LED_B = chBin[digit3][1];
	SEG_LED_C = chBin[digit3][2];
	SEG_LED_D = chBin[digit3][3];
	SEG_LED_E = chBin[digit3][4];
	SEG_LED_F = chBin[digit3][5];
	SEG_LED_G = chBin[digit3][6];
	SEG_LED_DP = chBin[digit3][7];
	R_BSP_SoftwareDelay(5, BSP_DELAY_MILLISECS);
}

void GPIO_init(void){
	PDR_DIGIT_CTRL_0 = GPIO_OUTPUT;
	PDR_DIGIT_CTRL_1 = GPIO_OUTPUT;
	PDR_DIGIT_CTRL_2 = GPIO_OUTPUT;
	PDR_DIGIT_CTRL_3 = GPIO_OUTPUT;
	PDR_SEG_LED_A = GPIO_OUTPUT;
	PDR_SEG_LED_B = GPIO_OUTPUT;
	PDR_SEG_LED_C = GPIO_OUTPUT;
	PDR_SEG_LED_D = GPIO_OUTPUT;
	PDR_SEG_LED_E = GPIO_OUTPUT;
	PDR_SEG_LED_F = GPIO_OUTPUT;
	PDR_SEG_LED_G = GPIO_OUTPUT;
	PDR_SEG_LED_DP = GPIO_OUTPUT;
}



