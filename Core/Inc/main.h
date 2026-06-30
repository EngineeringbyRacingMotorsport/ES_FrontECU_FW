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
#define RfSTArefriaccu_Pin GPIO_PIN_13
#define RfSTArefriaccu_GPIO_Port GPIOC
#define RfSTArefriinverter_Pin GPIO_PIN_14
#define RfSTArefriinverter_GPIO_Port GPIOC
#define RfSTArefrimot_Pin GPIO_PIN_15
#define RfSTArefrimot_GPIO_Port GPIOC
#define RfSTAbrkledR_Pin GPIO_PIN_4
#define RfSTAbrkledR_GPIO_Port GPIOA
#define RfSTAbrkledG_Pin GPIO_PIN_5
#define RfSTAbrkledG_GPIO_Port GPIOA
#define RfSTAbrkledB_Pin GPIO_PIN_6
#define RfSTAbrkledB_GPIO_Port GPIOA
#define RfSHU_Pin GPIO_PIN_7
#define RfSHU_GPIO_Port GPIOA
#define RfSIGlvs_Pin GPIO_PIN_2
#define RfSIGlvs_GPIO_Port GPIOB
#define RfSDChvd_Pin GPIO_PIN_11
#define RfSDChvd_GPIO_Port GPIOB
#define RfSDCtsms_Pin GPIO_PIN_12
#define RfSDCtsms_GPIO_Port GPIOB
#define RfSDClsdb_Pin GPIO_PIN_13
#define RfSDClsdb_GPIO_Port GPIOB
#define RfSDCrsdb_Pin GPIO_PIN_14
#define RfSDCrsdb_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

typedef struct {
		//Analog
		uint16_t RfSIGlvs;
		uint16_t RfSHU;
		//Digital
		uint8_t RfSDChvd;
		uint8_t RfSDCtsms;
		uint8_t RfSDClsdb;
		uint8_t RfSDCrsdb;
		uint8_t RfSTAbrkledR;
		uint8_t RfSTAbrkledG;
		uint8_t RfSTAbrkledB;
		//Front ECU
		uint8_t FfANLbrake;
		//Inverter
		uint16_t IfRPM;
		uint16_t IfI;
		uint16_t IfPar;
		uint16_t IfV;
		uint16_t IfT_IGBT;
		uint16_t IfT_Mot;
		uint32_t IfErr;
} DICCF_t;

extern DICCF_t DICCF;

typedef struct {
	//Analog
	uint16_t RpSIGlvs;
	uint16_t RpSHU;
	//Digital
	uint8_t RpSDChvd;
	uint8_t RpSDCtsms;
	uint8_t RpSDClsdb;
	uint8_t RpSDCrsdb;
	uint8_t RpSTAbrkledR;
	uint8_t RpSTAbrkledG;
	uint8_t RpSTAbrkledB;
	//Front ECU
	uint8_t FpANLbrake;
	//Inverter
	uint8_t IpRPM;
	uint8_t IpI;
	uint8_t IpPar;
	uint8_t IpV;
	uint8_t IpT_IGBT;
	uint8_t IpT_Mot;
	uint8_t IpErrL1;
	uint8_t IpErrH1;
	uint8_t IpErrL2;
	uint8_t IpErrH2;
} DICCP_t;

extern DICCP_t DICCP;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
