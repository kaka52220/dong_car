/*
 *此为基于u8g2搭建的二级目录
 */

#ifndef _DRAW_H_
#define _DRAW_H_


#include "time.h"
#include "u8g2.h"
#include "stdbool.h"

 //------------BEEP---------------//
#define BEEP(v)  (v ? DL_GPIO_setPins(BEEP_PORT, BEEP_beep_PIN) : DL_GPIO_clearPins(BEEP_PORT, BEEP_beep_PIN))

extern float kp, ki, kd;
extern int8_t base_speed;
extern int8_t differential;
extern bool stop_flag;
extern int8_t number_of_turns;

void Calibration_waiting(u8g2_t *u8g2);
void OLED_SHOW(u8g2_t *u8g2);
#endif