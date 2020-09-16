# RX64M_CANBUS  
## 2020/09/16  
### Mailbox Switching and Frame ID testing  
Mailbox with lower Frame ID will have higher send pirority  
## Using Onboard(RSK) CAN Tranciever works  
Using Ti Tranciever not working, incorrect cycle time  
``` c
	tx_data.id         = 0x0002;
	tx_data.dlc        = 8;
	tx_data.data[0]     = 0xC1;
	tx_data.data[1]     = 0xC3;
	tx_data.data[2]     = 0xC5;
	tx_data.data[3]     = 0xC7;
	tx_data.data[4]     = 0xC9;
	tx_data.data[5]     = 0xCB;
	tx_data.data[6]     = 0xCD;
	tx_data.data[7]     = 0xCF;
	
	R_CAN_TxSet(g_can_channel, CANBOX(1), &tx_data, DATA_FRAME);
	
```