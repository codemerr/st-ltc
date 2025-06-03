#include "uart.h"



const char* NamePerif[kolv_test]={
		"Все устройства	  ", 	  //a
		"Турбинного вращения    ",//b
		"Сопр. изоляции	  ", 	  //c
		"Прогр. DS1720	  ", 	  //d
		"DS1720		  ",		  //e
		"Реле		  ", 		  //f
		"Счетчик импульсов	  ",  //g
		"Питания		  ", 	  //h
		"Отчет		  ", 		  //i
		"Все устр. кроме питания" //j
};



struct USART_name husart1;



void USART_Init(void){
	//Tx - Alternative Function output Push-pull(Maximum output speed 50 MHz)
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF9_Msk, 0b10 << GPIO_CRH_CNF9_Pos);
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE9_Msk, 0b11 << GPIO_CRH_MODE9_Pos);

	//Rx - Input floating
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF10_Msk, 0b1 << GPIO_CRH_CNF10_Pos);
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE10_Msk, 0b00 << GPIO_CRH_MODE10_Pos);

	//Запуск тактирования USART1
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

//	скорость 115200:
//	115200 = 72000000/(16*USARTDIV)
//	Тогда USARTDIV = 72000000/115200*16 = 39.0625
//	DIV_Mantissa в данном случае будет 39, что есть 0x27
//	DIV_Fraction будет, как 0.0625*16 = 1, что есть 0x1
	MODIFY_REG(USART1->BRR, USART_BRR_DIV_Mantissa_Msk, 0x27 << USART_BRR_DIV_Mantissa_Pos);
	MODIFY_REG(USART1->BRR, USART_BRR_DIV_Fraction_Msk, 0x1 << USART_BRR_DIV_Fraction_Pos);

	SET_BIT(USART1->CR1, USART_CR1_UE); //USART enable
	CLEAR_BIT(USART1->CR1, USART_CR1_M); //Word lenght 1 Start bit, 8 Data bits, n Stop bit
	CLEAR_BIT(USART1->CR1, USART_CR1_WAKE); //Wake up idle Line
	CLEAR_BIT(USART1->CR1, USART_CR1_PCE); //Partity control disabled

	//Настройка прерываний
	CLEAR_BIT(USART1->CR1, USART_CR1_PEIE); //partity error interrupt disabled
	CLEAR_BIT(USART1->CR1, USART_CR1_TXEIE); //TXE interrupt is inhibited
	CLEAR_BIT(USART1->CR1, USART_CR1_TCIE); //Transmission complete interrupt disabled
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE); //Прерывание по приему данных включено
	SET_BIT(USART1->CR1, USART_CR1_IDLEIE); //Прерывание по флагу IDLE включено
	SET_BIT(USART1->CR1, USART_CR1_TE); //Transmitter is enabled
	SET_BIT(USART1->CR1, USART_CR1_RE); //Receiver is enabled and begins searching for a start bit
	CLEAR_BIT(USART1->CR1, USART_CR1_RWU);
	CLEAR_BIT(USART1->CR1, USART_CR1_SBK);
	USART1->CR2 = 0;
	USART1->CR3 = 0;
	USART1->GTPR = 0;
	NVIC_EnableIRQ(USART1_IRQn);
}


void USART_Transmit(USART_TypeDef *USART, uint8_t *data, uint16_t Size){
	for (uint16_t i = 0; i < Size; i++){
		while (READ_BIT(USART->SR, USART_SR_TXE) == 0);
		USART->DR = *data++;
	}
}


void SendTextUSART(const char* format, ...) {
    // Создаем переменную для аргументов
    va_list args;
    // Инициализируем список аргументов
    va_start(args, format);
    // Форматируем строку в буфер
    vsprintf((char*)husart1.tx_buffer, format, args);
    // Завершаем обработку списка аргументов
    va_end(args);

    // Отправляем текст через USART
    USART_Transmit(USART1, husart1.tx_buffer, strlen((char*)husart1.tx_buffer));
}

char ugetchar(void){
	 char ch = 0;

	 // Если есть данные в буфере
	 if (husart1.rx_counter > 0){
	     ch = husart1.rx_buffer[0]; // Берем первый символ из буфера

	     // Сдвигаем остальные данные в буфере
	     for (uint16_t i = 1; i < husart1.rx_counter; i++){
	        husart1.rx_buffer[i-1] = husart1.rx_buffer[i];
	     }
	      husart1.rx_counter--; // Уменьшаем счетчик
	 }
	    return ch;
}


void USART1_IRQHandler(void) {
	if (READ_BIT(USART1->SR, USART_SR_RXNE)) {
		//Если пришли данные по USART
		husart1.rx_buffer[husart1.rx_counter] = USART1->DR; //Считаем данные в соответствующую ячейку в rx_buffer
		husart1.rx_counter++; //Увеличим счетчик принятых байт на 1
	}
	if (READ_BIT(USART1->SR, USART_SR_IDLE)) {
		//Если прилетел флаг IDLE
		USART1->DR; //Сбросим флаг IDLE
		husart1.rx_len = husart1.rx_counter; //Узнаем, сколько байт получили
//		USART_Transmit(USART1, husart1.rx_buffer, husart1.rx_counter); //Отправим в порт то, что прилетело для проверки.
		husart1.rx_counter = 0; //сбросим счетчик приходящих данных
	}
}


void USART_Start_Menu(){
	SendTextUSART("\r\nДля начала тестирования нажмите y\n");
	while(ugetchar() != 'y');
}






