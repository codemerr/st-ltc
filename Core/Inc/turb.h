#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32f1xx.h"
#include "stm32f1xx_CMSIS.h"


#define TURB_CS_ON GPIOB->BSRR = GPIO_BSRR_BR4
#define TURB_CS_OFF GPIOB->BSRR = GPIO_BSRR_BS4



bool CMSIS_SPI_Data_Transmit_16BIT(SPI_TypeDef* SPI, uint16_t* data, uint16_t Size_data, uint32_t Timeout_ms);
void turb_Init(void);





#ifndef INC_TURB_H_
#define INC_TURB_H_



#endif /* INC_TURB_H_ */
