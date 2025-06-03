#include "relay.h"
#include "uart.h"
#include "adc.h"





RelayStatus bph, ten;

uint16_t bph_adc_value;
uint16_t ten_adc_value;
uint32_t Counter = 0;
uint32_t current_time = 0;


// Функция для определения состояния реле по значению АЦП
RelayStatus GetRelayStatus(uint16_t adcValue) {
    if (adcValue == ADC_CLOSED_THRESHOLD) {
        return RELAY_STATUS_CLOSED;
    } else if (adcValue >= ADC_OPEN_MIN && adcValue <= ADC_OPEN_MAX) {
        return RELAY_STATUS_OPEN;
    } else if (adcValue > ADC_PARTIAL_MIN && adcValue < ADC_PARTIAL_MAX) {
        return RELAY_STATUS_PARTIALLY_CLOSED;
    } else{
        return ADC_STATUS_FAULT;	//неисправность АЦП
    }
}


void Dout_Pin_Switch(DoutPinCtrl pin, bool enable) {
    switch (pin) {
        case AC_POWER:
            WRITE_REG(GPIOB->BSRR, enable ? GPIO_BSRR_BS8 : GPIO_BSRR_BR8);
            break;
        case DC_POWER:
            WRITE_REG(GPIOA->BSRR, enable ? GPIO_BSRR_BS11 : GPIO_BSRR_BR11);
            break;
        case UMKA_HOT:
            WRITE_REG(GPIOA->BSRR, enable ? GPIO_BSRR_BS12 : GPIO_BSRR_BR12);
            break;
        case UMKA_RDY:
            WRITE_REG(GPIOB->BSRR, enable ? GPIO_BSRR_BS7 : GPIO_BSRR_BR7);
            break;
        case D_TERM:
            WRITE_REG(GPIOB->BSRR, enable ? GPIO_BSRR_BS6 : GPIO_BSRR_BR6);
            break;
        case TIME_CNT:
            WRITE_REG(GPIOB->BSRR, enable ? GPIO_BSRR_BS2 : GPIO_BSRR_BR2);
            break;
        default:
            break;
    }
}



void TIM2_IRQHandler(void){
	if (READ_BIT(TIM2->SR, TIM_SR_UIF)){
		CLEAR_BIT(TIM2->SR, TIM_SR_UIF); //Сброс фалага перывания
	}
	Counter++;
}



void DOUT_Init(){
	//ON_TIME_EN init PB2
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF2_Msk, 0b00 << GPIO_CRL_CNF2_Pos); //PB2 Output Push-pull
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_MODE2_Msk, 0b10 << GPIO_CRL_MODE2_Pos); // 2Mhz speed

	//D_TERM_EN init PB6
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF6_Msk, 0b00 << GPIO_CRL_CNF6_Pos); //PB6 Output Push-pull
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_MODE6_Msk, 0b10 << GPIO_CRL_MODE6_Pos); // 2Mhz speed

	//UMKA_RDY init PB7
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF7_Msk, 0b00 << GPIO_CRL_CNF7_Pos); //PA12 Output Push-pull
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_MODE7_Msk, 0b10 << GPIO_CRL_MODE7_Pos); // 2Mhz speed

	//AC_V_EN init PB8
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_CNF8_Msk, 0b00 << GPIO_CRH_CNF8_Pos); //PB8 Output Push-pull
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE8_Msk, 0b10 << GPIO_CRH_MODE8_Pos); // 2Mhz speed

	//LBP_EN init PA11
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF11_Msk, 0b00 << GPIO_CRH_CNF11_Pos); //PA11 Output Push-pull
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE11_Msk, 0b10 << GPIO_CRH_MODE11_Pos); // 2Mhz speed

	//UMKA_HOT init PA12
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF12_Msk, 0b00 << GPIO_CRH_CNF12_Pos); //PA12 Output Push-pull
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE12_Msk, 0b10 << GPIO_CRH_MODE12_Pos); // 2Mhz speed




	//Запуск тактирования таймера
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);

	//COUNTER_EN init PA0 for PWM mode
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF0_Msk, 0b10 << GPIO_CRL_CNF0_Pos); //AF Push-Pull
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE0_Msk, 0b11 << GPIO_CRL_MODE0_Pos); //50Mhz speed

	//Настройка таймера 2
	CLEAR_BIT(TIM2->CR1, TIM_CR1_UDIS); //Генерация события Update
	CLEAR_BIT(TIM2->CR1, TIM_CR1_URS); //Генерация прерывания
	CLEAR_BIT(TIM2->CR1, TIM_CR1_OPM); //One pulse mode off(Остановка счетчика при обновлении)
	CLEAR_BIT(TIM2->CR1, TIM_CR1_DIR); //Счет вверх
	MODIFY_REG(TIM2->CR1, TIM_CR1_CMS_Msk, 0b00 << TIM_CR1_CMS_Pos); //Выравнивание по краю
	SET_BIT(TIM2->CR1, TIM_CR1_ARPE); // Auto-reload preload enable
	MODIFY_REG(TIM2->CR1, TIM_CR1_CKD_Msk, 0b00 << TIM_CR1_CKD_Pos); //Без делителя


	//Настройка шим
	MODIFY_REG(TIM2->CCMR1, TIM_CCMR1_CC1S_Msk, 0b00 << TIM_CCMR1_CC1S_Pos);
	CLEAR_BIT(TIM2->CCMR1, TIM_CCMR1_OC1FE); //Fast mode disable
	SET_BIT(TIM2->CCMR1, TIM_CCMR1_OC1PE); // Preload enable
	MODIFY_REG(TIM2->CCMR1, TIM_CCMR1_OC1M_Msk, 0b110 << TIM_CCMR1_OC1M_Pos); // PWM MODE 1
	CLEAR_BIT(TIM2->CCMR1, TIM_CCMR1_OC1CE);

	//Настройка прерывания
	SET_BIT(TIM2->DIER, TIM_DIER_UIE); //Включение обновления прерывания
	NVIC_EnableIRQ(TIM2_IRQn); //Разрешить прерывание по таймеру 2


	TIM2->PSC = 71;
	TIM2->ARR = 124;
	TIM2->CCR1 = 62;



}

int Start_Counter_Test(){
	SendTextUSART("\r\nТестирование счетчика имульсов");
	Delay_ms(2000);
	current_time = 0;
	uint32_t start_time = SysTimer_ms;

	SET_BIT(TIM2->EGR, TIM_EGR_UG);  // Обновление регистров
	SET_BIT(TIM2->CR1, TIM_CR1_CEN); //Запуск таймера


	Dout_Pin_Switch(DC_POWER, true);
	Delay_ms(20);
	//Запуск ШИМ
	SET_BIT(TIM2->CCER, TIM_CCER_CC1E);

	while (1){
		current_time = SysTimer_ms;

		ADC_StartConversion();
		while(!ADC_IsConversionComplete()) {

		}
		bph_adc_value = ADC_Data[0];
		Process_Status_Relay((uint16_t *) ADC_Data, &bph, &ten);

		if (bph == RELAY_STATUS_CLOSED){
			 uint32_t elapsed_time = current_time - start_time;

			 // Условие 1: меньше 15 секунд — ошибка
			 if (elapsed_time < 14000) {
				 SendTextUSART("\r\nНеисправность счетчика: реле BPH замкнулось на %u сек.: требуется 14-18сек",elapsed_time / 1000);
				 Reset_State();
				 return notValid_rtest;
			 }
			 // Условие 2: в пределах 15-17 секунд — успех
			 else if (elapsed_time <= 18000) {
				 SendTextUSART("\r\nРеле BPH замкнулось через %u сек.",elapsed_time / 1000);
				 Reset_State();
				 return FuncOk_rtest;
			 }
			 // Условие 3: больше 17 секунд — ошибка
			 else {
				 SendTextUSART("\r\nНеисправность счетчика: реле BPH замкнулось на %u сек.: требуется 14-18сек",elapsed_time / 1000);
				 Reset_State();
				 return notValid_rtest;
			 }
			 break;
		}

		if ((current_time - start_time) > 20000) {
			SendTextUSART("\r\nСчетчик превысил 20 сек. реле BPH не замкнуто");
			Reset_State();
		    break;
		}
	}
	Reset_State();
	return notValid_rtest;
}


// Функция для обработки массива АЦП и получения состояния для элементов [2] и [3]
void Process_Status_Relay(uint16_t *adcValues, RelayStatus *bph, RelayStatus *ten) {

    // Проверка состояния для элемента [2]
    *bph = GetRelayStatus(adcValues[0]);

    // Проверка состояния для элемента [3]
    *ten = GetRelayStatus(adcValues[1]);
}

void Reset_State(void){
	//Отключение блока
	Dout_Pin_Switch(DC_POWER, false);
	Dout_Pin_Switch(UMKA_RDY, false);
	Dout_Pin_Switch(D_TERM, false);
	Dout_Pin_Switch(TIME_CNT, false);
	//Отключение шим
	CLEAR_BIT(TIM2->CCER, TIM_CCER_CC1E);
	CLEAR_BIT(TIM2->CR1, TIM_CR1_CEN);
	SET_BIT(TIM2->EGR, TIM_EGR_UG);

	Delay_ms(3000);
}


int Start_Relay_Test(){
	//Проверка реле, подача питания, включение ON_TIME: Тен замкнут, bph разомкнут
	SendTextUSART("\r\nТестирование реле");
	Dout_Pin_Switch(TIME_CNT, true);
	Dout_Pin_Switch(DC_POWER, true);
//	Dout_Pin_Switch(UMKA_RDY, true);
	Delay_ms(500);
//	Dout_Pin_Switch(D_TERM, true);

	ADC_StartConversion();
	while(!ADC_IsConversionComplete()) {

	}

	bph_adc_value = ADC_Data[0];
	ten_adc_value = ADC_Data[1];


	Process_Status_Relay((uint16_t *) ADC_Data, &bph, &ten);

	if(bph != RELAY_STATUS_OPEN || ten != RELAY_STATUS_CLOSED){
		SendTextUSART("\r\nОшибка после подачи питания, требуется: ТЕН-замкнут, BPH-разомкнут");

		if (bph == RELAY_STATUS_CLOSED) {
			SendTextUSART("\r\nНеисправность: реле BPH замкнуто");
			SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 2600-2800",bph_adc_value);
		} else if (bph == RELAY_STATUS_PARTIALLY_CLOSED) {
			SendTextUSART("\r\nНеисправность: залипание реле BPH");
			SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 2600-2800",bph_adc_value);
		} else if (bph == ADC_STATUS_FAULT){
			SendTextUSART("\r\nНеисправность АЦП: реле BPH");
			SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 2600-2800",bph_adc_value);
		}

		if (ten == RELAY_STATUS_OPEN) {
			SendTextUSART("\r\nНеисправность: реле тена разомкнуто");
			SendTextUSART("\r\nЗначение АЦП тена = %u, требуется: 4095",ten_adc_value);
		} else if (ten == RELAY_STATUS_PARTIALLY_CLOSED) {
			SendTextUSART("\r\nНеисправность: залипание реле тена");
			SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 4095",ten_adc_value);
		} else if (ten == ADC_STATUS_FAULT){
			SendTextUSART("\r\nНеисправность АЦП: реле тена");
			SendTextUSART("\r\nЗначение АЦП ТЕН = %u, требуется: 4095",ten_adc_value);
		}
		Reset_State();
		return notValid_rtest;
	}



	//Влючение UMKA_RDY, проверка состояния: Тен замкнут, bph замкнут
	Dout_Pin_Switch(UMKA_RDY, true);
	Delay_ms(300);

	ADC_StartConversion();
	while(!ADC_IsConversionComplete()) {

	}
	bph_adc_value = ADC_Data[0];
	ten_adc_value = ADC_Data[1];
	Process_Status_Relay((uint16_t *) ADC_Data, &bph, &ten);


	if(bph != RELAY_STATUS_CLOSED || ten != RELAY_STATUS_CLOSED){
				SendTextUSART("\r\nОшибка после включения READY(XT4), требуется: ТЕН-замкнут, BPH-замкнут");

				if (bph == RELAY_STATUS_OPEN) {
					SendTextUSART("\r\nНеисправность: реле BPH разомкнуто");
					SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 4095",bph_adc_value);
				} else if (bph == RELAY_STATUS_PARTIALLY_CLOSED) {
					SendTextUSART("\r\nНеисправность: залипание реле BPH");
					SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 4095",bph_adc_value);
				} else if (bph == ADC_STATUS_FAULT){
					SendTextUSART("\r\nНеисправность АЦП: реле BPH, проверьте стенд");
					SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 4095",bph_adc_value);
				}

				if (ten == RELAY_STATUS_CLOSED) {
					SendTextUSART("\r\nНеисправность: реле тена разомкнут");
					SendTextUSART("\r\nЗначение АЦП тена = %u, требуется: 4095",ten_adc_value);
				} else if (ten == RELAY_STATUS_PARTIALLY_CLOSED) {
					SendTextUSART("\r\nНеисправность: залипание реле тена");
					SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 4095",ten_adc_value);
				} else if (ten == ADC_STATUS_FAULT){
					SendTextUSART("\r\nНеисправность АЦП: реле тена");
					SendTextUSART("\r\nЗначение АЦП ТЕН = %u, требуется: 4095",ten_adc_value);
				}
				Reset_State();
				return notValid_rtest;
			}




	//Влючение D_TERM, проверка состояния: Тен разомкнут, bph замкнут
	Dout_Pin_Switch(D_TERM, true);
	Delay_ms(300);

	ADC_StartConversion();
	while(!ADC_IsConversionComplete()) {

	}
	bph_adc_value = ADC_Data[0];
	ten_adc_value = ADC_Data[1];
	Process_Status_Relay((uint16_t *) ADC_Data, &bph, &ten);

	if(bph != RELAY_STATUS_CLOSED || ten != RELAY_STATUS_OPEN){
			SendTextUSART("\r\nОшибка после включения D_TERM(XT7), требуется: ТЕН-разомкнут, BPH-замкнут");

			if (bph == RELAY_STATUS_OPEN) {
				SendTextUSART("\r\nНеисправность: реле BPH разомкнуто");
				SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 4095",bph_adc_value);
			} else if (bph == RELAY_STATUS_PARTIALLY_CLOSED) {
				SendTextUSART("\r\nНеисправность: залипание реле BPH");
				SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 4095",bph_adc_value);
			} else if (bph == ADC_STATUS_FAULT){
				SendTextUSART("\r\nНеисправность АЦП: реле BPH");
				SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 4095",bph_adc_value);
			}

			if (ten == RELAY_STATUS_CLOSED) {
				SendTextUSART("\r\nНеисправность: реле тена замкнуто");
				SendTextUSART("\r\nЗначение АЦП тена = %u, требуется: 2600-2800",ten_adc_value);
			} else if (ten == RELAY_STATUS_PARTIALLY_CLOSED) {
				SendTextUSART("\r\nНеисправность: залипание реле тена");
				SendTextUSART("\r\nЗначение АЦП BPH = %u, требуется: 2600-2800",ten_adc_value);
			} else if (ten == ADC_STATUS_FAULT) {
				SendTextUSART("\r\nНеисправность АЦП: реле тена");
				SendTextUSART("\r\nЗначение АЦП ТЕН = %u, требуется: 2600-2800",ten_adc_value);
			}
			Reset_State();
			return notValid_rtest;
		}



	//Отключение блока
	Reset_State();





	return FuncOk_rtest;
}






