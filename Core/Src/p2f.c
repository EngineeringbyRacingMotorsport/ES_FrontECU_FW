#include "p2f.h"
#include "f2p.h"

void R2D(DICCF_t *DICCF, DICCP_t *DICCP){
/*------------VARIABLES R2D-----------*/
uint8_t 	SDC = DICCP-> DpSDC; 								// Estat de l'entrada SDC (Safety Disconnect / Shutdown circuit)
uint8_t 	TSoff  = DICCP->FpINTtsoff;							// Estat del tsoff (pressió OK)
uint8_t 	Button_R2DX = DICCP->FpINTr2d; 						// Estat del botó Ready To Drive
uint32_t 	Brake = DICCF->FfANLbrake;							// Valor que llegeix el ADC del sensor de fre
uint8_t 	Estat_r2d = DICCP-> FpDIGr2d;						// Estat final que determinarà si el cotxe està ready to drive o no
uint8_t 	switch_state_r = 0;									// Estat en el que es troba el r2d
uint32_t 	temp_R2D = 0;										// Temps (en ms) en què entrem a STEP2

	switch (switch_state_r)										// Màquina d'estats finits per la seqüència Ready To Drive
	{
		// Condicions per passar d'inicial a STEP1: fre premut (Brake >= 300), SDC actiu (SDC != 0), botó R2D premut (Button_R2DX != 0),
		//Aire positiu OK (TSoff != 0) i sense error d'APPS (!error_apps)
		case 0:
			if (Brake >= 300 && SDC == 1 && Button_R2DX == 1 && TSoff == 1){
				switch_state_r = 1;
			}
			break;

		// En STEP1 exigim: Fre continua premut, SDC actiu, botó alliberat (!Button_R2DX), air positiu OK i sense error APPS
		case 1:
			if (Brake >= 300 && SDC == 1 && Button_R2DX != 1 && TSoff == 1)
			{
				temp_R2D = HAL_GetTick();						// HAL_GetTick() dona el nombre de mil·lisegons des de HAL_Init() (és un contador global de temps del sistema incrementat per l'interrupt de SysTick).
				switch_state_r = 2;								// Passem a STEP2 (finestra d'espera de 2 segons)
			}
			else if (SDC == 0 || TSoff == 0){ 					// Si es perd SDC, air positiu o hi ha error d'APPS, tornem a l'estat inicial
				switch_state_r = 0;
			}
			break;

		// En STEP2: SDC actiu, botó segueix alliberat, air positiu OK, sense error APPS i han passat com a mínim 2000 ms (2 s) des de temp_R2D
		case 2:
			if (Brake >= 300 && SDC == 1 && TSoff == 1 && (HAL_GetTick() - temp_R2D) >= 2000){				// Aquí, determinem el temps que s'ha avançat respecte el punt inicial
				switch_state_r = 3;																			// Si es compleix tot això, passem a STEP3
			}
			else if (SDC == 0 || TSoff == 0 || Brake <= 300)	{											// Qualsevol pèrdua de SDC, air positiu o error APPS ens fa tornar a inicial
				switch_state_r = 0;
			}
			break;

		// En Ready To Drive, vigilem contínuament que: SDC segueixi actiu, air positiu OK i sense error d'APPS
		case 3:
			if (SDC == 0 || TSoff == 0){
				switch_state_r = 0;
			}
			break;
	}

	// Sortides segons l'estat
	if (switch_state_r == 0 || switch_state_r == 1)
	{
		HAL_GPIO_WritePin(GPIOB, FfINTsbms_Pin, GPIO_PIN_RESET); 			//Estat buzzer en repòs
		Estat_r2d = 0;
	}
	else if (switch_state_r == 2)
	{
		HAL_GPIO_WritePin(GPIOB, FfINTsbms_Pin, GPIO_PIN_SET);				//Estat buzzer actiu
		Estat_r2d = 0;
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, FfINTsbms_Pin, GPIO_PIN_RESET);			//Estat buzzer en repòs
		Estat_r2d = 1;
	}
}

void APPS(DICCF_t *DICCF, DICCP_t *DICCP){
/*------------VARIABLES APPS-----------*/
int32_t 	RPotX = DICCF -> FfANLRpot;									// Valor que llegeix el ADC del potenciometre dret de l'accelerador
int32_t 	LPotX = DICCF -> FfANLLpot;									// Valor que llegeix el ADC del potenciometre esquerre de l'accelerador
uint8_t 	Estat_apps = DICCP-> FpERRapps;								// Estat final que determinarà si el cotxe té una implausibilitat o no
uint8_t 	Rpotmin = 0;
uint8_t 	Rpotmax = 0;
uint8_t 	Lpotmin = 0;
uint8_t 	Lpotmax = 0;
uint8_t 	switch_state_a = 0;											// Estat en el que es troba el apps
uint8_t		Perc_Pright = (RPotX - Rpotmin)/((Rpotmax - Rpotmin)/100);  // Quantitat de bits que canvia el senyal del potenciometre dret per cada % que trepitjes el pedal dret.
uint8_t		Perc_Pleft = (LPotX - Lpotmin)/((Lpotmax - Lpotmin)/100);   // Quantitat de bits que canvia el senyal del potenciometre esquerra per cada % que trepitjes el pedal esquerra.
uint32_t 	APPS_temp=0;												// Temps (en ms) en què entrem a STEP1

	switch(switch_state_a) 												// Estructura de control per gestionar en quin punt de la lògica APPS ens trobem.
	{

	  // Estat base: Tot funciona correctament.
	  case 0:
		  /* EXPLICACIÓ (int32_t): Forcem el canvi de tipus a enter amb signe.
		   * Si RPotX (100) < LPotX (500), la resta directa donaria un valor positiu gegant (overflow).
		   * Amb (int32_t), la resta dóna -400, i abs() pot convertir-ho correctament a 400.*/
		  if( abs((int32_t)Perc_Pright - (int32_t)Perc_Pleft) >= 10){
			  APPS_temp = HAL_GetTick(); 															// Error detectat: Guardem el "timestamp" actual en mil·lisegons.
			  switch_state_a = 1;}   																// Passem a l'estat de verificació (comprovar si l'error dura 500ms).
		else if( RPotX <= Rpotmin || LPotX <= Lpotmin || RPotX >= Rpotmax || LPotX >= Lpotmax){		// Comprova si els sensors estan fora de rang (per sota de 10 o per sobre de 1000).
			  switch_state_a = 2;}   																// Error crític immediat (ex: cable tallat), anem a l'estat de falla.
		break;

	  // Estat de "confirmació" d'error de plausibilitat.
	  case 1:
		  if ((HAL_GetTick() - APPS_temp) >= 100){										// Si la diferència ha persistit durant 500ms o més (regla T.4.2 de Formula Student).
			  switch_state_a = 2;}   													// L'error és real i persistent, bloquegem el sistema.
		  else if ( abs((int32_t)Perc_Pright - (int32_t)Perc_Pleft) <= 10){				// Si la diferència torna a valors acceptables abans dels 500ms.
			  switch_state_a = 0;} 														// Ha estat un soroll transitori, tornem a l'estat normal.
		  break;

	  // Estat d'error crític (Shutdown).
	  case 2:
		  switch_state_a = 2; 															// Bucle infinit en aquest estat: el cotxe no pot accelerar fins a reiniciar.
		  break;
	}
	// Sortides segons l'estat
		if (switch_state_a == 0 || switch_state_a == 1)
		{
			Estat_apps = 0;
		}
		else
		{
			Estat_apps = 1;
		}
}

void LEDs(DICCF_t *DICCF, DICCP_t *DICCP){
/*------------VALORS DE SENYALS-----------*/
uint8_t 	BMSerror = DICCP-> FpINTebms;								// Valor de si hi ha error de BMS
uint8_t 	IMDerror = DICCP-> FpINTeimd;								// Valor de si hi ha error de IMD
uint8_t 	Apps_implausibility = DICCP-> FpERRapps;					// Valor de si hi ha error de APPS
uint8_t 	SDCstate = DICCP-> DpSDC; 								    // Estat de l'entrada SDC (Safety Disconnect / Shutdown circuit)
uint8_t 	BMSstate_error = DICCP-> FpINTsbms;							// Valor del state de la BMS

	if (BMSerror == 1){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
	}
	if (IMDerror == 1){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
	}
	else{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
		}
	if (Apps_implausibility == 1){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
	}
	else{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
		}
	if (SDCstate == 1){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	}
	else{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		}
	if (BMSstate_error == 1){
		HAL_GPIOB_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
	}
	else{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
		}
}
