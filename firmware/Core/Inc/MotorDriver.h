/*
 * MotorDriver.h — Phase 2 (STM32WB55, 64 MHz)
 * Ported from Phase 1 (STM32F103RB, 72 MHz)
 */

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "stm32wbxx_hal.h"

/* TIM1 runs at 64 MHz; prescaler = 999 → effective timer clock = 64 kHz */
#define TIM_CLOCK 64000000ul

/* Debug cap: never drive PWM to full 100%. Enforced in Motor_SetDuty. */
#define MOTOR_MAX_DUTY 100.0f
/* Minimum running duty is mode-specific (see M1_MIN_DUTY/M2_MIN_DUTY in main.h)
 * and passed into Motor_Init; stored per-motor and enforced by the PID. */
/* Speed-reporting timeout. Motor_UpdatePID runs at 10 Hz (100 ms/tick), and a
 * tacho pulse may legitimately be slower than one tick at low RPM. Hold the
 * last measured speed until this many consecutive ticks pass with no pulse,
 * then declare 0. 20 ticks = 2 s → won't false-zero down to ~30 RPM (ppr=1).
 * Lower it for faster stop detection; raise it to support slower speeds. */
#define SPEED_TIMEOUT_TICKS 20u

/* Glitch filter for the tacho input capture. Any capture implying a speed
 * above this RPM is physically impossible (sensor bounce / noise) and is
 * ignored. The capture callback decimates by ppr, so the measured interval is
 * always one full revolution and the tick threshold (computed in Motor_Init)
 * is PPR-independent. Keep comfortably above the real motor max (~1500 RPM). */
#define MOTOR_MAX_RPM 3000u

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
    uint32_t min_diff_ticks; /* IC glitch filter: reject captures closer than
                                this (prescaled ticks); one rev at MOTOR_MAX_RPM */

    uint16_t speed_rpm;
    uint16_t setpoint_rpm;
    float    duty;
    float    min_duty;       /* mode-specific running floor (0 = none); enforced in PID */
    PID_t    pid;
    uint8_t  moving;
    uint16_t stale_ticks;    /* PID ticks since last tacho pulse (speed-hold timeout) */
} Motor_t;

void Motor_Init(Motor_t *motor,
                TIM_HandleTypeDef *pwm_tim, uint32_t pwm_channel,
                TIM_HandleTypeDef *ic_tim,  uint32_t ic_channel,
                float kp, float ki, float kd,
                uint32_t ppr, float min_duty);

void Motor_Start(Motor_t *motor, float duty);
void Motor_Stop(Motor_t *motor);
void Motor_SetDuty(Motor_t *motor, float duty);
void Motor_SetTargetSpeed(Motor_t *motor, float rpm);
void Motor_HandleICInterrupt(Motor_t *motor);
void Motor_UpdatePID(Motor_t *motor, float dt_sec);

#endif /* MOTOR_DRIVER_H */
