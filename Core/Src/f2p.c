/*
 * f2p.c
 *
 *  Created on: Jun 25, 2026
 *      Author: oriol
 */

#include "f2p.h"

void DMA2DICCF(DICCF_t *DICCF, uint32_t *buffer) {
		DICCF->FfANLRpot=buffer[0]&0xFFF;
		DICCF->FfANLLpot=buffer[1]&0xFFF;
		DICCF->FfANLbrake=buffer[2];
}

void DIG2DICCF(DICCF_t *DICCF){
	DICCF->FfINTr2d = HAL_GPIO_ReadPin(GPIOB, FfINTr2d_Pin);
}

void DICCF2DICCP(DICCF_t *DICCF, DICCP_t *DICCP) {
	DICCP->FpINTr2d=!DICCF->FfINTr2d;

	DICCP->FpANLRpot=((DICCF->FfANLRpot)*100)/4095;
	DICCP->FpANLLpot=((DICCF->FfANLLpot)*100)/4095;
	DICCP->FpANLbrake=DICCF->FfANLbrake;

}
