/*
 * f2p.c
 *
 *  Created on: Jun 25, 2026
 *      Author: oriol
 */

#include "f2p.h"

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


#define TIMER_ARR_MAX       0xFFFF       // Timer de 16 bits
#define SPEED_TIMEOUT_MS    250         // Tiempo sin pulsos para considerar rueda parada (0 km/h)
#define FACTOR_SPEED_KMH    123456.0f   // Factor de conversión (depende del perímetro de la rueda y nº de tornillos/dientes)

void f2p_speed_calculator(volatile DICCF_t *DICCF, volatile DICCP_t *DICCP, volatile uint16_t *dma_buf_Rspeed, volatile uint16_t *dma_buf_Lspeed) {
	// Variables estáticas para mantener el estado entre llamadas
	static uint16_t prev_capture_R = 0;
	static uint16_t prev_capture_L = 0;
	static uint32_t last_tick_R = 0;
	static uint32_t last_tick_L = 0;

	uint32_t current_time = HAL_GetTick();

	// ==========================================
	// 1. RUEDA DERECHA (Rspeed)
	// ==========================================
	uint16_t current_capture_R = dma_buf_Rspeed[0];

	if (current_capture_R != prev_capture_R) {
		uint32_t delta_t_R;

		// Manejo de desbordamiento (rollover) del contador del Timer
		if (current_capture_R >= prev_capture_R) {
			delta_t_R = current_capture_R - prev_capture_R;
		} else {
			delta_t_R = (TIMER_ARR_MAX - prev_capture_R) + current_capture_R;
		}

		prev_capture_R = current_capture_R;
		last_tick_R = current_time;

		if (delta_t_R > 0) {
			// Sustituye FpANLRspeed por el nombre exacto de la variable en tu struct DICCP
			DICCP->FpANLRspeed = (uint16_t)(FACTOR_SPEED_KMH / (float)delta_t_R);
		}
	} else if ((current_time - last_tick_R) > SPEED_TIMEOUT_MS) {
		// Timeout: La rueda se ha detenido
		DICCP->FpANLRspeed = 0;
	}

	// ==========================================
	// 2. RUEDA IZQUIERDA (Lspeed)
	// ==========================================
	uint16_t current_capture_L = dma_buf_Lspeed[0];

	if (current_capture_L != prev_capture_L) {
		uint32_t delta_t_L;

		if (current_capture_L >= prev_capture_L) {
			delta_t_L = current_capture_L - prev_capture_L;
		} else {
			delta_t_L = (TIMER_ARR_MAX - prev_capture_L) + current_capture_L;
		}

		prev_capture_L = current_capture_L;
		last_tick_L = current_time;

		if (delta_t_L > 0) {
			// Sustituye FpANLLspeed por el nombre exacto de la variable en tu struct DICCP
			DICCP->FpANLLspeed = (uint16_t)(FACTOR_SPEED_KMH / (float)delta_t_L);
		}
	} else if ((current_time - last_tick_L) > SPEED_TIMEOUT_MS) {
		// Timeout: La rueda se ha detenido
		DICCP->FpANLLspeed = 0;
	}

	// ==========================================
	// 3. VELOCIDAD GENERAL DEL VEHÍCULO
	// ==========================================
	// Si ambos sensores están dando lectura (coche en movimiento normal)
	if (DICCP->FpANLRspeed > 0 && DICCP->FpANLLspeed > 0) {
		DICCP->FpANLspeed = (DICCP->FpANLRspeed + DICCP->FpANLLspeed) / 2;
	}
	// Redundancia: si un sensor falla o la rueda patina/bloquea completamente
	else if (DICCP->FpANLRspeed > 0) {
		DICCP->FpANLspeed = DICCP->FpANLRspeed;
	}
	else if (DICCP->FpANLLspeed > 0) {
		DICCP->FpANLspeed = DICCP->FpANLLspeed;
	}
	else {
		DICCP->FpANLspeed = 0;
	}
}
