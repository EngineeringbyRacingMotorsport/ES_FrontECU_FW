/*
 * p2f.c
 *
 *  Created on: Mar 27, 2026
 *      Author: Guillem Vílchez
 */
#include "p2f.h"

void R2D(void){
uint8_t SDC = 0; // Estat de l'entrada SDC (Safety Disconnect / Shutdown circuit)
uint8_t EstatAirpositiu  = false;// Estat del sensor d'aire positiu (pressió OK)
uint8_t Button_R2DX = false; // Estat del botó Ready To Drive
uint8_t state_r2d = 0;
uint16_t Brake_SensorX = 0;// Valor analògic del sensor de fre (lectura ADC)
uint32_t temp_R2D = 0;// Temps (en ms) en què entrem a STEP2

// Llegir l'entrada digital SDC (GPIOB pin 6): HAL_GPIO_ReadPin retorna GPIO_PIN_SET o GPIO_PIN_RESET
	SDC = HAL_GPIO_ReadPin(GPIOB, SDC_Pin);
	// Llegir l'entrada de l'air positiu (GPIOB pin 7)
	EstatAirpositiu = HAL_GPIO_ReadPin(GPIOB, EstatAir_Pin);
	// Llegir l'estat del botó Ready To Drive (GPIOB pin 9)
	Button_R2DX = HAL_GPIO_ReadPin(GPIOB, Button_R2DX_Pin);


	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	Brake_SensorX = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	// Màquina d'estats finits per la seqüència Ready To Drive
	switch (state_r2d)
	{

		// Condicions per passar d'inicial a STEP1: fre premut (Brake_SensorX >= 300), SDC actiu (SDC != 0), botó R2D premut (Button_R2DX != 0),
		//Aire positiu OK (EstatAirpositiu != 0) i sense error d'APPS (!error_apps)
		case 0:
			if (Brake_SensorX >= 300 && SDC && Button_R2DX && EstatAirpositiu)
				state_r2d = 1;
			break;

		// En STEP1 exigim: Fre continua premut, SDC actiu, botó alliberat (!Button_R2DX), air positiu OK i sense error APPS
		case 1:
			if (Brake_SensorX >= 300 && SDC && !Button_R2DX && EstatAirpositiu)
			{
				temp_R2D = HAL_GetTick();// HAL_GetTick() dona el nombre de mil·lisegons des de HAL_Init() (és un contador global de temps del sistema incrementat per l'interrupt de SysTick).
				state_r2d = 2;// Passem a STEP2 (finestra d'espera de 2 segons)
			}
			else if (SDC == false || EstatAirpositiu == false) // Si es perd SDC, air positiu o hi ha error d'APPS, tornem a l'estat inicial
				state_r2d = 0;
			break;

		// En STEP2: SDC actiu, botó segueix alliberat, air positiu OK, sense error APPS i han passat com a mínim 2000 ms (2 s) des de temp_R2D
		case 2:
			if (SDC && !Button_R2DX && EstatAirpositiu && (HAL_GetTick() - temp_R2D) >= 2000)// Aquí, determinem el temps que s'ha avançat respecte el punt inicial
				state_r2d = 3;// Si es compleix tot això, passem a STEP3
			else if (SDC == false || EstatAirpositiu== false) // Qualsevol pèrdua de SDC, air positiu o error APPS ens fa tornar a inicial
				state_r2d = 0;
			break;

		// En STEP3 tornem a demanar: fre premut, SDC actiu, botó R2D premut, air positiu OK i sense error APPS
		case 3:
			if (Brake_SensorX >= 300 && SDC == true && Button_R2DX == true && EstatAirpositiu == true)
				state_r2d = 4;// Tot correcte: entrem a READY TO DRIVE (STEP4)
			else if (SDC == false || EstatAirpositiu == false)
				state_r2d = 0; // Qualsevol fallada de seguretat → tornar a inicial
			break;

		// En Ready To Drive, vigilem contínuament que: SDC segueixi actiu, air positiu OK i sense error d'APPS
		case 4:
			if (Brake_SensorX >= 300 && SDC == true && Button_R2DX == false && EstatAirpositiu == true)
				state_r2d = 4;// Si alguna condició de seguretat cau, deshabilitem R2D
			else if(SDC == false || EstatAirpositiu == false)
				state_r2d = 0;
			break;
	}

	// Sortides segons l'estat

	// Estat segur/no preparat: tot apagat
	if (state_r2d == 0 || state_r2d == 1)
	{
		/*HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);*/
		HAL_GPIO_WritePin(GPIOB, Buzzer_Pin, GPIO_PIN_RESET); //Estat buzzer en repòs
		HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_RESET);//Estat enable en repòs
	}
	else if (state_r2d == 2)
	{

		/*HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);*/
		HAL_GPIO_WritePin(GPIOB, Buzzer_Pin, GPIO_PIN_SET);//Estat buzzer actiu
		HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_RESET);//Estat enable en repòs
	}
	else
	{	/*__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);*/
		/*HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);*/
		HAL_GPIO_WritePin(GPIOB, Buzzer_Pin, GPIO_PIN_RESET);//Estat buzzer en repòs
		HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_SET);//Estat enable en actiu
	}
}


