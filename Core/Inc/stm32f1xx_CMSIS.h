#ifndef INC_STM32F1XX_CMSIS_H_
#define INC_STM32F1XX_CMSIS_H_





#include <main.h>




void CMSIS_Debug_Init(void); //Настройка Debug Serial Wire
void CMSIS_RCC_SystemClock_72MHz(void); //Настройка тактирования МК на 72Mhz
void CMSIS_SysTick_Timer_Init(void); //Инициализация системного таймера
void Delay_ms(uint32_t Milliseconds); //Функция задержки
void SysTick_Handler(void); //Прерывание от системного таймера
void CMSIS_PB9_OUTPUT_Push_Pull_Init(void); //Настройка TEST светодиода на выход в режим Push-Pull
void Test_Led_Enable();

extern volatile uint32_t SysTimer_ms;
extern volatile uint32_t Timeout_counter_ms; //Переменная для таймаута функций

#endif /* INC_STM32F1XX_CMSIS_H_ */
