/*
 *存放中断函数
 */
#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

typedef enum {
    FORWARD,  // 正向
    REVERSAL  // 反向
} ENCODER_DIR;

typedef struct {
    volatile long long temp_count;  //保存实时计数值
    int count;         				//根据定时器时间更新的计数值
    ENCODER_DIR dir;            	//旋转方向
} ENCODER_RES;

extern int nextway;//-1向上移动,1向下移动,2进入下一目录/启动应用,3返回上一级

int get_encoder_count(char encoder_name);
ENCODER_DIR get_encoder_dir(char encoder_name);
volatile long long get_encoder_temp_count(char encoder_name);

#endif  /* #ifndef _INTERRUPT_H_ */