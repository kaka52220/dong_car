/*
 *小车直流电机控制
 */
#ifndef _ELECTRICAL_MACHINERY_H_
#define _ELECTRICAL_MACHINERY_H_

void car_run(int base_speed, int differential);
void car_stop();
extern int MAX;
extern int MIN;

void MOTOR_CONTROL(int TargetVelocity_A, int TargetVelocity_B, int TargetVelocity_C, int TargetVelocity_D);

/* 速度计算 (mm/s) — 13线编码器, 65mm轮径, 20ms采样 */
float calculate_motor_speed(char motor_name);
void  reset_motor_speed(char motor_name);

#endif