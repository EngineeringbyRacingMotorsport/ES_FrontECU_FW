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


// Configuración mecánica y eléctrica
#define DMA_BUF_SIZE        100         // Tamaño del array DMA (como en tu foto)
#define TIMER_FREQ_HZ       1000000.0f  // 1 MHz (Prescaler = 95 a 96 MHz Clock)
#define WHEEL_PERIMETER_M   1.45f       // Ajusta al perímetro real de tu neumático en metros
#define TEETH_COUNT         10          // 10 tramos de metal por vuelta
#define SPEED_TIMEOUT_MS    250         // Timeout para considerar rueda parada

// Factor para obtener velocidad en DÉCIMAS de km/h (ej: 1205 = 120.5 km/h)
// Formula: (Perímetro / Nº_Dientes) * Frecuencia_Timer * 36
#define FACTOR_SPEED_KMH    ((WHEEL_PERIMETER_M / (float)TEETH_COUNT) * TIMER_FREQ_HZ * 36.0f)

// Función auxiliar para obtener el tiempo entre los dos últimos impulsos depositados por el DMA
uint16_t obtener_ultimo_delta_dma(volatile uint16_t *buf, DMA_HandleTypeDef *hdma) {
    // CNDTR decrece desde DMA_BUF_SIZE hasta 0
    uint16_t remaining = __HAL_DMA_GET_COUNTER(hdma);
    uint16_t head = (DMA_BUF_SIZE - remaining) % DMA_BUF_SIZE; // Siguiente posición a escribir por DMA

    // Tomamos las dos últimas posiciones que el DMA escribió consecutivamente
    uint16_t curr_idx = (head == 0) ? (DMA_BUF_SIZE - 1) : (head - 1);
    uint16_t prev_idx = (curr_idx == 0) ? (DMA_BUF_SIZE - 1) : (curr_idx - 1);

    uint16_t t_curr = buf[curr_idx];
    uint16_t t_prev = buf[prev_idx];

    // Cálculo del tiempo con gestión de rollover de 16 bits
    if (t_curr >= t_prev) {
        return t_curr - t_prev;
    } else {
        return (65536 - t_prev) + t_curr;
    }
}

void f2p_speed_calculator(volatile DICCF_t *DICCF,
                          volatile DICCP_t *DICCP,
                          volatile uint16_t *dma_buf_Rspeed,
                          volatile uint16_t *dma_buf_Lspeed,
                          DMA_HandleTypeDef *hdma_Rspeed,
                          DMA_HandleTypeDef *hdma_Lspeed)
{
    static uint16_t last_capture_R = 0;
    static uint16_t last_capture_L = 0;
    static uint32_t last_tick_R = 0;
    static uint32_t last_tick_L = 0;

    uint32_t current_time = HAL_GetTick();

    // ==========================================
    // 1. RUEDA DERECHA (Rspeed)
    // ==========================================
    uint16_t delta_t_R = obtener_ultimo_delta_dma(dma_buf_Rspeed, hdma_Rspeed);

    // Verificamos si el DMA ha recibido un pulso nuevo comparando la muestra actual
    uint16_t current_head_R = dma_buf_Rspeed[(DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(hdma_Rspeed)) % DMA_BUF_SIZE];

    if (current_head_R != last_capture_R) {
        last_capture_R = current_head_R;
        last_tick_R = current_time;

        if (delta_t_R > 0) {
            DICCP->FpANLRspeed = (uint16_t)(FACTOR_SPEED_KMH / (float)delta_t_R);
        }
    } else if ((current_time - last_tick_R) > SPEED_TIMEOUT_MS) {
        DICCP->FpANLRspeed = 0; // Rueda parada
    }

    // ==========================================
    // 2. RUEDA IZQUIERDA (Lspeed)
    // ==========================================
    uint16_t delta_t_L = obtener_ultimo_delta_dma(dma_buf_Lspeed, hdma_Lspeed);
    uint16_t current_head_L = dma_buf_Lspeed[(DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(hdma_Lspeed)) % DMA_BUF_SIZE];

    if (current_head_L != last_capture_L) {
        last_capture_L = current_head_L;
        last_tick_L = current_time;

        if (delta_t_L > 0) {
            DICCP->FpANLLspeed = (uint16_t)(FACTOR_SPEED_KMH / (float)delta_t_L);
        }
    } else if ((current_time - last_tick_L) > SPEED_TIMEOUT_MS) {
        DICCP->FpANLLspeed = 0; // Rueda parada
    }

    // ==========================================
    // 3. VELOCIDAD GENERAL DEL VEHÍCULO
    // ==========================================
    if (DICCP->FpANLRspeed > 0 && DICCP->FpANLLspeed > 0) {
        DICCP->FpANLspeed = (DICCP->FpANLRspeed + DICCP->FpANLLspeed) / 2;
    } else if (DICCP->FpANLRspeed > 0) {
        DICCP->FpANLspeed = DICCP->FpANLRspeed;
    } else if (DICCP->FpANLLspeed > 0) {
        DICCP->FpANLspeed = DICCP->FpANLLspeed;
    } else {
        DICCP->FpANLspeed = 0;
    }
}
