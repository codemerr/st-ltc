#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include "stm32f1xx.h"
#include "stm32f1xx_CMSIS.h"





typedef enum {
    AC_POWER,   // PB8
    DC_POWER,   // PA11
    UMKA_HOT,   // PA12
    UMKA_RDY,   // PB7
    D_TERM,     // PB6
	TIME_CNT	// PB2
} DoutPinCtrl;


// ����������� ��������
typedef enum {
    RELAY_STATUS_CLOSED,           // ���� ��������
    RELAY_STATUS_OPEN,             // ���� ����������
    RELAY_STATUS_PARTIALLY_CLOSED, // ���� ������� �� �� �����
    ADC_STATUS_FAULT               // ������������� ���
} RelayStatus;





void DOUT_Init();
void Dout_Pin_Switch(DoutPinCtrl pin, bool enable);

int Start_Relay_Test();
void PrintRelayStatus(RelayStatus status);
void Process_Status_Relay(uint16_t *adcValues, RelayStatus *bph, RelayStatus *ten);
void Reset_State(void);
int Start_Counter_Test();





#ifndef INC_RELAY_H_
#define INC_RELAY_H_



#endif /* INC_RELAY_H_ */
