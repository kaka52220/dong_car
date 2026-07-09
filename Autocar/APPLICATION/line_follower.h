/*
 *此为循迹逻辑
*/
#ifndef _LINE_FOLLOWER_H_
#define _LINE_FOLLOWER_H_

#define IIC_ADDRESS                 0x20
#define REGISTER_WRITE_ADDRESS      0x40
#define REGISTER_READ_ADDRESS       0x41

#include <stdbool.h>
extern bool stop_flag;
void CAR_CONTROL(void);

#endif /* _LINE_FOLLOWER_H_ */