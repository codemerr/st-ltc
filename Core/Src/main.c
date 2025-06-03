#include "main.h"
#include "stm32f1xx_CMSIS.h"
#include "adc.h"
#include "relay.h"
#include "uart.h"
#include "ris.h"
#include "ds1720.h"
#include "turb.h"





char TestResultBff[kolv_test];
uint32_t i;
uint16_t freq = 10737;


const char	*ResultTest[kolv_rtest]={
	"Тестирование не проводилось",
	"Испр.",
	"Нет связи",
	"Не исправно",
	"Проводилось"
};



int main(void){

	uint16_t spi_tx_buffer[4] = {
			0x2100,                                    // Control: RESET + B28
			0x4000 | (freq & 0x00FF),                  // FREQ0 LSBs
			0x4000 | ((freq >> 14) & 0x3FFF),          // FREQ0 MSBs
			0x2028                                     // Control: OPBITEN, MODE, B28 (square wave, RESET=0)
	};

 	CMSIS_Debug_Init(); //Настройка SWD, запуск тактирование портов А, В
	CMSIS_RCC_SystemClock_72MHz(); //Системная частота 72Mhz
	CMSIS_SysTick_Timer_Init(); //Настройка системного таймера
	ADC_Riz_Turb_Relay_Init(); //Настройка ADC
	USART_Init(); //Настройка USART прием/передача 115200
	DS1720_Init(); //Настройка регистров для программирования DS1720
	RIS_Init();	//Настройка регистров для программирования DS1720
	DOUT_Init(); //Настройка регистров для дискретных выходов
	turb_Init();

	//PB4 - CS
    MODIFY_REG(GPIOB->CRL, GPIO_CRL_MODE4_Msk, 0b11 << GPIO_CRL_MODE4_Pos); // 50 MHz
    MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF4_Msk, 0b00 << GPIO_CRL_CNF4_Pos);	//GPIO Push-Pull
    TURB_CS_OFF;


	Dout_Pin_Switch(AC_POWER, true);





		TURB_CS_ON;
		CMSIS_SPI_Data_Transmit_16BIT(SPI1, spi_tx_buffer, 4, 100);
		TURB_CS_OFF;




	USART_Start_Menu();







	while (1){



		SendTextUSART("\r\nМеню тестирования:\r\n");
		for(i=0; i<kolv_test; i++){
			SendTextUSART("Тестирование %s - нажмите %c\r\n", NamePerif[i], i+'a');
		}


		do{
			i = ugetchar();
		} while ((i < 'a')||(i>= ('a' + kolv_test)));

		i -= 'a'; // Example: if i=a, else c(99) - a(97) = 2


		switch(i)
		{
//		case Full_test:
//		case FullWithoutPower_Test:
//		case Turb_Test:
		case Ris_Test: TestResultBff[i] = TestRiz(); break;
		case ProgDs1720_Test: TestResultBff[i] = ProgDs1720(); break;
//		case DS1720_Test:
		case Relay_Test: TestResultBff[i] = Start_Relay_Test();	break;
		case Counter_Test: TestResultBff[i] = Start_Counter_Test();	break;
//		case Power_Test:
//		case Report_Test:
		default : continue;
		}

		SendTextUSART("\r %s - %s \r\n", NamePerif[i], ResultTest[(int)TestResultBff[i]]);


//		Test_Led_Enable(500);



	}


}
