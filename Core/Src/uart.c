#include "uart.h"



const char* NamePerif[kolv_test]={
		"��� ����������	  ", 	  //a
		"���������� ��������    ",//b
		"����. ��������	  ", 	  //c
		"�����. DS1720	  ", 	  //d
		"DS1720		  ",		  //e
		"����		  ", 		  //f
		"������� ���������	  ",  //g
		"�������		  ", 	  //h
		"�����		  ", 		  //i
		"��� ����. ����� �������" //j
};



struct USART_name husart1;



void USART_Init(void){
	//Tx - Alternative Function output Push-pull(Maximum output speed 50 MHz)
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF9_Msk, 0b10 << GPIO_CRH_CNF9_Pos);
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE9_Msk, 0b11 << GPIO_CRH_MODE9_Pos);

	//Rx - Input floating
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF10_Msk, 0b1 << GPIO_CRH_CNF10_Pos);
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE10_Msk, 0b00 << GPIO_CRH_MODE10_Pos);

	//������ ������������ USART1
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

//	�������� 115200:
//	115200 = 72000000/(16*USARTDIV)
//	����� USARTDIV = 72000000/115200*16 = 39.0625
//	DIV_Mantissa � ������ ������ ����� 39, ��� ���� 0x27
//	DIV_Fraction �����, ��� 0.0625*16 = 1, ��� ���� 0x1
	MODIFY_REG(USART1->BRR, USART_BRR_DIV_Mantissa_Msk, 0x27 << USART_BRR_DIV_Mantissa_Pos);
	MODIFY_REG(USART1->BRR, USART_BRR_DIV_Fraction_Msk, 0x1 << USART_BRR_DIV_Fraction_Pos);

	SET_BIT(USART1->CR1, USART_CR1_UE); //USART enable
	CLEAR_BIT(USART1->CR1, USART_CR1_M); //Word lenght 1 Start bit, 8 Data bits, n Stop bit
	CLEAR_BIT(USART1->CR1, USART_CR1_WAKE); //Wake up idle Line
	CLEAR_BIT(USART1->CR1, USART_CR1_PCE); //Partity control disabled

	//��������� ����������
	CLEAR_BIT(USART1->CR1, USART_CR1_PEIE); //partity error interrupt disabled
	CLEAR_BIT(USART1->CR1, USART_CR1_TXEIE); //TXE interrupt is inhibited
	CLEAR_BIT(USART1->CR1, USART_CR1_TCIE); //Transmission complete interrupt disabled
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE); //���������� �� ������ ������ ��������
	SET_BIT(USART1->CR1, USART_CR1_IDLEIE); //���������� �� ����� IDLE ��������
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
    // ������� ���������� ��� ����������
    va_list args;
    // �������������� ������ ����������
    va_start(args, format);
    // ����������� ������ � �����
    vsprintf((char*)husart1.tx_buffer, format, args);
    // ��������� ��������� ������ ����������
    va_end(args);

    // ���������� ����� ����� USART
    USART_Transmit(USART1, husart1.tx_buffer, strlen((char*)husart1.tx_buffer));
}

char ugetchar(void){
	 char ch = 0;

	 // ���� ���� ������ � ������
	 if (husart1.rx_counter > 0){
	     ch = husart1.rx_buffer[0]; // ����� ������ ������ �� ������

	     // �������� ��������� ������ � ������
	     for (uint16_t i = 1; i < husart1.rx_counter; i++){
	        husart1.rx_buffer[i-1] = husart1.rx_buffer[i];
	     }
	      husart1.rx_counter--; // ��������� �������
	 }
	    return ch;
}


void USART1_IRQHandler(void) {
	if (READ_BIT(USART1->SR, USART_SR_RXNE)) {
		//���� ������ ������ �� USART
		husart1.rx_buffer[husart1.rx_counter] = USART1->DR; //������� ������ � ��������������� ������ � rx_buffer
		husart1.rx_counter++; //�������� ������� �������� ���� �� 1
	}
	if (READ_BIT(USART1->SR, USART_SR_IDLE)) {
		//���� �������� ���� IDLE
		USART1->DR; //������� ���� IDLE
		husart1.rx_len = husart1.rx_counter; //������, ������� ���� ��������
//		USART_Transmit(USART1, husart1.rx_buffer, husart1.rx_counter); //�������� � ���� ��, ��� ��������� ��� ��������.
		husart1.rx_counter = 0; //������� ������� ���������� ������
	}
}


void USART_Start_Menu(){
	SendTextUSART("\r\n��� ������ ������������ ������� y\n");
	while(ugetchar() != 'y');
}






