/*
 *此为循迹逻辑
*/
#ifndef _LINE_FOLLOWER_H_
#define _LINE_FOLLOWER_H_

#define IIC_ADDRESS                 0x20  /* PCF8575 7位地址，A0=A1=A2=GND */
#define REGISTER_WRITE_ADDRESS      0x40
#define REGISTER_READ_ADDRESS       0x41
#include <stdint.h>
#include <stdbool.h>
extern bool stop_flag;
extern uint8_t s1, s2, s3, s4, s5, s6, s7, s8;
// extern unsigned char IRbuf[2];  /* 旧 I2C 模块，已废弃 */
void line_follower_update(void);
void CAR_CONTROL(void);
void dayin(uint8_t *test);
// int i2c_scan(void);  /* I2C 扫描，调试用 */
#endif /* _LINE_FOLLOWER_H_ */