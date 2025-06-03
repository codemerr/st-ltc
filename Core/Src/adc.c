#include "adc.h"


volatile uint16_t ADC_Data[6] = {};
volatile uint8_t ADC_Conversion_Complete = 0;





void DMA1_Channel1_IRQHandler(void){
	if(READ_BIT(DMA1->ISR, DMA_ISR_TCIF1)){
		ADC_Conversion_Complete = 1;
		SET_BIT(DMA1->IFCR, DMA_IFCR_CGIF1); //Reset global flag
//		Counter_DMA++;
	} else if (READ_BIT(DMA1->ISR, DMA_ISR_TEIF1)){
		SET_BIT(DMA1->IFCR, DMA_IFCR_CGIF1);
	}
}




void ADC_Riz_Turb_Relay_Init(void) {
/*DMA section*/
	SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN); //Включение тактирования DMA1
	DMA1_Channel1->CPAR = (uint32_t)&(ADC1->DR); //Задаем адрес периферийного устройства
	DMA1_Channel1->CMAR = (uint32_t)ADC_Data; //Задаем адрес в памяти, куда будем кидать данные.
	DMA1_Channel1->CNDTR = 6; //Настроим количество данных для передачи. После каждого периферийного события это значение будет уменьшаться.
	MODIFY_REG(DMA1_Channel1->CCR, DMA_CCR_PL_Msk, 0b00 << DMA_CCR_PL_Pos); //Зададим приоритет канала на высокий
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_DIR); //Чтение будем осуществлять с периферии
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_CIRC); //Настроим DMA в Circular mode
	MODIFY_REG(DMA1_Channel1->CCR, DMA_CCR_PSIZE_Msk, 0b01 << DMA_CCR_PSIZE_Pos); //Размер данных периферийного устройства 16 бит
	MODIFY_REG(DMA1_Channel1->CCR, DMA_CCR_MSIZE_Msk, 0b01 << DMA_CCR_MSIZE_Pos); //Размер данных в памяти 16 бит
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_TCIE); //Включим прерывание по полной передаче
	CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_HTIE); //Отключим прерывание по половинной передаче
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_TEIE); //Включим прерывание по ошибке передачи.
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_MINC); //Включим инкрементирование памяти
	SET_BIT(DMA1_Channel1->CCR, DMA_CCR_EN); //DMA ON
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);



	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC1EN); //Запуск тактирования ADC1

	/*Настройка ножек PA3, PA4, PA5, PA6, PA7 PB1*/
	/*Pin PA3 - Analog*/
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF3_Msk, 0b00 << GPIO_CRL_CNF3_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE3_Msk, 0b00 << GPIO_CRL_MODE3_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF4_Msk, 0b00 << GPIO_CRL_CNF4_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE4_Msk, 0b00 << GPIO_CRL_MODE4_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF5_Msk, 0b00 << GPIO_CRL_CNF5_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE5_Msk, 0b00 << GPIO_CRL_MODE5_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF6_Msk, 0b00 << GPIO_CRL_CNF6_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE6_Msk, 0b00 << GPIO_CRL_MODE6_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF7_Msk, 0b00 << GPIO_CRL_CNF7_Pos);
    MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE7_Msk, 0b00 << GPIO_CRL_MODE7_Pos);
    MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF1_Msk, 0b00 << GPIO_CRL_CNF1_Pos);
    MODIFY_REG(GPIOB->CRL, GPIO_CRL_MODE1_Msk, 0b00 << GPIO_CRL_MODE1_Pos);


	/*Settings ADC control register*/
	CLEAR_BIT(ADC1->CR1, ADC_CR1_EOCIE); //EOC interrupt enable/disable.
	CLEAR_BIT(ADC1->CR1, ADC_CR1_AWDIE);
	CLEAR_BIT(ADC1->CR1, ADC_CR1_JEOCIE);
	SET_BIT(ADC1->CR1, ADC_CR1_SCAN); //Scan mode enable
	CLEAR_BIT(ADC1->CR1, ADC_CR1_AWDSGL); //Disable all watchdog channels
	CLEAR_BIT(ADC1->CR1, ADC_CR1_JAUTO);
	CLEAR_BIT(ADC1->CR1,ADC_CR1_DISCEN);
	CLEAR_BIT(ADC1->CR1, ADC_CR1_JDISCEN);
	MODIFY_REG(ADC1->CR1, ADC_CR1_DUALMOD_Msk, 0b0110 << ADC_CR1_DUALMOD_Pos); //Regular simultaneous mode only
	CLEAR_BIT(ADC1->CR1, ADC_CR1_JAWDEN); // Analog watchdog disable on injected channels
	CLEAR_BIT(ADC1->CR1, ADC_CR1_AWDEN); //Analog watchdog disable on regular channels

	/*Settings ADC control register 2*/
	SET_BIT(ADC1->CR2, ADC_CR2_ADON); //Enable ADC
	CLEAR_BIT(ADC1->CR2, ADC_CR2_CONT); //Disable continuous conversion mode (непрерывное преобразование)
	SET_BIT(ADC1->CR2, ADC_CR2_CAL); //Enable calibration
	while(READ_BIT(ADC1->CR2, ADC_CR2_CAL)); //Waiting for the end of calibration
//	Delay_ms(1);


	SET_BIT(ADC1->CR2, ADC_CR2_DMA); //Enable DMA
	CLEAR_BIT(ADC1->CR2, ADC_CR2_ALIGN); //Right alignment
	MODIFY_REG(ADC1->CR2, ADC_CR2_EXTSEL_Msk, 0b111 << ADC_CR2_EXTSEL_Pos);
	CLEAR_BIT(ADC1->CR2, ADC_CR2_EXTTRIG);
//	SET_BIT(ADC1->CR2, ADC_CR2_SWSTART); //Start conversion
//	SET_BIT(ADC1->CR2, ADC_CR2_TSVREFE); //Temperature sensor and VREFINT channel enable


	/*ADC sample time register 2*/
	MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP3_Msk, 0b111 << ADC_SMPR2_SMP3_Pos); // 239.5 cycles
	MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP4_Msk, 0b111 << ADC_SMPR2_SMP4_Pos); // 239.5 cycles
	MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP5_Msk, 0b111 << ADC_SMPR2_SMP5_Pos); // 239.5 cycles
	MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP6_Msk, 0b111 << ADC_SMPR2_SMP6_Pos); // 239.5 cycles
	MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP7_Msk, 0b111 << ADC_SMPR2_SMP7_Pos); // 239.5 cycles
	MODIFY_REG(ADC1->SMPR2, ADC_SMPR2_SMP9_Msk, 0b111 << ADC_SMPR2_SMP9_Pos); // 239.5 cycles PB1
//	MODIFY_REG(ADC1->SMPR1, ADC_SMPR1_SMP17_Msk, 0b111 << ADC_SMPR1_SMP17_Pos); // 239.5 cycles

	/*ADC regular sequence register 1 (ADC_SQR1)*/
	MODIFY_REG(ADC1->SQR1, ADC_SQR1_L_Msk, 0b0101 << ADC_SQR1_L_Pos); //6 преобразований

	/*ADC regular sequence regiser 3*/
	MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ1_Msk, 3 << ADC_SQR3_SQ1_Pos); //RELE_BPH
	MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ2_Msk, 4 << ADC_SQR3_SQ2_Pos); //RELE_TEN
	MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ3_Msk, 5 << ADC_SQR3_SQ3_Pos); //ADC_TURB_IN
	MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ4_Msk, 6 << ADC_SQR3_SQ4_Pos); //RIZ_X2_X1
	MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ5_Msk, 7 << ADC_SQR3_SQ5_Pos); //RIZ_50V
	MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ6_Msk, 9 << ADC_SQR3_SQ6_Pos); //RIZ_100V
//	MODIFY_REG(ADC1->SQR2, ADC_SQR2_SQ7_Msk, 17 << ADC_SQR2_SQ7_Pos); //Temperature MCU
	NVIC_EnableIRQ(ADC1_IRQn);
}

void ADC_StartConversion(void) {
    ADC_Conversion_Complete = 0;       // Сбрасываем флаг завершения

    // Сбрасываем флаги ADC (правильный способ)
    ADC1->SR = 0;  // Просто записываем 0 в регистр статуса для сброса всех флагов

    ADC_ResetDMA();                    // Переинициализируем DMA

    // Двойной вызов ADON для запуска (по спецификации STM32)
    SET_BIT(ADC1->CR2, ADC_CR2_ADON);
    SET_BIT(ADC1->CR2, ADC_CR2_ADON);

    SET_BIT(ADC1->CR2, ADC_CR2_SWSTART); // Старт преобразования
}

uint8_t ADC_IsConversionComplete(void) {

    return ADC_Conversion_Complete;
}

void ADC_ResetDMA(void) {
    CLEAR_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);  // Отключаем DMA
    DMA1_Channel1->CNDTR = 6;                   // Восстанавливаем счетчик
    SET_BIT(DMA1_Channel1->CCR, DMA_CCR_EN);    // Включаем DMA обратно
}

