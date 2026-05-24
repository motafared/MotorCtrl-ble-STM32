/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App/custom_app.h
  * @author  MCD Application Team
  * @brief   Header for custom_app.c module
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
#ifndef CUSTOM_APP_H
#define CUSTOM_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "custom_stm.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  CUSTOM_CONN_HANDLE_EVT,
  CUSTOM_DISCON_HANDLE_EVT,
} Custom_App_Opcode_Notification_evt_t;

typedef struct
{
  Custom_App_Opcode_Notification_evt_t     Custom_Evt_Opcode;
  uint16_t                                 ConnectionHandle;
} Custom_App_ConnHandle_Not_evt_t;
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define CMD_START      0x01   /* Start timed run (Mode_Running) */
#define CMD_STOP       0x02   /* Stop motors (any mode) */
#define CMD_CONTINUE   0x03   /* Start continuous run (Mode_Continue) */
/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions ---------------------------------------------*/
void Custom_APP_Init(void);
void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification);
/* USER CODE BEGIN EF */
/* Refresh a readable characteristic from its live firmware source.
 * Called from custom_stm.c's read-permit handler BEFORE aci_gatt_allow_read(),
 * so the client always reads the current truth instead of a stale cached value. */
void Custom_App_OnReadPermit(Custom_STM_Char_Opcode_t op);
/* USER CODE END EF */

#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_APP_H */
