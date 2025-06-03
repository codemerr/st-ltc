#include "ds1720.h"
#include <stdio.h>
#include "stm32f1xx.h"
#include "stm32f1xx_CMSIS.h"
#include "uart.h"


typedef struct {
	int16_t value;
	int16_t rest;
	int16_t hex;
} temp_t;



void DS1720_Init(void) {
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_CNF13_Msk, 0b00 << GPIO_CRH_CNF13_Pos); //PB13 Output Push-pull
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_CNF14_Msk, 0b00 << GPIO_CRH_CNF14_Pos); //PB14 Output Push-pull

	MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE13_Msk, 0b11 << GPIO_CRH_MODE13_Pos); //PB13 50Mhz speed
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE14_Msk, 0b11 << GPIO_CRH_MODE14_Pos); //PB14 50Mhz speed
}







void sendByte(uint8_t byte) {
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_CNF15_Msk, 0b00 << GPIO_CRH_CNF15_Pos); //PB15 Output Push-pull
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE15_Msk, 0b11 << GPIO_CRH_MODE15_Pos); //PB15 50Mhz speed

	for(int i = 0; i < 8; i++){
		uint8_t x = 0x01 & (byte >> i);

		WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR13); // SCL OFF

		if (x != 0)
		{
			WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS15); //dq = 1
		}
		else
		{
			WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR15); //dq = 0
		}

		WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS13); // SCL ON

	}
}
void startConversion(void) {
	WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS14); // Reset ON

	sendByte(0xEE); // Start continuous conversion

	WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR14); // Reset OFF
}

uint16_t getWord(void) {
	uint16_t word = 0x00;

	MODIFY_REG(GPIOB->CRH, GPIO_CRH_CNF15_Msk, 0b01 << GPIO_CRH_CNF15_Pos); // Input Floating
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE15_Msk, 0b00 << GPIO_CRH_MODE15_Pos); // Input mode (без скорости)


	for(int i = 0; i < 16; i++){
		WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR13); // SCL OFF
		word |= ((READ_BIT(GPIOA->IDR, GPIO_IDR_IDR15_Msk) >> GPIO_IDR_IDR15_Pos) & 0x1) << i; // Read DQ
		WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS13); // SCL ON
	}
	return word;
}

temp_t getTemperature(void) {

	  int16_t result, sign;
	  temp_t temperature;
	  WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS14); // Reset ON

	  sendByte(0xAA);
	  result = getWord(); //Read result 9-bit
	  temperature.hex = result;
	  sign = (result & 0x100) ? -1 : 1;

	  if(sign < 0) result |= 0xFF00;

	  temperature.value = result / 2;
	  temperature.rest = 5 * (result % 2);

	  WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR14); // Reset OFF

	  return temperature;
}

temp_t getTHigh(void) {

	  int16_t result, sign;
	  temp_t temperature;
	  WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS14); // Reset ON

	  sendByte(0xA1);
	  result = getWord(); //Read result 9-bit
	  temperature.hex = result;
	  sign = (result & 0x100) ? -1 : 1;

	  if(sign < 0) result |= 0xFF00;

	  temperature.value = result / 2;
	  temperature.rest = 5 * (result % 2);

	  WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR14); // Reset OFF

	  return temperature;
}

temp_t getTLow(void) {

	  int16_t result, sign;
	  temp_t temperature;
	  WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS14); // Reset ON

	  sendByte(0xA2);
	  result = getWord(); //Read result 9-bit
	  temperature.hex = result;
	  sign = (result & 0x100) ? -1 : 1;

	  if(sign < 0) result |= 0xFF00;

	  temperature.value = result / 2;
	  temperature.rest = 5 * (result % 2);

	  WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR14); // Reset OFF

	  return temperature;
}


int ProgDs1720(void) {
	startConversion();
	SendTextUSART("Программирование DS1720:\r\n");

	temp_t temperature = getTemperature();
	Delay_ms(1);
	temp_t tHigh = getTHigh();
	Delay_ms(1);
	temp_t tLow  = getTLow();

	SendTextUSART("\nCurrent temperature: %d.%d\r\n",temperature.value, temperature.rest);
	SendTextUSART("THigh level: %2d.%d\r\n", tHigh.value, tHigh.rest);
	SendTextUSART("TLow level: %2d.%d\r\n", tLow.value, tLow.rest);
	return FuncOk_rtest;
}


