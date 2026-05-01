/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App/custom_app.c
  * @author  MCD Application Team
  * @brief   Custom Example Application (Server)
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
#include "main.h"
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "custom_app.h"
#include "custom_stm.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  /* Control */
  /* Telemetry */
  uint8_t               Asm1_Notification_Status;
  uint8_t               Asm2_Notification_Status;
  uint8_t               Rm1_Notification_Status;
  uint8_t               Rm2_Notification_Status;
  uint8_t               Et_Notification_Status;
  /* USER CODE BEGIN CUSTOM_APP_Context_t */

  /* USER CODE END CUSTOM_APP_Context_t */

  uint16_t              ConnectionHandle;
} Custom_App_Context_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

static Custom_App_Context_t Custom_App_Context;

/**
 * END of Section BLE_APP_CONTEXT
 */

uint8_t UpdateCharData[512];
uint8_t NotifyCharData[512];
uint16_t Connection_Handle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* Control */
/* Telemetry */
static void Custom_Asm1_Update_Char(void);
static void Custom_Asm1_Send_Notification(void);
static void Custom_Asm2_Update_Char(void);
static void Custom_Asm2_Send_Notification(void);
static void Custom_Rm1_Update_Char(void);
static void Custom_Rm1_Send_Notification(void);
static void Custom_Rm2_Update_Char(void);
static void Custom_Rm2_Send_Notification(void);
static void Custom_Et_Update_Char(void);
static void Custom_Et_Send_Notification(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_1 */

  /* USER CODE END CUSTOM_STM_App_Notification_1 */
  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* USER CODE END CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* Control */
    case CUSTOM_STM_TSM1_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TSM1_READ_EVT */

      /* USER CODE END CUSTOM_STM_TSM1_READ_EVT */
      break;

    case CUSTOM_STM_TSM1_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TSM1_WRITE_EVT */

      /* USER CODE END CUSTOM_STM_TSM1_WRITE_EVT */
      break;

    case CUSTOM_STM_TSM2_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TSM2_READ_EVT */

      /* USER CODE END CUSTOM_STM_TSM2_READ_EVT */
      break;

    case CUSTOM_STM_TSM2_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TSM2_WRITE_EVT */

      /* USER CODE END CUSTOM_STM_TSM2_WRITE_EVT */
      break;

    case CUSTOM_STM_CT_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_CT_READ_EVT */

      /* USER CODE END CUSTOM_STM_CT_READ_EVT */
      break;

    case CUSTOM_STM_CT_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_CT_WRITE_EVT */

      /* USER CODE END CUSTOM_STM_CT_WRITE_EVT */
      break;

    case CUSTOM_STM_CMD_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_CMD_WRITE_EVT */

      /* USER CODE END CUSTOM_STM_CMD_WRITE_EVT */
      break;

    /* Telemetry */
    case CUSTOM_STM_ASM1_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ASM1_READ_EVT */

      /* USER CODE END CUSTOM_STM_ASM1_READ_EVT */
      break;

    case CUSTOM_STM_ASM1_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ASM1_NOTIFY_ENABLED_EVT */

      /* USER CODE END CUSTOM_STM_ASM1_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_ASM1_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ASM1_NOTIFY_DISABLED_EVT */

      /* USER CODE END CUSTOM_STM_ASM1_NOTIFY_DISABLED_EVT */
      break;

    case CUSTOM_STM_ASM2_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ASM2_READ_EVT */

      /* USER CODE END CUSTOM_STM_ASM2_READ_EVT */
      break;

    case CUSTOM_STM_ASM2_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ASM2_NOTIFY_ENABLED_EVT */

      /* USER CODE END CUSTOM_STM_ASM2_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_ASM2_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ASM2_NOTIFY_DISABLED_EVT */

      /* USER CODE END CUSTOM_STM_ASM2_NOTIFY_DISABLED_EVT */
      break;

    case CUSTOM_STM_RM1_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_RM1_READ_EVT */

      /* USER CODE END CUSTOM_STM_RM1_READ_EVT */
      break;

    case CUSTOM_STM_RM1_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_RM1_NOTIFY_ENABLED_EVT */

      /* USER CODE END CUSTOM_STM_RM1_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_RM1_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_RM1_NOTIFY_DISABLED_EVT */

      /* USER CODE END CUSTOM_STM_RM1_NOTIFY_DISABLED_EVT */
      break;

    case CUSTOM_STM_RM2_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_RM2_READ_EVT */

      /* USER CODE END CUSTOM_STM_RM2_READ_EVT */
      break;

    case CUSTOM_STM_RM2_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_RM2_NOTIFY_ENABLED_EVT */

      /* USER CODE END CUSTOM_STM_RM2_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_RM2_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_RM2_NOTIFY_DISABLED_EVT */

      /* USER CODE END CUSTOM_STM_RM2_NOTIFY_DISABLED_EVT */
      break;

    case CUSTOM_STM_ET_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ET_READ_EVT */

      /* USER CODE END CUSTOM_STM_ET_READ_EVT */
      break;

    case CUSTOM_STM_ET_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ET_NOTIFY_ENABLED_EVT */

      /* USER CODE END CUSTOM_STM_ET_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_ET_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ET_NOTIFY_DISABLED_EVT */

      /* USER CODE END CUSTOM_STM_ET_NOTIFY_DISABLED_EVT */
      break;

    case CUSTOM_STM_NOTIFICATION_COMPLETE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */

      /* USER CODE END CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_STM_App_Notification_default */

      /* USER CODE END CUSTOM_STM_App_Notification_default */
      break;
  }
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_2 */

  /* USER CODE END CUSTOM_STM_App_Notification_2 */
  return;
}

void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_APP_Notification_1 */

  /* USER CODE END CUSTOM_APP_Notification_1 */

  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_APP_Notification_Custom_Evt_Opcode */

    /* USER CODE END P2PS_CUSTOM_Notification_Custom_Evt_Opcode */
    case CUSTOM_CONN_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_CONN_HANDLE_EVT */

      /* USER CODE END CUSTOM_CONN_HANDLE_EVT */
      break;

    case CUSTOM_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */

      /* USER CODE END CUSTOM_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_APP_Notification_default */

      /* USER CODE END CUSTOM_APP_Notification_default */
      break;
  }

  /* USER CODE BEGIN CUSTOM_APP_Notification_2 */

  /* USER CODE END CUSTOM_APP_Notification_2 */

  return;
}

void Custom_APP_Init(void)
{
  /* USER CODE BEGIN CUSTOM_APP_Init */

  /* USER CODE END CUSTOM_APP_Init */
  return;
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/* Control */
/* Telemetry */
__USED void Custom_Asm1_Update_Char(void) /* Property Read */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Asm1_UC_1*/

  /* USER CODE END Asm1_UC_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_ASM1, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN Asm1_UC_Last*/

  /* USER CODE END Asm1_UC_Last*/
  return;
}

void Custom_Asm1_Send_Notification(void) /* Property Notification */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Asm1_NS_1*/

  /* USER CODE END Asm1_NS_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_ASM1, (uint8_t *)NotifyCharData);
  }

  /* USER CODE BEGIN Asm1_NS_Last*/

  /* USER CODE END Asm1_NS_Last*/

  return;
}

__USED void Custom_Asm2_Update_Char(void) /* Property Read */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Asm2_UC_1*/

  /* USER CODE END Asm2_UC_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_ASM2, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN Asm2_UC_Last*/

  /* USER CODE END Asm2_UC_Last*/
  return;
}

void Custom_Asm2_Send_Notification(void) /* Property Notification */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Asm2_NS_1*/

  /* USER CODE END Asm2_NS_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_ASM2, (uint8_t *)NotifyCharData);
  }

  /* USER CODE BEGIN Asm2_NS_Last*/

  /* USER CODE END Asm2_NS_Last*/

  return;
}

__USED void Custom_Rm1_Update_Char(void) /* Property Read */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Rm1_UC_1*/

  /* USER CODE END Rm1_UC_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_RM1, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN Rm1_UC_Last*/

  /* USER CODE END Rm1_UC_Last*/
  return;
}

void Custom_Rm1_Send_Notification(void) /* Property Notification */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Rm1_NS_1*/

  /* USER CODE END Rm1_NS_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_RM1, (uint8_t *)NotifyCharData);
  }

  /* USER CODE BEGIN Rm1_NS_Last*/

  /* USER CODE END Rm1_NS_Last*/

  return;
}

__USED void Custom_Rm2_Update_Char(void) /* Property Read */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Rm2_UC_1*/

  /* USER CODE END Rm2_UC_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_RM2, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN Rm2_UC_Last*/

  /* USER CODE END Rm2_UC_Last*/
  return;
}

void Custom_Rm2_Send_Notification(void) /* Property Notification */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Rm2_NS_1*/

  /* USER CODE END Rm2_NS_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_RM2, (uint8_t *)NotifyCharData);
  }

  /* USER CODE BEGIN Rm2_NS_Last*/

  /* USER CODE END Rm2_NS_Last*/

  return;
}

__USED void Custom_Et_Update_Char(void) /* Property Read */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Et_UC_1*/

  /* USER CODE END Et_UC_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_ET, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN Et_UC_Last*/

  /* USER CODE END Et_UC_Last*/
  return;
}

void Custom_Et_Send_Notification(void) /* Property Notification */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN Et_NS_1*/

  /* USER CODE END Et_NS_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_ET, (uint8_t *)NotifyCharData);
  }

  /* USER CODE BEGIN Et_NS_Last*/

  /* USER CODE END Et_NS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/

/* USER CODE END FD_LOCAL_FUNCTIONS*/
