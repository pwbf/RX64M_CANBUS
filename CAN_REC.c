
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
	
	//DisplayControl(1,2,3,4);
    while(1)
    {
	CHK_ALL_MB();
	
	//如果CANBUS return = R_CAN_OK, 將數值讀回
	if(STATUS_WHEEL_SPEED_INFO == R_CAN_OK){
		R_CAN_RxRead(CAN_CHANNEL_SET, MAILBOX_ID_SPEED_INFO ,&SPEED_DATA);
	}
	if(STATUS_TRANSMISSION_INFO == R_CAN_OK){
		R_CAN_RxRead(CAN_CHANNEL_SET, MAILBOX_ID_TRANS_CTRL ,&TRANS_DATA);
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
	uint32_t rpmNow = SPEED_DATA.data[0]*256 | SPEED_DATA.data[1];	//0 High, 1 Low
	vehicleSpeed = WHEEL_DIA * rpmNow * 0.1885;				//v = Dia * RPM * 0.1885
	uint8_t speedDigit[3] = {0,0,0};
	vehicleGear = TRANS_DATA.data[2];
	
	//拆解速度值為三位元資訊(for 7段顯示器)
	if(vehicleSpeed < 1000){
		speedDigit[2] = vehicleSpeed % 10;
		vehicleSpeed /= 10;
		speedDigit[1] = vehicleSpeed % 10;
		vehicleSpeed /= 10;
		speedDigit[0] = vehicleSpeed % 10;
	}
	else{
		speedDigit[2] = 9;
		speedDigit[1] = 9;
		speedDigit[0] = 9;
	}
	
	printf("Speed Data: [0] 0x%02X | [1] 0x%02X\n", SPEED_DATA.data[0], SPEED_DATA.data[1]);
	printf("rpmNow: 0x%04X(%d)\n", rpmNow, rpmNow);
	printf("vehicleSpeed: 0x%04X(%d)\n", vehicleSpeed, vehicleSpeed);
	printf("Gear Data: 0x%02X\n", vehicleGear);
	
	//轉換CANBUS數值為實際檔位資訊
	switch(vehicleGear){
		case 0x7C:
			printf("Gear Now[R]\n");
			break;
		case 0x7D:
			printf("Gear Now[N]\n");
			break;
		case 0xFC:
			printf("Gear Now[D]\n");
			break;
		case 0xF1:
			printf("Gear Now[1]\n");
			break;
		case 0xF2:
			printf("Gear Now[2]\n");
			break;
		case 0xF3:
			printf("Gear Now[3]\n");
			break;
		case 0xF4:
			printf("Gear Now[4]\n");
			break;
		case 0xF5:
			printf("Gear Now[5]\n");
			break;
		
	}
	
	printf("Display: %d %d %d %d\n", TRANS_DATA.data[0],speedDigit[0],speedDigit[1],speedDigit[2]);		//檢查會送到7段顯示器上的數值
	// DisplayControl(TRANS_DATA.data[0],speedDigit[0],speedDigit[1],speedDigit[2]); //先不顯示到7段顯示器上
	printf("\n");
}

void DisplayControl(uint8_t digit3, uint8_t digit2, uint8_t digit1, uint8_t digit0){

}

void GPIO_init(void){

}



