#ifndef CAN_REC_H
#define CAN_REC_H


#define CAN_CHANNEL_SET			(0)

#define CANID_WHEEL_SPEED_INFO		(0x18FEBF0B)
#define CANID_TRANSMISSION_CTRL		(0x0C010305)


#define MAILBOX_ID_SPEED_INFO		(0)
#define MAILBOX_ID_TRANS_CTRL		(1)

#define WHEEL_DIA				(0.5)			/*Diameter of Tire(m)*/

#define CONFIG_DISABLE			(0)
#define CONFIG_ENABLE			(1)

/* CONFIG_ENABLE or CONFIG_DISABLE */
#define SHOW_DEBUG_MESSAGE		CONFIG_ENABLE



#define STATUS_WHEEL_SPEED_INFO		CAN_MB_STATUS[0]
#define STATUS_TRANSMISSION_INFO	CAN_MB_STATUS[1]


void CHK_ALL_MB(void);
void SHOW_DATA(void);

void DisplayControl(uint8_t digit3, uint8_t digit2, uint8_t digit1, uint8_t digit0);
void GPIO_init(void);

uint16_t vehicleSpeed = 0;
uint16_t vehicleGear = 0;

#endif