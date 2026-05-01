/*
 * MotorDriver.h — Phase 2 (STM32WB55, 64 MHz)
 * Ported from Phase 1 (STM32F103RB, 72 MHz)
 */

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "stm32wbxx_hal.h"

/* TIM1 runs at 64 MHz; prescaler = 999 → effective timer clock = 64 kHz */
#define TIM_CLOCK 64000000ul

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float integral;
    float prev_error;
} PID_t;

typedef struct {
    TIM_HandleTypeDef *pwm_tim;
    uint32_t           pwm_channel;

    TIM_HandleTypeDef *ic_tim;
    uint32_t           ic_channel;

    uint32_t last_capture;
    uint32_t period_ticks;
    uint32_t pulses_count;   /* counts completed revolutions (incremented every ppr pulses) */
    uint32_t ppr;            /* pulses per revolution */

    uint16_t speed_rpm;
    uint16_t setpoint_rpm;
    float    duty;
    PID_t    pid;
    uint8_t  moving;
} Motor_t;

void Motor_Init(Motor_t *motor,
                TIM_HandleTypeDef *pwm_tim, uint32_t pwm_channel,
                TIM_HandleTypeDef *ic_tim,  uint32_t ic_channel,
                float kp, float ki, float kd,
                uint32_t ppr);

void Motor_Start(Motor_t *motor, float duty);
void Motor_Stop(Motor_t *motor);
void Motor_SetDuty(Motor_t *motor, float duty);
void Motor_SetTargetSpeed(Motor_t *motor, float rpm);
void Motor_HandleICInterrupt(Motor_t *motor);
void Motor_UpdatePID(Motor_t *motor, float dt_sec);

#endif /* MOTOR_DRIVER_H */
