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
            }
        }
    }
