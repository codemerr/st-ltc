#include "uart.h"
#include "ris.h"





volatile uint16_t ADC_Riz_50V_Value = 0; //ADC0
volatile float ADC_Riz_50V_Vin = 0;

//volatile uint16_t ADC_Riz_100V_Value = 0; //ADC1
//volatile float ADC_Riz_100V_Vin = 0;

volatile uint16_t ADC_Riz_X2_X1_Value = 0; //ADC3
volatile float ADC_Riz_X2_X1_100Vin = 0;
volatile float ADC_Riz_X2_X1_90Vin = 0;





//float convert_adc_to_turb(uint16_t adc_value) {
//	float Vin = (adc_value * VREF) / ADC_MAX; //// Пересчет АЦП в напряжение
//	float Vturb = ((10000 + 1000) / 1000) * Vin;
//	return Vturb;
//}
//float convert_adc_to_100v(uint16_t adc_value) {
//	float Vin = (adc_value * VREF) / ADC_MAX;
//	float riz100v = ((100000 + 2400) / 2400) * Vin;
//	return riz100v;
//}
float convert_adc_to_50v(uint16_t adc_value) {
    const float R1 = 680000.0f;  // 100k резистор
    const float R2 = 16200.0f;    // 2.4k резистор
    const float divider_ratio = R1 / R2;  // Предварительно вычисляем коэффициент

	float Vin = (adc_value * VREF) / ADC_MAX;
	float riz50v = divider_ratio * Vin;
	return riz50v;
}
float convert_adc_to_x1_x2(uint16_t adc_value) {
    const float R1 = 100000.0f;  // 100k резистор
    const float R2 = 2400.0f;    // 2.4k резистор
    const float divider_ratio = R1 / R2;  // Предварительно вычисляем коэффициент

    float Vin = (adc_value * VREF) / ADC_MAX;
    float rizx1x2 = divider_ratio * Vin;
    return rizx1x2;
}

void RIS_Init(void) {
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF0_Msk, 0b00 << GPIO_CRL_CNF0_Pos); //PB0 Output Push-pull
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_MODE0_Msk, 0b11 << GPIO_CRL_MODE0_Pos); // 50Mhz speed
}


int TestRiz(void){
ADC_StartConversion();  // Запускаем преобразование

// Ждем завершения преобразования
while(!ADC_IsConversionComplete()) {
    // Можно добавить небольшую задержку или выполнять другие задачи
}


ADC_Riz_X2_X1_Value = ADC_Data[3];
ADC_Riz_X2_X1_90Vin = convert_adc_to_x1_x2(ADC_Riz_X2_X1_Value);
int voltage_x90 = (int)(ADC_Riz_X2_X1_90Vin * 100);





WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS0); // Relay K1 on
Delay_ms(500);

ADC_StartConversion();  // Запускаем преобразование
// Ждем завершения преобразования
while(!ADC_IsConversionComplete()) {
    // Можно добавить небольшую задержку или выполнять другие задачи
}

ADC_Riz_50V_Value = ADC_Data[4];
ADC_Riz_50V_Vin = convert_adc_to_50v(ADC_Riz_50V_Value);
int voltage_x50 = (int)(ADC_Riz_50V_Vin * 100);

ADC_Riz_X2_X1_Value = ADC_Data[3];
ADC_Riz_X2_X1_100Vin = convert_adc_to_x1_x2(ADC_Riz_X2_X1_Value);
int voltage_x100 = (int)(ADC_Riz_X2_X1_100Vin * 100);


SendTextUSART("Значение RIS-U требуется 50V факт.: %d.%02d V\r\n",voltage_x50 / 100, voltage_x50 % 100);
SendTextUSART("Значение RIS-I требуется 90V факт.: %d.%02d V\r\n",voltage_x90 / 100, voltage_x90 % 100);
SendTextUSART("Значение RIS-I требуется 100V факт.: %d.%02d V\r\n", voltage_x100 / 100, voltage_x100 % 100);


WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR0);// Relay K1 off
Delay_ms(300);




if((ADC_Riz_50V_Vin < 45) || (ADC_Riz_50V_Vin > 55)  ||
(ADC_Riz_X2_X1_90Vin < 81) || (ADC_Riz_X2_X1_90Vin > 99) ||
(ADC_Riz_X2_X1_100Vin < 90) || (ADC_Riz_X2_X1_100Vin > 110))  return notValid_rtest;




return FuncOk_rtest;
}








