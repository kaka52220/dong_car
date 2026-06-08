/*
 *此为基于u8g2搭建的二级目录
 */

#ifndef _DRAW_H_
#define _DRAW_H_


#include "time.h"
#include "u8g2.h"
#include "stdbool.h"


extern int car_speed;
extern int now_speed;
extern bool stop_flag;
extern int quanshu;

void Calibration_waiting(u8g2_t *u8g2);
void DIRECTORY_control(u8g2_t *u8g2);
#endif