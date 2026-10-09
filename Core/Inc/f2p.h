/*
 * f2p.h
 *
 *  Created on: Jun 25, 2026
 *      Author: oriol
 */

#ifndef INC_F2P_H_
#define INC_F2P_H_

#include "main.h"

void DMA2DICCF(volatile DICCF_t *DICCF, volatile uint32_t *buffer);
void DIG2DICCF(volatile DICCF_t *DICCF);
void DICCF2DICCP(volatile DICCF_t *DICCF, volatile DICCP_t *DICCP);
uint16_t obtener_ultimo_delta_dma(volatile uint16_t *buf, DMA_HandleTypeDef *hdma);
void f2p_speed_calculator(volatile DICCF_t *DICCF, volatile DICCP_t *DICCP, volatile uint16_t *dma_buf_Rspeed, volatile uint16_t *dma_buf_Lspeed, DMA_HandleTypeDef *hdma_Rspeed, DMA_HandleTypeDef *hdma_Lspeed);

#endif /* INC_F2P_H_ */
