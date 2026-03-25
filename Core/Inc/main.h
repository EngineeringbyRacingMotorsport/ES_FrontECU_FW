/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FfINTmenu_Pin GPIO_PIN_15
#define FfINTmenu_GPIO_Port GPIOC
#define FfANLRpot_Pin GPIO_PIN_0
#define FfANLRpot_GPIO_Port GPIOA
#define FfANLLpot_Pin GPIO_PIN_1
#define FfANLLpot_GPIO_Port GPIOA
#define FfANLRsus_Pin GPIO_PIN_2
#define FfANLRsus_GPIO_Port GPIOA
#define FfANLLsus_Pin GPIO_PIN_3
#define FfANLLsus_GPIO_Port GPIOA
#define FfDIGr2d_Pin GPIO_PIN_4
#define FfDIGr2d_GPIO_Port GPIOA
#define FfDIGmicrosd_Pin GPIO_PIN_6
#define FfDIGmicrosd_GPIO_Port GPIOA
#define FfSHU_Pin GPIO_PIN_7
#define FfSHU_GPIO_Port GPIOA
#define FfANLbrake_Pin GPIO_PIN_0
#define FfANLbrake_GPIO_Port GPIOB
#define FfINTebms_Pin GPIO_PIN_1
#define FfINTebms_GPIO_Port GPIOB
#define FfINTeimd_Pin GPIO_PIN_2
#define FfINTeimd_GPIO_Port GPIOB
#define FfINTpwr_Pin GPIO_PIN_11
#define FfINTpwr_GPIO_Port GPIOB
#define FfSDCinertia_Pin GPIO_PIN_12
#define FfSDCinertia_GPIO_Port GPIOB
#define FfSDCbots_Pin GPIO_PIN_13
#define FfSDCbots_GPIO_Port GPIOB
#define FfSDCcsdb_Pin GPIO_PIN_14
#define FfSDCcsdb_GPIO_Port GPIOB
#define FfINTsbms_Pin GPIO_PIN_15
#define FfINTsbms_GPIO_Port GPIOB
#define FfDIGRvel_Pin GPIO_PIN_10
#define FfDIGRvel_GPIO_Port GPIOA
#define FfDIGrefrion_Pin GPIO_PIN_15
#define FfDIGrefrion_GPIO_Port GPIOA
#define FfDIGLvel_Pin GPIO_PIN_4
#define FfDIGLvel_GPIO_Port GPIOB
#define FfERRapps_Pin GPIO_PIN_5
#define FfERRapps_GPIO_Port GPIOB
#define FfDIGrefriauto_Pin GPIO_PIN_6
#define FfDIGrefriauto_GPIO_Port GPIOB
#define FfINTpre_Pin GPIO_PIN_7
#define FfINTpre_GPIO_Port GPIOB
#define FfINTtsoff_Pin GPIO_PIN_8
#define FfINTtsoff_GPIO_Port GPIOB
#define FfINTr2d_Pin GPIO_PIN_9
#define FfINTr2d_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
typedef union {
	uint8_t array[23];
	struct{
	/* ================ MISSATGE 1 ================ */
	uint8_t  FfANLRpot  ;
	uint8_t  FfANLLpot  ;
	uint16_t FfANLRsus  ;
	uint16_t FfANLLsus  ;
	uint8_t  FfDIGRvel  ;
	uint8_t  FfDIGLvel  ;
	uint8_t  FfANLbrake ;

	/* ================ MISSATGE 2 ================ */
	uint8_t  FfINTebms      ;
	uint8_t  FfINTeimd      ;
	uint8_t  FfINTtsoff     ;
	uint8_t  FfINTsbms      ;
	uint8_t  FfINTpre       ;
	uint8_t  FfINTr2d       ;
	uint8_t  FfINTmenu      ;
	uint8_t  FfDIGmicrosd   ;
	uint8_t  FfSDCinertia   ;
	uint8_t  FfSDCbots      ;
	uint8_t  FfSDCcsdb      ;
	uint8_t  FfERRapps      ;
	uint8_t  FfDIGrefrion   ;
	uint8_t  FfDIGrefriauto ;
	uint8_t  FfDIGr2d       ;
	uint16_t FfSHU          ;
	};
} DICCF_t;

typedef struct {
	/* ================ MISSATGE 1 ================ */
	uint8_t  FpANLRpot  ;
	uint8_t  FpANLLpot  ;
	uint16_t FpANLRsus  ;
	uint16_t FpANLLsus  ;
	uint8_t  FpDIGRvel  ;
	uint8_t  FpDIGLvel  ;
	uint8_t  FpANLbrake ;

	/* ================ MISSATGE 2 ================ */
	uint8_t  FpINTebms    ;
	uint8_t  FpINTeimd    ;
	uint8_t  FpINTtsoff   ;
	uint8_t  FpINTsbms    ;
	uint8_t  FpINTpre     ;
	uint8_t  FpINTr2d     ;
	uint8_t  FpINTmenu    ;
	uint8_t  FpDIGmicrosd ;
	uint8_t  FpSDCinertia ;
	uint8_t  FpSDCbots    ;
	uint8_t  FpSDCcsdb    ;
	uint8_t  FpERRapps    ;
	uint8_t  FpDIGrefri   ;
	uint8_t  FpDIGr2d     ;
	uint16_t FpSHU        ;
	uint8_t  FpDIGvel     ;
	uint8_t  FpANLtaccu   ;
	uint8_t  FpANLvaccu   ;
} DICCP_t;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
