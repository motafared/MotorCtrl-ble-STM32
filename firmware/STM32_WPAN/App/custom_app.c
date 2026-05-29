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
#include "MotorDriver.h"
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
extern TIM_HandleTypeDef htim16;
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
static Motor_t motor1, motor2;
static uint8_t  motor1_counter, motor2_counter;
static uint16_t cycle_timer;
static uint8_t  mode = Mode_Stopped;
static uint32_t cycle_period = CYCLE_PERIOD;
static uint8_t  ble_connected = 0;

/* Distinguishes the two ways Mode_Continue can be entered, because they have
 * different stop semantics:
 *   1 = physical CONTINUE button (PA4 held) -> momentary: the 100 ms tick polls
 *       PA4 and stops when it reads LOW (button released).
 *   0 = web CMD_CONTINUE -> latched: PA4 is irrelevant, runs until a STOP
 *       command (or the physical STOP button).
 * Edge-based release detection proved unreliable (a bouncing transition lets the
 * ISR sample a transient HIGH, misclassify the release as a press, then the
 * 50 ms lockout swallows the settling-LOW edge). Level polling reads the settled
 * level every tick and self-corrects, so it can't get stuck. */
static volatile uint8_t continue_held = 0;
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
static void Motor_StartAll(void);
static void Motor_StopAll(void);
static void Telemetry_SendTask(void);
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
      /* Not dispatched by stack — read-refresh is in custom_stm.c VSEVT_CODE_1 blocks. */
      /* USER CODE END CUSTOM_STM_TSM1_READ_EVT */
      break;

    case CUSTOM_STM_TSM1_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TSM1_WRITE_EVT */
      if (pNotification->DataTransfered.Length >= 2)
      {
        uint16_t rpm = (uint16_t)(pNotification->DataTransfered.pPayload[0]
                     | (pNotification->DataTransfered.pPayload[1] << 8));
        Motor_SetTargetSpeed(&motor1, rpm);
      }
      /* USER CODE END CUSTOM_STM_TSM1_WRITE_EVT */
      break;

    case CUSTOM_STM_TSM2_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TSM2_READ_EVT */
      /* Not dispatched by stack. */
      /* USER CODE END CUSTOM_STM_TSM2_READ_EVT */
      break;

    case CUSTOM_STM_TSM2_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_TSM2_WRITE_EVT */
      if (pNotification->DataTransfered.Length >= 2)
      {
        uint16_t rpm = (uint16_t)(pNotification->DataTransfered.pPayload[0]
                     | (pNotification->DataTransfered.pPayload[1] << 8));
        Motor_SetTargetSpeed(&motor2, rpm);
      }
      /* USER CODE END CUSTOM_STM_TSM2_WRITE_EVT */
      break;

    case CUSTOM_STM_CT_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_CT_READ_EVT */
      /* Not dispatched by stack. */
      /* USER CODE END CUSTOM_STM_CT_READ_EVT */
      break;

    case CUSTOM_STM_CT_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_CT_WRITE_EVT */
      if (pNotification->DataTransfered.Length >= 2)
      {
        uint16_t secs = (uint16_t)(pNotification->DataTransfered.pPayload[0]
                      | (pNotification->DataTransfered.pPayload[1] << 8));
        cycle_period = (uint32_t)secs * 10ul; /* convert seconds to 100 ms ticks */
      }
      /* USER CODE END CUSTOM_STM_CT_WRITE_EVT */
      break;

    case CUSTOM_STM_CMD_WRITE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_CMD_WRITE_EVT */
      if (pNotification->DataTransfered.Length >= 1)
      {
        uint8_t cmd = pNotification->DataTransfered.pPayload[0];
        if (cmd == CMD_START && mode == Mode_Stopped)
        {
          Motor_StartAll();
          mode = Mode_Running;
        }
        else if (cmd == CMD_STOP)
        {
          Motor_StopAll();
          mode = Mode_Stopped;
          continue_held = 0;
        }
        else if (cmd == CMD_CONTINUE && mode == Mode_Stopped)
        {
          Motor_StartAll();
          mode = Mode_Continue;
          continue_held = 0;  /* web Continue is latched; PA4 must not stop it */
        }
        /* Push a fresh telemetry+status frame so the client sees the new state
         * at once — covers the stop case where the periodic timer is halted. */
        if (ble_connected)
          UTIL_SEQ_SetTask(1 << CFG_TASK_MOTOR_TELE_ID, CFG_SCH_PRIO_0);
      }
      /* USER CODE END CUSTOM_STM_CMD_WRITE_EVT */
      break;

    /* Telemetry */
    case CUSTOM_STM_ASM1_READ_EVT:
      /* USER CODE BEGIN CUSTOM_STM_ASM1_READ_EVT */
      /* Not dispatched by stack. */
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
      /* Not dispatched by stack. */
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
      /* Not dispatched by stack. */
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
      /* Not dispatched by stack. */
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
      /* Not dispatched by stack. */
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

    /* USER CODE BEGIN CUSTOM_STM_Status_EVTS */
    case CUSTOM_STM_STATUS_READ_EVT:
      /* Not dispatched by stack — read-refresh is in Custom_App_OnReadPermit. */
      break;

    case CUSTOM_STM_STATUS_NOTIFY_ENABLED_EVT:
      /* Client subscribed; current value comes via the connect-time read and the
       * next state-change notification. */
      break;

    case CUSTOM_STM_STATUS_NOTIFY_DISABLED_EVT:
      break;
    /* USER CODE END CUSTOM_STM_Status_EVTS */

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
      ble_connected = 1;
      /* USER CODE END CUSTOM_CONN_HANDLE_EVT */
      break;

    case CUSTOM_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */
      ble_connected = 0;
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
  Motor_Init(&motor1, &M1_pwm_tim, M1_pwm_channel, &M1_ic_tim, M1_ic_channel,
             M1_Kp, M1_Ki, M1_Kd, M1_PPR, M1_MIN_DUTY);
  Motor_Init(&motor2, &M2_pwm_tim, M2_pwm_channel, &M2_ic_tim, M2_ic_channel,
             M2_Kp, M2_Ki, M2_Kd, M2_PPR, M2_MIN_DUTY);
  Motor_SetTargetSpeed(&motor1, M1_SPEED);
  Motor_SetTargetSpeed(&motor2, M2_SPEED);

  UTIL_SEQ_RegTask(1 << CFG_TASK_MOTOR_TELE_ID, UTIL_SEQ_RFU, Telemetry_SendTask);
  /* USER CODE END CUSTOM_APP_Init */
  return;
}

/* USER CODE BEGIN FD */
void Custom_App_OnReadPermit(Custom_STM_Char_Opcode_t op)
{
  uint16_t v16;
  uint32_t v32;
  switch (op)
  {
    case CUSTOM_STM_TSM1:
      v16 = motor1.setpoint_rpm;
      Custom_STM_App_Update_Char(CUSTOM_STM_TSM1, (uint8_t *)&v16);
      break;
    case CUSTOM_STM_TSM2:
      v16 = motor2.setpoint_rpm;
      Custom_STM_App_Update_Char(CUSTOM_STM_TSM2, (uint8_t *)&v16);
      break;
    case CUSTOM_STM_CT:
      v16 = (uint16_t)(cycle_period / 10ul);
      Custom_STM_App_Update_Char(CUSTOM_STM_CT, (uint8_t *)&v16);
      break;
    case CUSTOM_STM_ASM1:
      v16 = motor1.speed_rpm;
      Custom_STM_App_Update_Char(CUSTOM_STM_ASM1, (uint8_t *)&v16);
      break;
    case CUSTOM_STM_ASM2:
      v16 = motor2.speed_rpm;
      Custom_STM_App_Update_Char(CUSTOM_STM_ASM2, (uint8_t *)&v16);
      break;
    case CUSTOM_STM_RM1:
      v32 = motor1.pulses_count;
      Custom_STM_App_Update_Char(CUSTOM_STM_RM1, (uint8_t *)&v32);
      break;
    case CUSTOM_STM_RM2:
      v32 = motor2.pulses_count;
      Custom_STM_App_Update_Char(CUSTOM_STM_RM2, (uint8_t *)&v32);
      break;
    case CUSTOM_STM_ET:
      v16 = (uint16_t)(cycle_timer / 10u);
      Custom_STM_App_Update_Char(CUSTOM_STM_ET, (uint8_t *)&v16);
      break;
    case CUSTOM_STM_STATUS:
    {
      uint8_t st = mode;  /* 0=stopped/idle, 1=timed, 2=continuous */
      Custom_STM_App_Update_Char(CUSTOM_STM_STATUS, &st);
      break;
    }
    default:
      break;
  }
}
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

static void Motor_StartAll(void)
{
    cycle_timer    = 0;
    motor1_counter = 1;
    motor2_counter = 1;
    Motor_Start(&motor1, 15.0f);
    Motor_Start(&motor2, 15.0f);
    HAL_TIM_Base_Start_IT(&htim16);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
}

static void Motor_StopAll(void)
{
    Motor_Stop(&motor1);
    Motor_Stop(&motor2);
    HAL_TIM_Base_Stop_IT(&htim16);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
}

static void Telemetry_SendTask(void)
{
    uint8_t buf[4];

    /* ActualSpeedM1 (uint16 LE) */
    buf[0] = (uint8_t)(motor1.speed_rpm & 0xFF);
    buf[1] = (uint8_t)(motor1.speed_rpm >> 8);
    Custom_STM_App_Update_Char(CUSTOM_STM_ASM1, buf);

    /* ActualSpeedM2 (uint16 LE) */
    buf[0] = (uint8_t)(motor2.speed_rpm & 0xFF);
    buf[1] = (uint8_t)(motor2.speed_rpm >> 8);
    Custom_STM_App_Update_Char(CUSTOM_STM_ASM2, buf);

    /* RotationsM1 (uint32 LE) — pulses_count = completed revolutions */
    buf[0] = (uint8_t)(motor1.pulses_count & 0xFF);
    buf[1] = (uint8_t)((motor1.pulses_count >> 8)  & 0xFF);
    buf[2] = (uint8_t)((motor1.pulses_count >> 16) & 0xFF);
    buf[3] = (uint8_t)((motor1.pulses_count >> 24) & 0xFF);
    Custom_STM_App_Update_Char(CUSTOM_STM_RM1, buf);

    /* RotationsM2 (uint32 LE) */
    buf[0] = (uint8_t)(motor2.pulses_count & 0xFF);
    buf[1] = (uint8_t)((motor2.pulses_count >> 8)  & 0xFF);
    buf[2] = (uint8_t)((motor2.pulses_count >> 16) & 0xFF);
    buf[3] = (uint8_t)((motor2.pulses_count >> 24) & 0xFF);
    Custom_STM_App_Update_Char(CUSTOM_STM_RM2, buf);

    /* ElapsedTime in seconds (uint16 LE) — cycle_timer ticks at 100 ms */
    uint16_t elapsed_s = cycle_timer / 10u;
    buf[0] = (uint8_t)(elapsed_s & 0xFF);
    buf[1] = (uint8_t)(elapsed_s >> 8);
    Custom_STM_App_Update_Char(CUSTOM_STM_ET, buf);

    /* Status (1 byte) — only notify when the run state actually changed, so the
     * client always reflects the true firmware state (button presses, timed
     * auto-stop) without re-sending the same byte every tick. */
    static uint8_t last_status_sent = 0xFF;
    if (mode != last_status_sent)
    {
        uint8_t st = mode;
        Custom_STM_App_Update_Char(CUSTOM_STM_STATUS, &st);
        last_status_sent = mode;
    }
}

/* -----------------------------------------------------------------------
 * HAL timer callbacks (run from ISR context)
 * ----------------------------------------------------------------------- */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM16)
        return;

    /* Elapsed run-time (ET on the dashboard) counts in BOTH running modes.
     * Clamp at uint16 max so a long web-latched Continue run reads "maxed" instead
     * of wrapping ET back to 0 (cycle_timer is 100 ms ticks => ~109 min cap). */
    if ((mode == Mode_Running || mode == Mode_Continue) && cycle_timer < 0xFFFF)
        cycle_timer++;

    /* Only the timed mode has a cycle limit that auto-stops. */
    if (mode == Mode_Running && cycle_timer >= cycle_period)
    {
        Motor_StopAll();
        mode = Mode_Stopped;
        if (ble_connected)
            UTIL_SEQ_SetTask(1 << CFG_TASK_MOTOR_TELE_ID, CFG_SCH_PRIO_0);
        return;
    }

    /* Button-held Continue: stop when PA4 settles LOW (button released). We poll
     * the settled level instead of trusting an EXTI edge — see continue_held.
     * Consensus debounce: require 2 consecutive LOW ticks (~200 ms) so a single
     * mis-sampled tick during a transition can't false-trigger. */
    {
        static uint8_t low_ticks = 0;
        if (mode == Mode_Continue && continue_held)
        {
            if (HAL_GPIO_ReadPin(BTN_CONTINUE_GPIO_Port, BTN_CONTINUE_Pin) == GPIO_PIN_RESET)
            {
                if (++low_ticks >= 2)
                {
                    Motor_StopAll();
                    mode = Mode_Stopped;
                    continue_held = 0;
                    low_ticks = 0;
                    if (ble_connected)
                        UTIL_SEQ_SetTask(1 << CFG_TASK_MOTOR_TELE_ID, CFG_SCH_PRIO_0);
                    return;
                }
            }
            else
            {
                low_ticks = 0;  /* still held high — reset the consensus counter */
            }
        }
        else
        {
            low_ticks = 0;  /* not a button-held Continue — keep the counter clean */
        }
    }

    /* PID update */
    Motor_UpdatePID(&motor1, PID_dt);
    Motor_UpdatePID(&motor2, PID_dt);
    HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin); /* heartbeat: 5 Hz blink while running */

    /* Schedule telemetry notification via sequencer (safe BLE context) */
    if (ble_connected)
        UTIL_SEQ_SetTask(1 << CFG_TASK_MOTOR_TELE_ID, CFG_SCH_PRIO_0);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM1)
        return;

    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        motor1.moving = 1;
        if (motor1_counter++ >= (uint8_t)(motor1.ppr - 1))
        {
            Motor_HandleICInterrupt(&motor1);
            motor1_counter = 0;
        }
    }
    else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        motor2.moving = 1;
        if (motor2_counter++ >= (uint8_t)(motor2.ppr - 1))
        {
            Motor_HandleICInterrupt(&motor2);
            motor2_counter = 0;
        }
    }
}

/* -----------------------------------------------------------------------
 * EXTI button callbacks (run from ISR context)
 * START/STOP are rising-edge, time-window debounced (first edge wins).
 * CONTINUE is press-to-start here; its RELEASE is handled by level polling in
 * HAL_TIM_PeriodElapsedCallback (see continue_held) rather than a falling edge.
 * ----------------------------------------------------------------------- */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_start    = 0;
    static uint32_t last_stop     = 0;
    static uint32_t last_continue = 0;
    uint32_t now = HAL_GetTick();

    uint8_t mode_before = mode;

    if (GPIO_Pin == BTN_START_Pin)
    {
        if ((now - last_start) >= DEBOUNCE_MS)
        {
            last_start = now;
            if (mode == Mode_Stopped)
            {
                Motor_StartAll();
                mode = Mode_Running;
            }
        }
    }
    else if (GPIO_Pin == BTN_STOP_Pin)
    {
        if ((now - last_stop) >= DEBOUNCE_MS)
        {
            last_stop = now;
            if (mode != Mode_Stopped)
            {
                Motor_StopAll();
                mode = Mode_Stopped;
                continue_held = 0;
            }
        }
    }
    else if (GPIO_Pin == BTN_CONTINUE_Pin)
    {
        /* Only the PRESS is handled here: a rising edge with the pin actually
         * HIGH starts Continue and marks it button-held. The RELEASE is detected
         * by polling the settled level in the tick (continue_held), which avoids
         * the bounce-misread + lockout failure of edge-based release detection.
         * Bounce on press is harmless: the mode==Stopped guard makes every edge
         * after the first a no-op. */
        if ((now - last_continue) >= DEBOUNCE_MS)
        {
            last_continue = now;
            if (mode == Mode_Stopped &&
                HAL_GPIO_ReadPin(BTN_CONTINUE_GPIO_Port, BTN_CONTINUE_Pin) == GPIO_PIN_SET)
            {
                Motor_StartAll();
                mode = Mode_Continue;
                continue_held = 1;
            }
        }
    }

    /* On a physical-button state change, schedule a telemetry+status frame so
     * BLE clients track buttons too. Scheduling from ISR is the safe pattern;
     * never call aci_* directly here. */
    if (mode != mode_before && ble_connected)
        UTIL_SEQ_SetTask(1 << CFG_TASK_MOTOR_TELE_ID, CFG_SCH_PRIO_0);
}

/* USER CODE END FD_LOCAL_FUNCTIONS*/
