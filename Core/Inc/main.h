#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif



#include <stdio.h>
#include <string.h>
#include "stm32f1xx.h"




// ADC macros
#define VREF 3.285f
#define ADC_MAX 4095.0f
#define V25 1.43f
#define AVG_SLOPE 0.0043f
#define kolv_test 10			  // Кол-во тестов всегов программе
#define ADC_MAX_VALUE 4095        // Максимальное значение АЦП (12-битное)

// Значение для теста реле
#define ADC_CLOSED_THRESHOLD 4095 // Значение, при котором реле замкнуто
#define ADC_OPEN_MIN 2600         // Минимальное значение для разомкнутого состояния
#define ADC_OPEN_MAX 2800         // Максимальное значение для разомкнутого состояния
#define ADC_PARTIAL_MIN 2801      // Минимальное значение для частичного замыкания
#define ADC_PARTIAL_MAX 4094      // Максимальное значение для частичного замыкания
#define ADC_FAULT_THRESHOLD 2600  // Порог неисправности АЦП

// Определение последовательности тестирования периферии
#define Full_test 0
#define Turb_Test 1
#define Ris_Test 2
#define ProgDs1720_Test 3
#define DS1720_Test 4
#define Relay_Test 5
#define Counter_Test 6
#define Power_Test 7
#define Report_Test 8
#define FullWithoutPower_Test 9

// Статуст результата тестирования
#define	notTest_rtest		0 	  // Тест не проводился
#define	FuncOk_rtest		1 	  // Испр.
#define	notConnect_rtest	2 	  // Нет связи
#define	notValid_rtest		3 	  // Не исправно
#define	Test_rtest			4 	  // Проводилось


#define	kolv_rtest		5











#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
