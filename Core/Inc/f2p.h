/*
 * f2p.h
 *
 *  Created on: Jun 25, 2026
 *      Author: oriol
 */

#ifndef INC_F2P_H_
#define INC_F2P_H_

#include "main.h"

void DMA2DICCF(DICCF_t *DICCF, uint32_t *buffer);
void DIG2DICCF(DICCF_t *DICCF);
void DICCF2DICCP(DICCF_t *DICCF, DICCP_t *DICCP);

#endif /* INC_F2P_H_ */
