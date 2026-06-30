/*
 * can.c
 *
 *  Created on: Jun 25, 2026
 *      Author: oriol
 */

#include <can.h>

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

void CAN_Msg_Maker(DICCP_t *DICCP, uint8_t *Msg1, uint8_t *Msg2, uint8_t *Msg3)
{
	Msg1[0] |= ((DICCP->RpSDChvd & 0x01) << 0);
	Msg1[0] |= ((DICCP->RpSDCtsms & 0x01) << 1);
	Msg1[0] |= ((DICCP->RpSDCrsdb & 0x01) << 2);
	Msg1[0] |= ((DICCP->RpSDClsdb & 0x01) << 3);
	Msg1[0] |= ((DICCP->RpSTAbrkledR & 0x01) << 4);
	Msg1[0] |= ((DICCP->RpSTAbrkledG & 0x01) << 5);
	Msg1[0] |= ((DICCP->RpSTAbrkledB & 0x01) << 6);

	Msg1[1] |= ((DICCP->RpSIGlvs & 0x00FF) << 0);
	Msg1[2] |= ((DICCP->RpSIGlvs & 0xFF00) >> 8);

	Msg1[3] |= ((DICCP->RpSHU & 0x00FF) << 0);
	Msg1[4] |= ((DICCP->RpSHU & 0xFF00) >> 8);

	Msg2[0] |= ((DICCP->IpRPM & 0x00FF) << 0);
	Msg2[1] |= ((DICCP->IpRPM & 0xFF00) >> 8);

	Msg2[2] |= ((DICCP->IpI & 0x00FF) << 0);
	Msg2[3] |= ((DICCP->IpI & 0xFF00) >> 8);

	Msg2[4] |= ((DICCP->IpV & 0x00FF) << 0);
	Msg2[5] |= ((DICCP->IpV & 0xFF00) >> 8);

	Msg2[6] |= ((DICCP->IpPar & 0x00FF) << 0);
	Msg2[7] |= ((DICCP->IpPar & 0xFF00) >> 8);

	Msg3[0] |= ((DICCP->IpT_IGBT & 0x00FF) << 0);
	Msg3[1] |= ((DICCP->IpT_IGBT & 0xFF00) >> 8);

	Msg3[2] |= ((DICCP->IpT_Mot & 0x00FF) << 0);
	Msg3[3] |= ((DICCP->IpT_Mot & 0xFF00) >> 8);

	Msg3[4] |= DICCP->IpErrL1;
	Msg3[5] |= DICCP->IpErrH1;
	Msg3[6] |= DICCP->IpErrL2;
	Msg3[7] |= DICCP->IpErrH2;
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


void Inverter_Request_Data(FDCAN_HandleTypeDef *hfdcan, uint8_t regID, uint8_t interval_ms)
{
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[3];

    // Configuración específica para el periférico FDCAN del STM32G4
    TxHeader.Identifier = 0x201;                      // ID estándar del Bamocar (Nodo 1)
    TxHeader.IdType = FDCAN_STANDARD_ID;              // Identificador estándar (11 bits)
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;          // Frame de datos normal
    TxHeader.DataLength = FDCAN_DLC_BYTES_3;          // El Bamocar exige exactamente 3 bytes
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;           // Sin cambio de velocidad
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;            // ¡CRÍTICO! Forzar modo CAN Clásico
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    // Datos del protocolo Unitek
    TxData[0] = 0x3D;                                 // Comando de lectura
    TxData[1] = regID;                                // Registro solicitado
    TxData[2] = interval_ms;                          // Intervalo cíclico

    // Enviar a la cola de transmisión del FDCAN
    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, TxData) != HAL_OK)
    {
        // Manejar error de bus si fuera necesario
    }
}

extern DICCF_t DICCF;
extern DICCP_t DICCP;

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    FDCAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
    {
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
        {
            if (RxHeader.Identifier == 0x181)
            {
                uint8_t regID = RxData[0];

                if (regID != 0x8F)
                {
                    int16_t raw_value = (RxData[2] << 8) | RxData[1];

                    switch (regID)
                    {
                        case 0x30:
                            DICCF.IfRPM = raw_value;
                            break;
                        case 0x48:
                        	 DICCF.IfI = raw_value;
                            break;
                        case 0xA8:
                        	 DICCF.IfV = raw_value ;
                            break;
                        case 0x4A:
                        	 DICCF.IfT_IGBT = raw_value;
                            break;
                        case 0x49:
                        	 DICCF.IfT_Mot = raw_value;
                            break;
                        case 0xA0:
                        	DICCF.IfPar = raw_value;
                        	break;
                        default:
                        	break;
                    }
                }
                else if (regID == 0x8F)
                {
                	 DICCF.IfErr = ((uint32_t)RxData[4] << 24) |
                                      ((uint32_t)RxData[3] << 16) |
                                      ((uint32_t)RxData[2] << 8)  |
                                      RxData[1];
                }
            }

            if(RxHeader.IdType == 0x100)
            {
            	DICCF.FfANLbrake = RxData[4];
            }
        }
    }
}
