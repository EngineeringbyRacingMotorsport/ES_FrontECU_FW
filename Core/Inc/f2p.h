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
void f2p_speed_calculator(volatile DICCF_t *DICCF, volatile DICCP_t *DICCP, volatile uint32_t *dma_buf_Rspeed, volatile uint32_t *dma_buf_Lspeed);

#endif /* INC_F2P_H_ */
