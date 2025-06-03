#include "stm32f1xx_CMSIS.h"





void CMSIS_Debug_Init(void){
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN); //��������� ������������ ����� �
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN); //��������� ������������ ����� �
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN); //������ ������������ �������������� �������


	MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG, 0b010 << AFIO_MAPR_SWJ_CFG_Pos); //Serial wire

	/*���������� ��������� SW PA13, PA14*/
	GPIOA->LCKR = GPIO_LCKR_LCKK | GPIO_LCKR_LCK13 | GPIO_LCKR_LCK14;
	GPIOA->LCKR = GPIO_LCKR_LCK13 | GPIO_LCKR_LCK14;
	GPIOA->LCKR = GPIO_LCKR_LCKK | GPIO_LCKR_LCK13 | GPIO_LCKR_LCK14;
	GPIOA->LCKR;
}




/*
��������� �� STM32f103C8T6 �� ������� 72Mhz �� �������� ���������� ����������
������� ��������� ��������� �� 8 Mhz
*/
void CMSIS_RCC_SystemClock_72MHz(void) {
	SET_BIT(RCC->CR, RCC_CR_HSEON); //������ ����������� RC ���������� �� 8 Mhz
	while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0); // ������� �������� ����� � ����������
	SET_BIT(RCC->CR, RCC_CR_HSEON); //������ �������� ������ 8 Mhz
	while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0); // ������� �������� ����� � ����������
	CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP); //����� ��� �������
	SET_BIT(RCC->CR, RCC_CR_CSSON); //������ Clock detector
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL); //����� PLL � �������� Ssytem clock
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SWS, RCC_CFGR_SWS_PLL); //���������� PLL � �������� System clock
	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_DIV1); //AHB Prescaler /1

	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_2); //010 Two wait states, if 48 MHz < SYSCLK <= 72 Mhz
	SET_BIT(FLASH->ACR, FLASH_ACR_PRFTBE); //1: Prefetch is enabled
	SET_BIT(FLASH->ACR, FLASH_ACR_PRFTBS); //1: Prefetch buffer is enabled

	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV2); //ABP1 Prescaler /2, �.�. PCLR 1 max 36 Mhz
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV1); //APB2 Prescaler /1

	MODIFY_REG(RCC->CFGR, RCC_CFGR_ADCPRE, RCC_CFGR_ADCPRE_DIV6); // 72 / 6 = 12 Mhz (14 max);
	SET_BIT(RCC->CFGR, RCC_CFGR_PLLSRC); //� �������� �������� ������� ��� PLL �������� HSE
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_PLLXTPRE); //��� ����������� PLL
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PLLMULL, RCC_CFGR_PLLMULL9); // �.�. ����� �� 8 Mhz, 8*9 = 72 Mhz
	MODIFY_REG(RCC->CFGR, RCC_CFGR_MCO, RCC_CFGR_MCO_PLLCLK_DIV2);

	SET_BIT(RCC->CR, RCC_CR_PLLON); //������ PLL
	while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0); //������� ���������� ��������� PLL
}




/*��������� SysTick �� ������������*/
void CMSIS_SysTick_Timer_Init(void) {
	CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk); // ��������� �������
	SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk); //��������� ���������� �� �������
	SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk); //�������� ��� ������� �, 72 Mhz
	MODIFY_REG(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk, (72000-1) << SysTick_LOAD_RELOAD_Pos); //��������� �� 1��
	MODIFY_REG(SysTick->LOAD, SysTick_VAL_CURRENT_Msk, (72000-1) << SysTick_VAL_CURRENT_Pos); //������ ������� �� 71999 �� 0
	SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk); //������ ��������

	/*������������� PB9 (VD5-TEST) �� ����� � ������ Push-Pull*/
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_CNF9_Msk, 0b00 << GPIO_CRH_CNF9_Pos); //PB9 Output Push-pull
	MODIFY_REG(GPIOB->CRH, GPIO_CRH_MODE9_Msk, 0b10 << GPIO_CRH_MODE9_Pos); // 2Mhz speed
}




/*��������� Delay � ������ ������� HAL_GetTick()*/
volatile uint32_t SysTimer_ms = 0; // ������ HAL_GetTick();
volatile uint32_t Delay_counter_ms = 0; // ������� ��� ������� Delay_MS
volatile uint32_t Timeout_counter_ms = 0;



/*����� �������� � ��*/
void Delay_ms(uint32_t Milliseconds){
	Delay_counter_ms = Milliseconds;
	while (Delay_counter_ms != 0);
}




void Test_Led_Enable(uint32_t Time_Delay){
	uint32_t Led_Time_Ms = Time_Delay;
	WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR9);
	Delay_ms(Led_Time_Ms);
	WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS9);
	Delay_ms(Led_Time_Ms);
}




/*���������� �� ����� COUNTFLAG*/
void SysTick_Handler(void) {
	SysTimer_ms++;

	if(Delay_counter_ms){
		Delay_counter_ms--;
	}

	if(Timeout_counter_ms){
		Timeout_counter_ms--;
	}
}


















