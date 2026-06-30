/*
 * f2p.c
 *
 *  Created on: Jun 25, 2026
 *      Author: oriol
 */

#include "f2p.h"

void DMA2DICCF(DICCF_t *DICCF, uint32_t *buffer) {
	DICCF->RfSHU = buffer[0];
	DICCF->RfSIGlvs = buffer[1];
}

void DIG2DICCF(DICCF_t *DICCF){
	DICCF->RfSDChvd = HAL_GPIO_ReadPin(GPIOB, RfSDChvd_Pin);
	DICCF->RfSDCtsms = HAL_GPIO_ReadPin(GPIOB, RfSDCtsms_Pin);
	DICCF->RfSDClsdb = HAL_GPIO_ReadPin(GPIOB, RfSDClsdb_Pin);
	DICCF->RfSDCrsdb = HAL_GPIO_ReadPin(GPIOB, RfSDCrsdb_Pin);
	DICCF->RfSTAbrkledR = HAL_GPIO_ReadPin(GPIOA, RfSTAbrkledR_Pin);
	DICCF->RfSTAbrkledG = HAL_GPIO_ReadPin(GPIOA, RfSTAbrkledG_Pin);
	DICCF->RfSTAbrkledB = HAL_GPIO_ReadPin(GPIOA, RfSTAbrkledB_Pin);
}

void DICCF2DICCP(DICCF_t *DICCF, DICCP_t *DICCP) {
	DICCP->RpSDChvd = !DICCF->RfSDChvd;
	DICCP->RpSDCtsms = !DICCF->RfSDCtsms;
	DICCP->RpSDClsdb = !DICCF->RfSDClsdb;
	DICCP->RpSDCrsdb = !DICCF->RfSDCrsdb;

	DICCP->RpSTAbrkledR = DICCF->RfSTAbrkledR;
	DICCP->RpSTAbrkledG = DICCF->RfSTAbrkledG;
	DICCP->RpSTAbrkledB = DICCF->RfSTAbrkledB;

	DICCP->RpSIGlvs = ((DICCF->RfSIGlvs*3.3)/2095)*9.195;
	DICCP->RpSHU = ((DICCF->RfSHU*3.3)/2095)/(0.13*100)*1000;

	DICCP->FpANLbrake = DICCF->FfANLbrake*6.34;

	DICCP->IpRPM = DICCF->IfRPM;
	DICCP->IpI = DICCF->IfI;
	DICCP->IpV = DICCF->IfV;
	DICCP->IpPar = DICCF->IfPar;
	DICCP->IpT_IGBT = DICCF->IfT_IGBT;
	DICCP->IpT_Mot = DICCF->IfT_Mot;
	DICCP->IpErrL1 = (DICCF->IfErr & 0xFF);
	DICCP->IpErrH1 = ((DICCF->IfErr >> 8) & 0xFF);
	DICCP->IpErrL2 = ((DICCF->IfErr >> 16) & 0xFF);
	DICCP->IpErrH2 = ((DICCF->IfErr >> 24) & 0xFF);
}
