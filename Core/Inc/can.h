#ifndef can_h
#define can_h

#include "main.h"

#define CAN_RX_BUF_SIZE 64

typedef struct {
    FDCAN_RxHeaderTypeDef header;
    uint8_t data[8];
} CAN_Msg;


void CAN_Msg_Maker(DICCP_t *DICCP, uint8_t *Msg1, uint8_t *Msg2);
void CAN_Init_Custom(FDCAN_HandleTypeDef *hfdcan);
HAL_StatusTypeDef CAN_Send(FDCAN_HandleTypeDef *hfdcan, uint32_t id, uint8_t *data, uint32_t len);
uint8_t CAN_Read(DICCP_t *DICCP);

#endif
