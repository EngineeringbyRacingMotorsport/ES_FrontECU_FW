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
		DICCP->FpDIGLvel = ((comptL*(14/200)) / ((HAL_GetTick() - t_time)/1000))*(36/10);
		DICCP->FpDIGRvel = ((comptR*(14/200)) / ((HAL_GetTick() - t_time)/1000))*(36/10);

		comptL=0;
		comptR=0;

		t_time = HAL_GetTick();
	};
};
