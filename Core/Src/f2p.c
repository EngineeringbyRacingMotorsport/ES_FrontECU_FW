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

}
