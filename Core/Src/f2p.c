#include <f2p.h>

void DMA2DICCF(DICCF_t *DICCF, uint32_t *buffer1, uint32_t *buffer2){
	DICCF->FfANLRpot  = buffer1[0];
	DICCF->FfANLLpot  = buffer1[1];
	DICCF->FfANLbrake = buffer1[4];

	DICCF->FfSHU = buffer2[0];
}

void DIG2DICCF(DICCF_t *DICCF){
	DICCF->FfDIGmicrosd   =  HAL_GPIO_ReadPin(GPIOA, FfDIGmicrosd_Pin   );
	DICCF->FfSDCinertia   =  HAL_GPIO_ReadPin(GPIOB, FfSDCinertia_Pin   );
	DICCF->FfSDCbots      =  HAL_GPIO_ReadPin(GPIOB, FfSDCbots_Pin      );
	DICCF->FfSDCcsdb      =  HAL_GPIO_ReadPin(GPIOB, FfSDCcsdb_Pin      );
	DICCF->FfDIGrefrion   =  HAL_GPIO_ReadPin(GPIOA, FfDIGrefrion_Pin   );
	DICCF->FfINTr2d       =! HAL_GPIO_ReadPin(GPIOB, FfINTr2d_Pin       );
};

void DICCF2DICCP(DICCF_t *DICCF, DICCP_t *DICCP) {
	DICCP-> FpDIGmicrosd   = DICCF->FfDIGmicrosd  ;
	DICCP-> FpSDCinertia   = DICCF->FfSDCinertia  ;
	DICCP-> FpSDCbots      = DICCF->FfSDCbots     ;
	DICCP-> FpSDCcsdb      = DICCF->FfSDCcsdb     ;
	DICCP-> FpDIGrefrion   = DICCF->FfDIGrefrion  ;
	DICCP-> FpDIGrefriauto = DICCF->FfDIGrefriauto;
	DICCP-> FpINTr2d       = DICCF->FfINTr2d      ;

	DICCP-> FpANLRpot      = DICCF->FfANLRpot     ;
	DICCP-> FpANLLpot      = DICCF->FfANLLpot     ;
	DICCP-> FpANLbrake     = DICCF->FfANLbrake    ;
	DICCP-> FpSHU          = DICCF->FfSHU         ;
};
