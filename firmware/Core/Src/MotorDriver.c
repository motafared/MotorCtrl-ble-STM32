/*
 * MotorDriver.c — Phase 2 (STM32WB55, 64 MHz)
 * Ported from Phase 1 (STM32F103RB, 72 MHz)
 * Speed reporting is now via BLE telemetry; printf removed.
 */

#include "MotorDriver.h"

void Motor_Init(Motor_t *motor,
                TIM_HandleTypeDef *pwm_tim, uint32_t pwm_channel,
                TIM_HandleTypeDef *ic_tim,  uint32_t ic_channel,
                float kp, float ki, float kd,
                uint32_t ppr, float min_duty)
{
    motor->moving       = 0;
    motor->pwm_tim      = pwm_tim;
    motor->pwm_channel  = pwm_channel;
    motor->ic_tim       = ic_tim;
    motor->ic_channel   = ic_channel;
    motor->ppr          = ppr;
    motor->min_duty     = min_duty;

    motor->last_capture = 0;
    motor->period_ticks = 0;
    motor->pulses_count = 0;
    motor->speed_rpm    = 0;
    motor->setpoint_rpm = 0;
    motor->duty         = 0;
    motor->stale_ticks  = 0;

    motor->pid.Kp         = kp;
    motor->pid.Ki         = ki;
    motor->pid.Kd         = kd;
    motor->pid.integral   = 0.0f;
    motor->pid.prev_error = 0.0f;

    Motor_SetDuty(motor, 0);
    HAL_TIM_PWM_Start(motor->pwm_tim, motor->pwm_channel);
}

void Motor_Start(Motor_t *motor, float duty)
{
    motor->moving           = 0;
    motor->pulses_count     = 0;
    motor->speed_rpm        = 0;
    motor->stale_ticks      = 0;
    motor->pid.integral     = 0;
    motor->pid.prev_error   = 0;
    Motor_SetDuty(motor, duty);
    HAL_TIM_IC_Start_IT(motor->ic_tim, motor->ic_channel);
}

void Motor_Stop(Motor_t *motor)
{
    Motor_SetDuty(motor, 0.0f);
    motor->speed_rpm = 0;
    HAL_TIM_IC_Stop_IT(motor->ic_tim, motor->ic_channel);
}

void Motor_SetDuty(Motor_t *motor, float duty)
{
	 if (duty <0)    duty = 0;
    if (duty > MOTOR_MAX_DUTY) duty = MOTOR_MAX_DUTY;   /* debug: keep PWM below full scale */
    motor->duty = duty;
    uint32_t arr = motor->pwm_tim->Instance->ARR;
    uint32_t ccr = (uint32_t)((duty / 100.0f) * arr);
    __HAL_TIM_SET_COMPARE(motor->pwm_tim, motor->pwm_channel, ccr);
}

void Motor_SetTargetSpeed(Motor_t *motor, float rpm)
{
    motor->setpoint_rpm = (uint16_t)rpm;
}

void Motor_HandleICInterrupt(Motor_t *motor)
{
    uint32_t capture = HAL_TIM_ReadCapturedValue(motor->ic_tim, motor->ic_channel);

    uint32_t diff = (capture >= motor->last_capture) ?
                    (capture - motor->last_capture) :
                    ((motor->ic_tim->Instance->ARR - motor->last_capture) + capture);

    if (diff > 72)
    {
        diff *= 1000; /* scale back to base-clock ticks (prescaler = 999 ≈ 1000) */
        motor->last_capture = capture;
        motor->period_ticks = diff;

        if (diff > 0 && motor->pulses_count > 0)
        {
            float freq        = (float)TIM_CLOCK / (float)diff;
            motor->speed_rpm  = (uint16_t)(freq * 60.0f);
        }
    }
    motor->pulses_count++;
}

void Motor_UpdatePID(Motor_t *motor, float dt_sec)
{
    /* Hold the last measured speed across ticks that had no pulse; only declare
     * 0 after SPEED_TIMEOUT_TICKS consecutive empty ticks (real stall/stop).
     * This stops the telemetry from dropping to 0 between pulses at low RPM. */
    if (motor->moving)
        motor->stale_ticks = 0;
    else if (++motor->stale_ticks >= SPEED_TIMEOUT_TICKS)
        motor->speed_rpm = 0;

    float error = (float)motor->setpoint_rpm - (float)motor->speed_rpm;

    motor->pid.integral += error * dt_sec;

    float derivative = (error - motor->pid.prev_error) / dt_sec;

    motor->duty += motor->pid.Kp * error
                 + motor->pid.Ki * motor->pid.integral
                 + motor->pid.Kd * derivative;

    if (motor->duty <  motor->min_duty)  motor->duty = motor->min_duty;
    if (motor->duty > MOTOR_MAX_DUTY)    motor->duty = MOTOR_MAX_DUTY;

    Motor_SetDuty(motor, motor->duty);
    motor->pid.prev_error = error;
    motor->moving = 0;
}
