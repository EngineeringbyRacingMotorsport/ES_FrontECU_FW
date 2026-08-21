#include "p2f.h"
#include <stdio.h>
#include "i2c-lcd.h"

static uint8_t  switch_state_r = 0;                     // Estat en el que es troba el r2d
static uint32_t temp_R2D = 0;
static uint32_t latch_start_time = 0;

void PLC(volatile DICCP_t *DICCP)
{
	if(DICCP->SpSDCbms == 1)
	{ HAL_GPIO_WritePin(GPIOB, FfINTebms_Pin, GPIO_PIN_SET); }
	else
	{ HAL_GPIO_WritePin(GPIOB, FfINTebms_Pin, GPIO_PIN_RESET); }
}

uint8_t APPS(volatile DICCF_t *DICCF, volatile DICCP_t *DICCP){
	/*------------VARIABLES APPS-----------*/
	int32_t 	RPotX = DICCF -> FfANLRpot;																		// Valor que llegeix el ADC del potenciometre dret de l'accelerador
	int32_t 	LPotX = DICCF -> FfANLLpot;																		// Valor que llegeix el ADC del potenciometre esquerre de l'accelerador
	uint16_t 	Lpotmin = 1280;
	uint16_t 	Lpotmax = 2850;
	uint16_t 	Rpotmin = 1620;
	uint16_t 	Rpotmax = 3190;
	static uint8_t 	switch_state_a = 0;																			// Estat en el que es troba el apps
    uint16_t     Perc_Pright = (RPotX - Rpotmin)/((Rpotmax - Rpotmin)/100);                                      // Quantitat de bits que canvia el senyal del potenciometre dret per cada % que trepitjes el pedal dret.
    uint16_t     Perc_Pleft = (LPotX - Lpotmin)/((Lpotmax - Lpotmin)/100);                                       // Quantitat de bits que canvia el senyal del potenciometre esquerra per cada % que trepitjes el pedal esquerra.  									// Quantitat de bits que canvia el senyal del potenciometre esquerra per cada % que trepitjes el pedal esquerra.
	uint32_t 	APPS_temp=0;																					// Temps (en ms) en què entrem a STEP1
	int32_t diffperc = (int16_t)Perc_Pright - (int16_t)Perc_Pleft;

	if (diffperc < 0){
		diffperc = -diffperc;
	}

	switch(switch_state_a) 																						// Estructura de control per gestionar en quin punt de la lògica APPS ens trobem.
	{

	// Estat base: Tot funciona correctament.
	case 0:
		/* EXPLICACIÓ (int32_t): Forcem el canvi de tipus a enter amb signe.
		 * Si RPotX (100) < LPotX (500), la resta directa donaria un valor positiu gegant (overflow).
		 * Amb (int32_t), la resta dóna -400, i abs() pot convertir-ho correctament a 400.*/
		if( RPotX <= Rpotmin || LPotX <= Lpotmin || RPotX >= Rpotmax || LPotX >= Lpotmax){					// Comprova si els sensors estan fora de rang (per sota de 10 o per sobre de 1000).
					switch_state_a = 0;
					break;}
		// Passem a l'estat de verificació (comprovar si l'error dura 500ms).
		else if( diffperc > 10){
			APPS_temp = HAL_GetTick(); 																			// Error detectat: Guardem el "timestamp" actual en mil·lisegons.
			switch_state_a = 1;}

		else if( RPotX >= Rpotmin || LPotX >= Lpotmin || RPotX <= Rpotmax || LPotX <= Lpotmax){					// Comprova si els sensors estan fora de rang (per sota de 10 o per sobre de 1000).
			switch_state_a = 2;}// Error crític immediat (ex: cable tallat), anem a l'estat de falla.
		break;

		// Estat de "confirmació" d'error de plausibilitat.
	case 1:
		if ((HAL_GetTick() - APPS_temp) >= 100)
		{																										// Si la diferència ha persistit durant 500ms o més (regla T.4.2 de Formula Student).
			switch_state_a = 0;
		}   																									// L'error és real i persistent, bloquegem el sistema.
		else if (diffperc <= 10)
		{																										// Si la diferència torna a valors acceptables abans dels 500ms.
			switch_state_a = 2;
		} 																										// Ha estat un soroll transitori, tornem a l'estat normal.
		break;

		// Estat d'error crític (Shutdown).
	case 2:
		if( RPotX <= Rpotmin || LPotX <= Lpotmin || RPotX >= Rpotmax || LPotX >= Lpotmax){					// Comprova si els sensors estan fora de rang (per sota de 10 o per sobre de 1000).
			switch_state_a = 0;}
		// Passem a l'estat de verificació (comprovar si l'error dura 500ms).
		else if( diffperc > 10){
			APPS_temp = HAL_GetTick(); 																			// Error detectat: Guardem el "timestamp" actual en mil·lisegons.
			switch_state_a = 1;}
		else{
			switch_state_a = 2;}																			// Bucle infinit en aquest estat: el cotxe no pot accelerar fins a reiniciar.
		break;
	}
	// Sortides segons l'estat
	if (switch_state_a == 0)
	{
		DICCP -> FpERRapps = 0;
		DICCP -> FpANLRpot = 0;
		return 0;
	}
	else if (switch_state_a == 2)
	{
		DICCP -> FpERRapps = 1;
		return 1;
	}
	else{switch_state_a=switch_state_a;}
}

void R2D(volatile DICCF_t *DICCF, volatile DICCP_t *DICCP){
/*------------VARIABLES R2D-----------*/
						// Temps (en ms) en què entrem a STEP2

	switch (switch_state_r)							// Màquina d'estats finits per la seqüència Ready To Drive
	{
		// Condicions per passar d'inicial a STEP1: fre premut (DICCF->FfANLbrake >= 300), SDC actiu (SDC != 0), botó R2D premut (DICCP->FpINTr2d != 0),
		//Air positiu OK (DICCP->FpINTtsoff != 0) i sense error d'APPS (!error_apps)
		case 0:
			if (DICCP->FpANLbrake >= 5 /*&& DICCP-> DpSDC == 1*/ && DICCP->FpINTr2d == 1 && DICCP->ApTHRhv == 1 && DICCP -> FpERRapps == 1){
				switch_state_r = 1;
			}
			break;

		// En STEP1 exigim: Fre continua premut, SDC actiu, botó alliberat (!DICCP->FpINTr2d), air positiu OK i sense error APPS
		case 1:
			if (DICCP->FpANLbrake >= 5 /*&& DICCP-> DpSDC == 1*/ && DICCP->FpINTr2d != 1 && DICCP->ApTHRhv == 1 && DICCP -> FpERRapps == 1)
			{
				temp_R2D = HAL_GetTick();																															// HAL_GetTick() dona el nombre de mil·lisegons des de HAL_Init() (és un contador global de temps del sistema incrementat per l'interrupt de SysTick).
				switch_state_r = 2;																																	// Passem a STEP2 (finestra d'espera de 2 segons)
			}
			else if (/*DICCP-> DpSDC == 0 ||*/ DICCP-> ApTHRhv == 0){ 																								// Si es perd SDC, air positiu o hi ha error d'APPS, tornem a l'estat inicial
				switch_state_r = 0;
			}
			break;

		// En STEP2: SDC actiu, botó segueix alliberat, air positiu OK, sense error APPS i han passat com a mínim 2000 ms (2 s) des de temp_R2D
		case 2:
			if (DICCP->FpANLbrake >= 5 /*&& DICCP-> DpSDC == 1*/ && DICCP->ApTHRhv == 1 && (HAL_GetTick() - temp_R2D) >= 2000 && DICCP -> FpERRapps == 1){			// Aquí, determinem el temps que s'ha avançat respecte el punt inicial
				switch_state_r = 3;																																	// Si es compleix tot això, passem a STEP3
			}
			else if (/*DICCP-> DpSDC == 0 ||*/ DICCP->ApTHRhv == 0 || DICCP->FpANLbrake <= 5)	{																	// Qualsevol pèrdua de SDC, air positiu o error APPS ens fa tornar a inicial
				switch_state_r = 0;
			}
			break;

		// En Ready To Drive, vigilem contínuament que: SDC segueixi actiu, air positiu OK i sense error d'APPS
		case 3:
			if (/*DICCP-> DpSDC == 0 ||*/ DICCP->ApTHRhv == 0 || DICCP -> FpERRapps == 0){
				switch_state_r = 0;
			}
			break;
	}

	// Sortides segons l'estat
	if (switch_state_r == 0 || switch_state_r == 1)
	{
		HAL_GPIO_WritePin(GPIOC, FfINTbuzz_Pin, GPIO_PIN_RESET); 			//Estat buzzer en repòs
		DICCP-> FpDIGr2d = 0;
	}
	else if (switch_state_r == 2)
	{
		HAL_GPIO_WritePin(GPIOC, FfINTbuzz_Pin, GPIO_PIN_SET);				//Estat buzzer actiu
		DICCP-> FpDIGr2d = 0;
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC, FfINTbuzz_Pin, GPIO_PIN_RESET);			//Estat buzzer en repòs
		DICCP-> FpDIGr2d = 1;
	}
}
void Display(volatile DICCF_t *DICCF, volatile DICCP_t *DICCP){
    char buffer[10];

    // Variables per al latch de l'error APPS
    static uint32_t latch_start_time = 0;

    // Variables per al parpalleig a 2 Hz de la barra de temperatura
    static uint32_t blink_timer = 0;
    static uint8_t blink_state = 1;

    // Actualització del parpalleig (cada 250 ms canvia l'estat -> 2 Hz)
    if (HAL_GetTick() - blink_timer >= 250) {
        blink_state = !blink_state;
        blink_timer = HAL_GetTick();
    }

    // ---------------- FILA 0 ----------------
    lcd_send_cmd(0x80 | 0x00);
    sprintf(buffer, "B%02uC", DICCP->BpANLmaxt);
    lcd_send_string(buffer);

    lcd_send_cmd(0x80 | (0x00 + 5));
    sprintf(buffer, "%03ukm/h", DICCP->FpDIGvel);
    lcd_send_string(buffer);


    // ---------------- FILA 1 ----------------
    lcd_send_cmd(0x80 | 0x40);
    sprintf(buffer, "I%02uC", DICCP->IpANLmaxt);
    lcd_send_string(buffer);

    lcd_send_cmd(0x80 | (0x40 + 5));
    sprintf(buffer, "%3uA", DICCP->BpANLbatc);
    lcd_send_string(buffer);

    lcd_send_cmd(0x80 | (0x40 + 13));
    lcd_send_string("010");


    if ((DICCP->FpERRapps == 0 && (latch_start_time = HAL_GetTick(), 1)) ||
        ((HAL_GetTick() - latch_start_time) < 4000))
    {
        lcd_send_cmd(0x80 | (0x40 + 17));
        lcd_send_string("APP");
    }
    else
    {
        // Quan es superin els 4 segons de l'error, esborrem escrivint espais
        lcd_send_cmd(0x80 | (0x40 + 17));
        lcd_send_string("   ");
    }

    // ---------------- FILA 2 ----------------
    lcd_send_cmd(0x80 | 0x14);
    sprintf(buffer, "M%02uC", DICCP->MpANLmaxt);
    lcd_send_string(buffer);

    lcd_send_cmd(0x80 | (0x14 + 7));
    sprintf(buffer, "%uV", DICCP->BpANLbatv);
    lcd_send_string(buffer);

    // ---------------- FILA 3 (INDICADOR TEMPERATURA BATERIA) ----------------
    uint8_t temp = DICCP->BpANLmaxt;
    char bar_buffer[11] = "          "; // 10 espais en blanc + '\0'

    if (temp >= 40)
    {
        if (temp > 50 && !blink_state)
        {
            // Si supera els 50 °C i estem a la fase apagada del parpalleig
            memset(bar_buffer, ' ', 10);
        }
        else
        {
            // Calcula quants blocs s'omplen (de 1 a 10 blocs entre 40 i 50 °C)
            uint8_t blocks = (temp >= 50) ? 10 : (temp - 40) + 1;

            for (uint8_t i = 0; i < 10; i++)
            {
                // 0xFF sol ser el caràcter de bloc complet en pantalles HD44780 (o utilitza '#')
                bar_buffer[i] = (i < blocks) ? 0xFF : ' ';
            }
        }
    }

    lcd_send_cmd(0x80 | (0x54 + 5));
    lcd_send_string(bar_buffer);
}

