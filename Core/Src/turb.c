#include "turb.h"
#include "uart.h"
#include "adc.h"





void turb_Init(void){
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN); //Включени тактирования SPI1
	SET_BIT(AFIO->MAPR, AFIO_MAPR_SPI1_REMAP); // Remap SPI1 из PA5-PB7 на PB3-PB5
	//MK  - AD9833
	//PB3 - FSYNC (CS)
	//PB4 - SCLK
	//PB5 - SDATA (MISO)

	//PB5 - MOSI
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_MODE5, 0b11 << GPIO_CRL_MODE5_Pos); //Maximum output speed 50 MHz
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF5, 0b10 << GPIO_CRL_CNF5_Pos); 	// Alternate func. output Push-pull

	//PB3 - SCLK
    MODIFY_REG(GPIOB->CRL, GPIO_CRL_MODE3_Msk, 0b11 << GPIO_CRL_MODE3_Pos); // 50 MHz
    MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF3_Msk, 0b10 << GPIO_CRL_CNF3_Pos); 	//Alternate func. output Push-pull





    //SPI Settings
    MODIFY_REG(SPI1->CR1, SPI_CR1_BR, 0b011 << SPI_CR1_BR_Pos); //fPCLK/16. 72000000 / 16
    SET_BIT(SPI1->CR1, SPI_CR1_CPOL); // Полярность
    CLEAR_BIT(SPI1->CR1, SPI_CR1_CPHA); // Фаза
    SET_BIT(SPI1->CR1, SPI_CR1_DFF);
    CLEAR_BIT(SPI1->CR1, SPI_CR1_LSBFIRST);
    SET_BIT(SPI1->CR1, SPI_CR1_SSM);
    SET_BIT(SPI1->CR1, SPI_CR1_SSI);
    SET_BIT(SPI1->CR1, SPI_CR1_MSTR);
    CLEAR_BIT(SPI1->CR1, SPI_CR1_BIDIMODE);
    CLEAR_BIT(SPI1->CR1, SPI_CR1_RXONLY);
    CLEAR_BIT(SPI1->CR1, SPI_CR1_CRCEN);
    CLEAR_BIT(SPI1->CR1, SPI_CR1_CRCNEXT);
    SET_BIT(SPI1->CR1, SPI_CR1_SPE); // Включить SPI

}


bool CMSIS_SPI_Data_Transmit_16BIT(SPI_TypeDef* SPI, uint16_t* data, uint16_t Size_data, uint32_t Timeout_ms) {
	//(см. Reference Manual стр. 712 Transmit-only procedure (BIDIMODE=0 RXONLY=0))
	if (!READ_BIT(SPI->SR, SPI_SR_BSY)) {
		//Проверим занятость шины
		SPI->DR = *(data); //Запишем первый элемент данных для отправки в регистр SPI_DR
		//(При этом очищается бит TXE)

		for (uint16_t i = 1; i < Size_data; i++) {
			Timeout_counter_ms = Timeout_ms;
			while (!READ_BIT(SPI->SR, SPI_SR_TXE)) {
				//Ждем, пока буфер на передачу не освободится
				if (!Timeout_counter_ms) {
					return false;
				}
			}
			SPI->DR = *(data + i); //Запишем следующий элемент данных.
		}
		Timeout_counter_ms = Timeout_ms;
		while (!READ_BIT(SPI->SR, SPI_SR_TXE)) {
			//После записи последнего элемента данных в регистр SPI_DR,
			//подождем, пока TXE станет равным 1.
			if (!Timeout_counter_ms) {
				return false;
			}
		}
		Timeout_counter_ms = Timeout_ms;
		while (READ_BIT(SPI->SR, SPI_SR_BSY)) {
			//Затем подождем, пока BSY станет равным 0.
			//Это указывает на то, что передача последних данных завершена.
			if (!Timeout_counter_ms) {
				return false;
			}
		}
		return true;
	} else {
		return false;
	}
	//Примечание:
	//После передачи двух элементов данных в режиме "transmit-only mode" в регистре SPI_SR устанавливается флаг OVR, так как принятые данные никогда не считываются.
}

