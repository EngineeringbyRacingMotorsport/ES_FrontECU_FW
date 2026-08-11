/*
 * can.c
 *
 *  Created on: Jun 25, 2026
 *      Author: oriol
 */
#include <can.h>

uint32_t sdctimer = 0;

void CAN_Init_Custom(FDCAN_HandleTypeDef *hfdcan) {
    FDCAN_FilterTypeDef sFilterConfig;

    // 1. Configuració de filtre per acceptar-ho TOT
    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = 0x000;
    sFilterConfig.FilterID2 = 0x7FF;

    if (HAL_FDCAN_ConfigFilter(hfdcan, &sFilterConfig) != HAL_OK) Error_Handler();

    // 2. Activar la interrupció de la FIFO 0
    if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) Error_Handler();

    // 3. Arrencar el perifèric
    if (HAL_FDCAN_Start(hfdcan) != HAL_OK) Error_Handler();
}

void CAN_Msg_Maker(volatile DICCP_t *DICCP, uint8_t *Msg1, uint8_t *Msg2, uint8_t *Msg3)
{
	/*------------MISSATGE 1-----------*/
	Msg1[0] |= (DICCP->FpDIGRpot  & 0xFF);
	Msg1[1] |= (DICCP->FpDIGLpot  & 0xFF);
	Msg1[2] |= (DICCP->FpDIGRvel  & 0xFF);
	Msg1[3] |= (DICCP->FpDIGLvel  & 0xFF);
	Msg1[4] |= (DICCP->FpANLbrake & 0xFF);

	/*---------------MISSATGE 2----------------*/
	Msg2[0] |= ((DICCP->FpINTtsoff   & 0x01) << 0);
	Msg2[0] |= ((DICCP->FpINTsbms    & 0x01) << 1);
	Msg2[0] |= ((DICCP->FpINTr2d     & 0x01) << 2);
	Msg2[0] |= ((DICCP->FpINTmenu    & 0x01) << 3);
	Msg2[0] |= ((DICCP->FpDIGmicrosd & 0x01) << 4);

	Msg2[1] |= ((DICCP->FpSDCinertia & 0x01) << 0);
	Msg2[1] |= ((DICCP->FpSDCbots    & 0x01) << 1);
	Msg2[1] |= ((DICCP->FpSDCcsdb    & 0x01) << 2);
	Msg2[1] |= ((DICCP->FpERRapps    & 0x01) << 3);
	Msg2[1] |= ((DICCP->FpDIGrefri   & 0x01) << 4);
	Msg2[1] |= ((DICCP->FpDIGr2d     & 0x01) << 5);
	Msg2[1] |= ((DICCP->FpINTrefrion & 0x01) << 6);

	Msg2[2] |= ((DICCP->FpDIGvel     & 0xFF) << 0);

	Msg2[3] |= ((DICCP->FpSHU      & 0x00FF) << 0);
	Msg2[4] |= ((DICCP->FpSHU      & 0xFF00) >> 8);

	Msg3[0] = 0x90;
	Msg3[1] = ((DICCP->FpANLRpot      & 0x00FF) << 0);
	Msg3[2] = ((DICCP->FpANLRpot      & 0xFF00) >> 8);
}

HAL_StatusTypeDef CAN_Send(FDCAN_HandleTypeDef *hfdcan, uint32_t id, uint8_t *data, uint32_t len) {
    FDCAN_TxHeaderTypeDef txHeader;
    txHeader.Identifier = id;
    txHeader.IdType = FDCAN_STANDARD_ID;
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.DataLength = len;
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker = 0;

    //HAL_GPIO_WritePin(GPIOB, SfSUPled_Pin, GPIO_PIN_SET);

    return HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &txHeader, data);
}

extern DICCP_t DICCP;

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    FDCAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
    {
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
        {
            if(RxHeader.Identifier == 0x400)
            {
            	DICCP.DpSDC = (RxData[1] & 0x01);
            }
            if(RxHeader.Identifier == 0x300)
            {
            	DICCP.ApTHRhv = (RxData[0] & 0x01);
            }
            if(RxHeader.Identifier == 0x600)
            {
            	DICCP.SpSDCbms = (RxData[0] & 0x01)>>5;
            }
            if(RxHeader.Identifier == 0x560)
            {
                // Reconstrucció del valor de 16 bits (Little-Endian / Intel Standard)
            	uint16_t raw_voltage = ((uint16_t)RxData[0] << 8) | (uint16_t)RxData[1];

                // Aplicar el factor 0.1 per obtenir els volts reals (V)
                DICCP.BpANLbatv = raw_voltage / 10;
            }
            if(RxHeader.Identifier == 0x580)
            {
            	if ((RxData[0] & 0xFF) == 1)
            	{
            		DICCP.BpANLmaxt = ((RxData[1] & 0xFF) - 50);
            	}
            }
            if (RxHeader.Identifier == 0x540)
            {
                // Reconstrucció del valor de 16 bits (Little-Endian / Intel Standard)
                uint16_t raw_current = ((uint16_t)RxData[0] << 8) | (uint16_t)RxData[1];

                // Càlcul del valor temporal
                int32_t val = (raw_current / 10) - 3200;

                // Assignació del valor absolut sense cridar abs()
                DICCP.BpANLbatc = (val < 0) ? -val : val;
            }
            if (RxHeader.Identifier == 0x103)
            {
            	if ((RxData[0] & 0xFF) == 0x4A)
            	{
            	    uint16_t raw = (uint16_t)RxData[1] | ((uint16_t)RxData[2] << 8);

            	    // Taula de punts de referència directes del fabricant (ADC vs Temp)
            	    static const uint16_t adc_lut[] = {
            	        16308, 16487, 16757, 17151, 17688, 18387, 19247,
            	        20250, 21357, 22515, 23671, 24775, 25792, 26702, 27497, 28480
            	    };
            	    static const int8_t temp_lut[] = {
            	        -30, -20, -10, 0, 10, 20, 30,
            	        40, 50, 60, 70, 80, 90, 100, 110, 125
            	    };

            	    int16_t temp = -30;
            	    uint8_t points = sizeof(adc_lut) / sizeof(adc_lut[0]);

            	    if (raw <= adc_lut[0]) {
            	        temp = temp_lut[0];
            	    } else if (raw >= adc_lut[points - 1]) {
            	        temp = temp_lut[points - 1];
            	    } else {
            	        for (uint8_t i = 0; i < points - 1; i++) {
            	            if (raw >= adc_lut[i] && raw <= adc_lut[i + 1]) {
            	                // Interpolació lineal entera entre els dos punts més propers
            	                int32_t x0 = adc_lut[i];
            	                int32_t x1 = adc_lut[i + 1];
            	                int32_t y0 = temp_lut[i];
            	                int32_t y1 = temp_lut[i + 1];

            	                temp = (int16_t)(y0 + ((int32_t)(raw - x0) * (y1 - y0)) / (x1 - x0));
            	                break;
            	            }
            	        }
            	    }

            	    DICCP.IpANLmaxt = temp; // Guardarà 32 amb raw=19435
            	}
                else if ((RxData[0] & 0xFF) == 0x49)
                {
                	// 1. Reconstrucció de la dada raw de 16 bits del Bamocar
                	uint16_t raw = (uint16_t)RxData[1] | ((uint16_t)RxData[2] << 8);

                	// 2. Càlcul lineal senzill sense decimals per al KTY81-210
                	int32_t temp = ((int32_t)raw * 11) / 1000 - 85;

                	// 3. Clamping de seguretat i assignació al camp del motor
                	if (temp < -20) temp = -20;
                	if (temp > 130) temp = 130;

                	DICCP.MpANLmaxt = (int16_t)temp;
                }
            }
        }
    }
}
