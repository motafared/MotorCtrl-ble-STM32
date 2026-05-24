/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App/custom_stm.h
  * @author  MCD Application Team
  * @brief   Header for custom_stm.c module.
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
#ifndef CUSTOM_STM_H
#define CUSTOM_STM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  /* Control */
  CUSTOM_STM_TSM1,
  CUSTOM_STM_TSM2,
  CUSTOM_STM_CT,
  CUSTOM_STM_CMD,
  /* Telemetry */
  CUSTOM_STM_ASM1,
  CUSTOM_STM_ASM2,
  CUSTOM_STM_RM1,
  CUSTOM_STM_RM2,
  CUSTOM_STM_ET,
  /* USER CODE BEGIN Custom_STM_Char_Opcode_t */
  CUSTOM_STM_STATUS,   /* run state: 0=idle/stopped, 1=timed, 2=continuous.
                        * Manually added (6th telemetry char) — CubeMX GATT
                        * designer caps STM32WB services at 5 chars. */
  /* USER CODE END Custom_STM_Char_Opcode_t */
} Custom_STM_Char_Opcode_t;

typedef enum
{
  /* TargetSpeedM1 */
  CUSTOM_STM_TSM1_READ_EVT,
  CUSTOM_STM_TSM1_WRITE_EVT,
  /* TargetSpeedM2 */
  CUSTOM_STM_TSM2_READ_EVT,
  CUSTOM_STM_TSM2_WRITE_EVT,
  /* CycleTime */
  CUSTOM_STM_CT_READ_EVT,
  CUSTOM_STM_CT_WRITE_EVT,
  /* Command */
  CUSTOM_STM_CMD_WRITE_EVT,
  /* ActualSpeedM1 */
  CUSTOM_STM_ASM1_READ_EVT,
  CUSTOM_STM_ASM1_NOTIFY_ENABLED_EVT,
  CUSTOM_STM_ASM1_NOTIFY_DISABLED_EVT,
  /* ActualSpeedM2 */
  CUSTOM_STM_ASM2_READ_EVT,
  CUSTOM_STM_ASM2_NOTIFY_ENABLED_EVT,
  CUSTOM_STM_ASM2_NOTIFY_DISABLED_EVT,
  /* RotationsM1 */
  CUSTOM_STM_RM1_READ_EVT,
  CUSTOM_STM_RM1_NOTIFY_ENABLED_EVT,
  CUSTOM_STM_RM1_NOTIFY_DISABLED_EVT,
  /* RotationsM2 */
  CUSTOM_STM_RM2_READ_EVT,
  CUSTOM_STM_RM2_NOTIFY_ENABLED_EVT,
  CUSTOM_STM_RM2_NOTIFY_DISABLED_EVT,
  /* ElapsedTime */
  CUSTOM_STM_ET_READ_EVT,
  CUSTOM_STM_ET_NOTIFY_ENABLED_EVT,
  CUSTOM_STM_ET_NOTIFY_DISABLED_EVT,
  /* USER CODE BEGIN Custom_STM_Status_Evts */
  CUSTOM_STM_STATUS_READ_EVT,
  CUSTOM_STM_STATUS_NOTIFY_ENABLED_EVT,
  CUSTOM_STM_STATUS_NOTIFY_DISABLED_EVT,
  /* USER CODE END Custom_STM_Status_Evts */
  CUSTOM_STM_NOTIFICATION_COMPLETE_EVT,

  CUSTOM_STM_BOOT_REQUEST_EVT
} Custom_STM_Opcode_evt_t;

typedef struct
{
  uint8_t * pPayload;
  uint8_t   Length;
} Custom_STM_Data_t;

typedef struct
{
  Custom_STM_Opcode_evt_t       Custom_Evt_Opcode;
  Custom_STM_Data_t             DataTransfered;
  uint16_t                      ConnectionHandle;
  uint8_t                       ServiceInstance;
  uint16_t                      AttrHandle;
} Custom_STM_App_Notification_evt_t;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
extern uint16_t SizeTsm1;
extern uint16_t SizeTsm2;
extern uint16_t SizeCt;
extern uint16_t SizeCmd;
extern uint16_t SizeAsm1;
extern uint16_t SizeAsm2;
extern uint16_t SizeRm1;
extern uint16_t SizeRm2;
extern uint16_t SizeEt;

/* USER CODE BEGIN EC */
extern uint16_t SizeStatus;
/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions ------------------------------------------------------- */
void SVCCTL_InitCustomSvc(void);
void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification);
tBleStatus Custom_STM_App_Update_Char(Custom_STM_Char_Opcode_t CharOpcode,  uint8_t *pPayload);
tBleStatus Custom_STM_App_Update_Char_Variable_Length(Custom_STM_Char_Opcode_t CharOpcode, uint8_t *pPayload, uint8_t size);
tBleStatus Custom_STM_App_Update_Char_Ext(uint16_t Connection_Handle, Custom_STM_Char_Opcode_t CharOpcode, uint8_t *pPayload);
/* USER CODE BEGIN EF */

/* USER CODE END EF */

#ifdef __cplusplus
}
#endif

#endif /*CUSTOM_STM_H */
