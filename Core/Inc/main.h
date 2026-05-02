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
#include "stm32wbxx_hal.h"
#include "app_conf.h"
#include "app_entry.h"
#include "app_common.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define M1_PWM_Pin GPIO_PIN_0
#define M1_PWM_GPIO_Port GPIOA
#define M2_PWM_Pin GPIO_PIN_1
#define M2_PWM_GPIO_Port GPIOA
#define BTN_STOP_Pin GPIO_PIN_2
#define BTN_STOP_GPIO_Port GPIOA
#define BTN_STOP_EXTI_IRQn EXTI2_IRQn
#define BTN_START_Pin GPIO_PIN_3
#define BTN_START_GPIO_Port GPIOA
#define BTN_START_EXTI_IRQn EXTI3_IRQn
#define BTN_CONTINUE_Pin GPIO_PIN_4
#define BTN_CONTINUE_GPIO_Port GPIOA
#define BTN_CONTINUE_EXTI_IRQn EXTI4_IRQn
#define M1_TACHO_Pin GPIO_PIN_8
#define M1_TACHO_GPIO_Port GPIOA
#define M2_TACHO_Pin GPIO_PIN_9
#define M2_TACHO_GPIO_Port GPIOA
#define LED_BLUE_Pin GPIO_PIN_4
#define LED_BLUE_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* Readable aliases — CubeMX keeps htim1/htim2/htim16 internally */
#define htim_tacho  htim1   /* tachometer input capture: M1=CH1(PA8), M2=CH2(PA9) */
#define htim_pwm    htim2   /* 20 kHz BLDC motor PWM:   M1=CH1(PA0), M2=CH2(PA1) */
#define htim_pid    htim16  /* 100 ms PID control tick */

/* Motor 1 — TIM2 CH1 PWM, TIM1 CH1 tachometer */
#define M1_PPR          6
#define M1_SPEED        1200
#define M1_Kp           0.012f
#define M1_Ki           0.0f
#define M1_Kd           0.0f
#define M1_pwm_tim      htim2
#define M1_pwm_channel  TIM_CHANNEL_1
#define M1_ic_tim       htim1
#define M1_ic_channel   TIM_CHANNEL_1

/* Motor 2 — TIM2 CH2 PWM, TIM1 CH2 tachometer */
#define M2_PPR          6
#define M2_SPEED        1200
#define M2_Kp           0.012f
#define M2_Ki           0.0f
#define M2_Kd           0.0f
#define M2_pwm_tim      htim2
#define M2_pwm_channel  TIM_CHANNEL_2
#define M2_ic_tim       htim1
#define M2_ic_channel   TIM_CHANNEL_2

/* Cycle and PID timing — TIM16 ticks at 100 ms */
#define CYCLE_PERIOD    (35 * 10ul)   /* 350 ticks = 35 s */
#define PID_dt          0.1f

/* Motor state machine modes */
#define Mode_Stopped    0
#define Mode_Running    1
#define Mode_Continue   2

/* Button debounce (ms) */
#define DEBOUNCE_MS     50

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
