/*
 *小车直流电机控制
 */
#ifndef _ELECTRICAL_MACHINERY_H_
#define _ELECTRICAL_MACHINERY_H_

//void car_run(int base_speed, int differential);
void car_run(int base_speed_pct, float differential);
void car_stop();
extern int MAX;
extern int MIN;
extern float Velcity_Kp;
extern float Velcity_Ki;

void MOTOR_CONTROL(int TargetVelocity_A, int TargetVelocity_B, int TargetVelocity_C, int TargetVelocity_D);
void MOTOR_RAW(int A, int B, int C, int D);  /* 绕过 PI，直接控制 PWM */

/* 速度计算 (mm/s) — 13线编码器, 65mm轮径, 20ms采样 */
float calculate_motor_speed(char motor_name);
void  reset_motor_speed(char motor_name);

/* 重置 4 路 PI 的 static CV 和 Last_bias，防止丢线瞬间积分饱和 */
void Velocity_ResetAll(int seed_target);

#endif