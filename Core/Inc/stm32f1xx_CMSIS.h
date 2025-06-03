#ifndef INC_STM32F1XX_CMSIS_H_
#define INC_STM32F1XX_CMSIS_H_





#include <main.h>




void CMSIS_Debug_Init(void); //��������� Debug Serial Wire
void CMSIS_RCC_SystemClock_72MHz(void); //��������� ������������ �� �� 72Mhz
void CMSIS_SysTick_Timer_Init(void); //������������� ���������� �������
void Delay_ms(uint32_t Milliseconds); //������� ��������
void SysTick_Handler(void); //���������� �� ���������� �������
void CMSIS_PB9_OUTPUT_Push_Pull_Init(void); //��������� TEST ���������� �� ����� � ����� Push-Pull
void Test_Led_Enable();

extern volatile uint32_t SysTimer_ms;
extern volatile uint32_t Timeout_counter_ms; //���������� ��� �������� �������

#endif /* INC_STM32F1XX_CMSIS_H_ */
