/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FfDIGr2d_GPIO_Port, FfDIGr2d_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, FfINTebms_Pin|FfINTeimd_Pin|FfINTpwr_Pin|FfINTsbms_Pin
                          |FfERRapps_Pin|FfINTtsoff_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : FfINTmenu_Pin */
  GPIO_InitStruct.Pin = FfINTmenu_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FfINTmenu_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : FfDIGr2d_Pin */
  GPIO_InitStruct.Pin = FfDIGr2d_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FfDIGr2d_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FfDIGmicrosd_Pin FfDIGrefrion_Pin */
  GPIO_InitStruct.Pin = FfDIGmicrosd_Pin|FfDIGrefrion_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : FfINTebms_Pin FfINTeimd_Pin FfINTpwr_Pin FfINTsbms_Pin
                           FfERRapps_Pin FfINTtsoff_Pin */
  GPIO_InitStruct.Pin = FfINTebms_Pin|FfINTeimd_Pin|FfINTpwr_Pin|FfINTsbms_Pin
                          |FfERRapps_Pin|FfINTtsoff_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : FfSDCinertia_Pin FfSDCbots_Pin FfSDCcsdb_Pin FfDIGrefriauto_Pin
                           FfINTpre_Pin FfINTr2d_Pin */
  GPIO_InitStruct.Pin = FfSDCinertia_Pin|FfSDCbots_Pin|FfSDCcsdb_Pin|FfDIGrefriauto_Pin
                          |FfINTpre_Pin|FfINTr2d_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : FfDIGRvel_Pin */
  GPIO_InitStruct.Pin = FfDIGRvel_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FfDIGRvel_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : FfDIGLvel_Pin */
  GPIO_InitStruct.Pin = FfDIGLvel_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FfDIGLvel_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
