#include "p2f.h"
#include "f2p.h"

void R2D(DICCF_t *DICCF, DICCP_t *DICCP){
/*------------VARIABLES R2D-----------*/
uint8_t 	SDC = DICCP-> DpSDC; 								// Estat de l'entrada SDC (Safety Disconnect / Shutdown circuit)
uint8_t 	TSoff  = DICCP->FpINTtsoff;							// Estat del tsoff (pressió OK)
uint8_t 	Button_R2DX = DICCP->FpINTr2d; 						// Estat del botó Ready To Drive
uint32_t 	Brake = DICCF->FfANLbrake;							// Valor que llegeix el ADC del sensor de fre
uint8_t 	Estat_r2d = DICCP-> FpDIGr2d;						// Estat final que determinarà si el cotxe està ready to drive o no
uint8_t 	switch_state = 0;									// Estat en el que es troba el r2d
uint32_t 	temp_R2D = 0;										// Temps (en ms) en què entrem a STEP2

	// Màquina d'estats finits per la seqüència Ready To Drive
	switch (switch_state)
	{
		// Condicions per passar d'inicial a STEP1: fre premut (Brake >= 300), SDC actiu (SDC != 0), botó R2D premut (Button_R2DX != 0),
		//Aire positiu OK (TSoff != 0) i sense error d'APPS (!error_apps)
		case 0:
			if (Brake >= 300 && SDC == 1 && Button_R2DX == 1 && TSoff == 1){
				switch_state = 1;
			}
			break;

		// En STEP1 exigim: Fre continua premut, SDC actiu, botó alliberat (!Button_R2DX), air positiu OK i sense error APPS
		case 1:
			if (Brake >= 300 && SDC == 1 && Button_R2DX != 1 && TSoff == 1)
			{
				temp_R2D = HAL_GetTick();			// HAL_GetTick() dona el nombre de mil·lisegons des de HAL_Init() (és un contador global de temps del sistema incrementat per l'interrupt de SysTick).
				switch_state = 2;					// Passem a STEP2 (finestra d'espera de 2 segons)
			}
			else if (SDC == 0 || TSoff == 0){ 		// Si es perd SDC, air positiu o hi ha error d'APPS, tornem a l'estat inicial
				switch_state = 0;
			}
			break;

		// En STEP2: SDC actiu, botó segueix alliberat, air positiu OK, sense error APPS i han passat com a mínim 2000 ms (2 s) des de temp_R2D
		case 2:
			if (Brake >= 300 && SDC == 1 && TSoff == 1 && (HAL_GetTick() - temp_R2D) >= 2000){		// Aquí, determinem el temps que s'ha avançat respecte el punt inicial
				switch_state = 3;																			// Si es compleix tot això, passem a STEP3
			}
			else if (SDC == 0 || TSoff == 0 || Brake <= 300)	{											// Qualsevol pèrdua de SDC, air positiu o error APPS ens fa tornar a inicial
				switch_state = 0;
			}
			break;

		// En Ready To Drive, vigilem contínuament que: SDC segueixi actiu, air positiu OK i sense error d'APPS
		case 3:
			if (SDC == 0 || TSoff == 0){
				switch_state = 0;
			}
			break;
	}

	// Sortides segons l'estat
	if (switch_state == 0 || switch_state == 1)
	{
		HAL_GPIO_WritePin(GPIOB, Buzzer_Pin, GPIO_PIN_RESET); 			//Estat buzzer en repòs
		HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_RESET);			//Estat enable en repòs
		Estat_r2d = 0;
	}
	else if (switch_state == 2)
	{
		HAL_GPIO_WritePin(GPIOB, Buzzer_Pin, GPIO_PIN_SET);				//Estat buzzer actiu
		HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_RESET);			//Estat enable en repòs
		Estat_r2d = 0;
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, Buzzer_Pin, GPIO_PIN_RESET);			//Estat buzzer en repòs
		HAL_GPIO_WritePin(GPIOB, Enable_Pin, GPIO_PIN_SET);				//Estat enable en actiu
		Estat_r2d = 1;
	}
}
