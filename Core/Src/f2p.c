/*
 * f2p.c
 *
 *  Created on: Jun 25, 2026
 *      Author: oriol
 */

#include "f2p.h"

volatile uint8_t comptL = 0;
volatile uint8_t comptR = 0;

volatile uint32_t t_time = 0;

// Configuración mecánica y eléctrica
#define DMA_BUF_SIZE        100         // Tamaño del array DMA (como en tu foto)
#define TIMER_FREQ_HZ       1000000.0f  // 1 MHz (Prescaler = 95 a 96 MHz Clock)
#define WHEEL_PERIMETER_M   1.45f       // Ajusta al perímetro real de tu neumático en metros
#define TEETH_COUNT         40         // 10 tramos de metal por vuelta
#define SPEED_TIMEOUT_MS    250         // Timeout para considerar rueda parada


// Factor para obtener velocidad en DÉCIMAS de km/h (ej: 1205 = 120.5 km/h)
// Formula: (Perímetro / Nº_Dientes) * Frecuencia_Timer * 36
#define FACTOR_SPEED_KMH    ((WHEEL_PERIMETER_M / (float)TEETH_COUNT) * TIMER_FREQ_HZ * 36.0f)

void DMA2DICCF(volatile DICCF_t *DICCF, volatile uint32_t *buffer) {
		DICCF->FfANLRpot=buffer[2]&0xFFF;
		DICCF->FfANLLpot=buffer[1]&0xFFF;
		DICCF->FfANLbrake=buffer[0]&0xFFF;
}

void DIG2DICCF(volatile DICCF_t *DICCF){
	DICCF->FfINTr2d = HAL_GPIO_ReadPin(GPIOB, FfINTr2d_Pin);
	DICCF->FfINTrefrion = HAL_GPIO_ReadPin(GPIOA, FfINTrefrion_Pin);
	DICCF->FfSDCbots = HAL_GPIO_ReadPin(GPIOB, FfSDCbots_Pin);
	DICCF->FfSDCcsdb = HAL_GPIO_ReadPin(GPIOB, FfSDCcsdb_Pin);
	DICCF->FfSDCinertia = HAL_GPIO_ReadPin(GPIOB, FfSDCinertia_Pin);
}

void DICCF2DICCP(volatile DICCF_t *DICCF, volatile DICCP_t *DICCP) {
	DICCP->FpINTr2d=!DICCF->FfINTr2d;

	// Si el valor és major que 1500, fa la resta i multiplica per 8. Si no, clava el resultat a 0.
	DICCP->FpANLRpot = (DICCF->FfANLRpot >= 3050) ? 32000 :
	                   (DICCF->FfANLRpot <= 1720) ? 0 :
	                   (DICCF->FfANLRpot - 1680) * 370 / 16;

	DICCP->FpANLLpot = (DICCF->FfANLLpot >= 2695) ? 32000 :
	                   (DICCF->FfANLLpot <= 1350) ? 0 :
	                   (DICCF->FfANLLpot - 1300) * 370 / 16;

	DICCP->FpDIGRpot = (DICCF->FfANLRpot >= 3050) ? 255 :
	                   (DICCF->FfANLRpot <= 1720) ? 0 :
	                   ((DICCF->FfANLRpot - 1680) * 60) / 330;

	DICCP->FpDIGLpot = (DICCF->FfANLLpot >= 2700) ? 255 :
	                   (DICCF->FfANLLpot <= 1350)  ? 0 :
	                   ((DICCF->FfANLLpot - 1300) * 60) / 330;

	DICCP->FpANLbrake = (DICCF->FfANLbrake <= 2) ? 0 : (DICCF->FfANLbrake >> 4);

	DICCP->FpINTrefrion = DICCF->FfINTrefrion;

	DICCP->FpSDCbots    = !DICCF->FfSDCbots;
	DICCP->FpSDCinertia = DICCF->FfSDCinertia;
	DICCP->FpSDCcsdb    = !DICCF->FfSDCcsdb;

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN)
  {
      if (GPIO_PIN == GPIO_PIN_4)
      {
          comptL++;
      }
      if (GPIO_PIN == GPIO_PIN_10)
      {
    	  comptR++;
      }
  }

void Get_Speed(volatile DICCP_t *DICCP){

	if((HAL_GetTick() - t_time) >= 250){
		DICCP->FpANLLspeed = (((uint64_t)comptL*25200ULL)/(HAL_GetTick() - t_time));
		DICCP->FpANLRspeed = (((uint64_t)comptR*25200ULL)/(HAL_GetTick() - t_time));
		DICCP->FpANLspeed = (DICCP->FpANLRspeed+DICCP->FpANLLspeed)/200;
		uint32_t L = DICCP->FpANLLspeed;
		uint32_t R = DICCP->FpANLRspeed;
		uint32_t v_max = (L > R ) ? L : R;
		uint32_t v_min = (L > R ) ? R : L;

/*CORRETGIR SEGONS AGAFEM DATES PER SABER LA DIFERENCIA ENTRE LA VELOCITAT NORMAL I LA REAL,
 CANVIAR EL FACTOR DE DIFF I MIRAR LA TENDENCIA DE LA VELOCITAT*/
		DICCP->FpANLNspeed  = (uint16_t)v_max;

		comptL=0;
		comptR=0;

		t_time = HAL_GetTick();
	};
};

#define PRECHARGE_TIMEOUT_MS 5000   // Temps màxim per tancar l'AIR+ després d'entrar a P
#define PRECHARGE_CONFIRM_MS 6000   // Temps màxim per arribar a T (AIR+ tancat però sense confirmar T)

// Calcula la fase del vehicle (FpSTAphase): I -> P -> T -> R.
// Només llegeix FpDIGr2d: el R2D el decideix R2D(), aquí només el reflectim.
void get_vehicle_phase(volatile DICCP_t *DICCP)
{
	static uint32_t precharge_start = 0;
	static uint8_t  precharge_timed_out = 0;   // Després de sortir de P cap a I, esperem que s'obri el relé de precàrrega

	// Còpia coherent de les entrades: la interrupció de CAN no les pot canviar a mitja lectura
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	uint8_t phase  = DICCP->FpSTAphase;
	uint8_t spre   = DICCP->TpDIGspre;
	uint8_t sairp  = DICCP->TpDIGsairp;
	uint8_t sairn  = DICCP->TpDIGsairn;
	uint8_t hv     = DICCP->TpTHRhv;
	uint8_t hv_d   = DICCP->DpTHRhv;
	uint8_t hv_a   = DICCP->ApTHRhv;
	uint8_t sdc    = DICCP->DpSDC;
	uint8_t tsoff  = DICCP->FpINTtsoff;
	uint8_t r2d    = DICCP->FpDIGr2d;
	__set_PRIMASK(primask);

	uint8_t old_phase = phase;

	// any -> I: SDC obert o TS-off premut
	if (sdc == 0 || tsoff == 1) {
		phase = PHASE_I;
	}
	else {
		switch (phase) {
		case PHASE_I:
			// I -> T: el TS ja està actiu (p.ex. reset del FrontECU amb el TS engegat)
			if (sairp == 1 && sairn == 1 && spre == 0 && hv == 1 && hv_d == 1 && hv_a == 1) {
				phase = PHASE_T;
			}
			// I -> P: SDC tancat, relé de precàrrega tancat, AIR- tancat
			else if (spre == 1 && sairn == 1 && !precharge_timed_out) {
				precharge_start = HAL_GetTick();
				phase = PHASE_P;
			}
			break;

		case PHASE_P:
			// P -> I: AIR- obert, precàrrega avortada
			if (sairn == 0) {
				phase = PHASE_I;
			}
			// P -> T: AIR+ tancat, relé de precàrrega obert, alta tensió present (TSAL, HVDB i HVAB)
			else if (sairp == 1 && spre == 0 && hv == 1 && hv_d == 1 && hv_a == 1) {
				phase = PHASE_T;
			}
			// P -> I: relé de precàrrega obert sense AIR+, precàrrega avortada
			else if (spre == 0 && sairp == 0) {
				phase = PHASE_I;
			}
			// P -> I: l'AIR+ no s'ha tancat a temps, o T no s'ha confirmat a temps
			else if ((HAL_GetTick() - precharge_start) >= (sairp ? PRECHARGE_CONFIRM_MS : PRECHARGE_TIMEOUT_MS)) {
				phase = PHASE_I;
			}
			break;

		case PHASE_T:
		case PHASE_R:
			// T/R -> I: AIR+ o AIR- obert, o alta tensió perduda
			if (sairp == 0 || sairn == 0 || hv == 0) {
				phase = PHASE_I;
			}
			// T <-> R: seguim l'estat de R2D()
			else {
				phase = (r2d == 1) ? PHASE_R : PHASE_T;
			}
			break;

		default:
			phase = PHASE_I;
			break;
		}
	}

	// Qualsevol sortida de P cap a I (timeout, AIR-, SDC o TS-off) bloqueja I -> P
	// fins que veiem el relé de precàrrega obert: així el temporitzador no es reinicia
	if (old_phase == PHASE_P && phase == PHASE_I) {
		precharge_timed_out = 1;
	}
	if (spre == 0) {
		precharge_timed_out = 0;
	}

	DICCP->FpSTAphase = phase;
}
