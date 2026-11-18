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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN);
void Get_Speed(volatile DICCP_t *DICCP);
void get_vehicle_phase(volatile DICCP_t *DICCP);

// Valors de FpSTAphase (FrontECU_M2, byte 0, bits 5-7)
#define PHASE_I 0   // Idle
#define PHASE_P 1   // Precharge
#define PHASE_T 2   // TS active
#define PHASE_R 3   // Ready-to-drive
#endif /* INC_F2P_H_ */
