#ifndef SEGMENT_DISPLAY_H
#define SEGMENT_DISPLAY_H

/*
 AAAA 
F    B
F    B
 GGGG 
E    C
E    C
 DDDD dp
 
format: [A,B,C,D,E,F,G,dp]
*/

#define GPIO_INPUT	(0)
#define GPIO_OUTPUT	(1)
#define GPIO_LOW	(1)
#define GPIO_HIGH	(0)

#define PDR_DIGIT_CTRL_0	PORTB.PDR.BIT.B0
#define PDR_DIGIT_CTRL_1	PORTB.PDR.BIT.B1
#define PDR_DIGIT_CTRL_2	PORTB.PDR.BIT.B2
#define PDR_DIGIT_CTRL_3	PORTB.PDR.BIT.B3

#define PDR_SEG_LED_A		PORTD.PDR.BIT.B0
#define PDR_SEG_LED_B		PORTD.PDR.BIT.B1
#define PDR_SEG_LED_C		PORTD.PDR.BIT.B2
#define PDR_SEG_LED_D		PORTD.PDR.BIT.B3
#define PDR_SEG_LED_E		PORTD.PDR.BIT.B4
#define PDR_SEG_LED_F		PORTD.PDR.BIT.B5
#define PDR_SEG_LED_G		PORTD.PDR.BIT.B6
#define PDR_SEG_LED_DP		PORTD.PDR.BIT.B7

#define DIGIT_CTRL_0		PORTB.PODR.BIT.B0
#define DIGIT_CTRL_1		PORTB.PODR.BIT.B1
#define DIGIT_CTRL_2		PORTB.PODR.BIT.B2
#define DIGIT_CTRL_3		PORTB.PODR.BIT.B3

#define SEG_LED_A		PORTD.PODR.BIT.B0
#define SEG_LED_B		PORTD.PODR.BIT.B1
#define SEG_LED_C		PORTD.PODR.BIT.B2
#define SEG_LED_D		PORTD.PODR.BIT.B3
#define SEG_LED_E		PORTD.PODR.BIT.B4
#define SEG_LED_F		PORTD.PODR.BIT.B5
#define SEG_LED_G		PORTD.PODR.BIT.B6
#define SEG_LED_DP		PORTD.PODR.BIT.B7

uint8_t chBin[16][8] = {
      /*{A,B,C,D,E,F,G,d}*/
	{0,0,0,0,0,0,1,1}, //0
	{1,0,0,1,1,1,1,1}, //1
	{0,0,1,0,0,1,0,1}, //2
	{0,0,0,0,1,1,0,1}, //3
	{1,0,0,1,1,0,0,1}, //4
	{0,1,0,0,1,0,0,1}, //5
	{1,1,0,0,0,0,0,1}, //6
	{0,0,0,1,1,0,1,1}, //7
	{0,0,0,0,0,0,0,1}, //8
	{0,0,0,1,1,0,0,1}, //9
	{0,0,0,1,0,0,0,0}, //A.
	{0,0,0,0,0,0,0,0}, //B.
	{0,1,1,0,0,0,1,0}, //C.
	{0,0,0,0,0,0,1,0}, //D.
	{0,1,1,0,0,0,0,0}, //E.
	{0,1,1,1,0,0,0,0}, //F.
};

#endif