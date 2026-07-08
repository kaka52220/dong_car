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

#endif