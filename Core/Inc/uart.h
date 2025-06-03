#include "main.h"
#include <stdarg.h>


void USART_Init();
void USART_Transmit(USART_TypeDef *USART, uint8_t *data, uint16_t Size);
void USART_Start_Menu();
void SendTextUSART(const char* format, ...);
char ugetchar();




extern struct USART_name husart1;
extern const char* NamePerif[];





struct USART_name {
	uint8_t tx_buffer[255]; //Буфер под выходящие данные
	uint8_t rx_buffer[255]; //Буфер под входящие данные
	uint16_t rx_counter; //Счетчик приходящих данных типа uint8_t по USART
	uint16_t rx_len; //Количество принятых байт после сработки флага IDLE
};









#ifndef INC_UART_H_
#define INC_UART_H_



#endif /* INC_UART_H_ */
