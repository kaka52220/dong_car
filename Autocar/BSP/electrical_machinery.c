#include "electrical_machinery.h"
#include "ti_msp_dl_config.h"
#include "usart.h"
//#include "draw.h"
#include "interrupt.h"
#include "clock.h"
#include "stdio.h"

int MAX=0;
int MIN=0;

float Velcity_Kp=0.5,  Velcity_Ki=0.3,  Velcity_Kd;

/*===================================================================
 *  速度计算 (mm/s) — 13线编码器 + 65mm轮径, 20ms采样
 *
 *  编码器 ISR: A相+B相都触发 → 每圈 26 中断
 *  减速比 1:28 → 车轮每圈 = 26×28 = 728 中断
 *  瞬时速度 = |脉冲数| × (π×65/728) × 50Hz = |脉冲数| × 14.03 mm/s
 *  取绝对值: 方向由 MOTOR_CONTROL 的 IN1/IN2 控制, 速度只算大小
 *  一阶低通滤波: speed = α×instant + (1-α)×speed
 *===================================================================*/
#define SPEED_FILTER_ALPHA   0.5f
#define PULSE_TO_MMS         14.03f  /* π×65/(13×2×28)×50 */

static float motor_speed_A, motor_speed_B, motor_speed_C, motor_speed_D;

float calculate_motor_speed(char motor_name)
{
    int   pulse   = get_encoder_count(motor_name);
    /* 取绝对值: 编码器ISR用±计数表示方向, 速度只看大小 */
    int   mag     = (pulse >= 0) ? pulse : -pulse;
    float instant = (float)mag * PULSE_TO_MMS;
    float *sp     = NULL;

    switch (motor_name) {
    case 'A': sp = &motor_speed_A; break;
    case 'B': sp = &motor_speed_B; break;
    case 'C': sp = &motor_speed_C; break;
    case 'D': sp = &motor_speed_D; break;
    default:  return 0;
    }
    *sp = SPEED_FILTER_ALPHA * instant + (1.0f - SPEED_FILTER_ALPHA) * (*sp);
    return *sp;
}

void reset_motor_speed(char motor_name)
{
    switch (motor_name) {
    case 'A': motor_speed_A = 0; break;
    case 'B': motor_speed_B = 0; break;
    case 'C': motor_speed_C = 0; break;
    case 'D': motor_speed_D = 0; break;
    }
}

int Velocity_A(int TargetVelocity, int CurrentVelocity)
{  
    int Bias;  //定义相关变量
	static int ControlVelocity, Last_bias; //静态变量，函数调用结束后其值依然存在
	
	Bias=TargetVelocity-CurrentVelocity; //求速度偏差
	
	ControlVelocity+=Velcity_Kp*(Bias-Last_bias)+Velcity_Ki*Bias;  
	if(ControlVelocity>999)ControlVelocity=999;
	if(ControlVelocity<-999)ControlVelocity=-999;
	Last_bias=Bias;	
	return ControlVelocity; //返回速度控制值
}

int Velocity_B(int TargetVelocity, int CurrentVelocity)
{  
    int Bias;  //定义相关变量
	static int ControlVelocity, Last_bias; //静态变量，函数调用结束后其值依然存在
	
	Bias=TargetVelocity-CurrentVelocity; //求速度偏差
	
	ControlVelocity+=Velcity_Kp*(Bias-Last_bias)+Velcity_Ki*Bias;  
	if(ControlVelocity>999)ControlVelocity=999;
	if(ControlVelocity<-999)ControlVelocity=-999;
	Last_bias=Bias;	
	return ControlVelocity; //返回速度控制值
}

int Velocity_C(int TargetVelocity, int CurrentVelocity)
{  
    int Bias;  //定义相关变量
	static int ControlVelocity, Last_bias; //静态变量，函数调用结束后其值依然存在
	
	Bias=TargetVelocity-CurrentVelocity; //求速度偏差
	
	ControlVelocity+=Velcity_Kp*(Bias-Last_bias)+Velcity_Ki*Bias;
	if(ControlVelocity>999)ControlVelocity=999;
	if(ControlVelocity<-999)ControlVelocity=-999;
	Last_bias=Bias;	
	return ControlVelocity; //返回速度控制值
}

int Velocity_D(int TargetVelocity, int CurrentVelocity)
{  
    int Bias;  //定义相关变量
	static int ControlVelocity, Last_bias; //静态变量，函数调用结束后其值依然存在
	
	Bias=TargetVelocity-CurrentVelocity; //求速度偏差
	
	ControlVelocity+=Velcity_Kp*(Bias-Last_bias)+Velcity_Ki*Bias;
	if(ControlVelocity>999)ControlVelocity=999;
	if(ControlVelocity<-999)ControlVelocity=-999;
	Last_bias=Bias;	
	return ControlVelocity; //返回速度控制值
}

void MOTOR_CONTROL(int TargetVelocity_A, int TargetVelocity_B, int TargetVelocity_C, int TargetVelocity_D)
{
    int ControlVelocity_A = Velocity_A(TargetVelocity_A, (int)calculate_motor_speed('A'));
    int ControlVelocity_B = Velocity_B(TargetVelocity_B, (int)calculate_motor_speed('B'));
    int ControlVelocity_C = Velocity_C(TargetVelocity_C, (int)calculate_motor_speed('C'));
    int ControlVelocity_D = Velocity_D(TargetVelocity_D, (int)calculate_motor_speed('D'));
	// int ControlVelocity_A = TargetVelocity_A;
    // int ControlVelocity_B = TargetVelocity_B;
    // int ControlVelocity_C =  TargetVelocity_C;
    // int ControlVelocity_D = TargetVelocity_D;

		if(ControlVelocity_A > 0)
	{
        DL_GPIO_setPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
    }
    else if (ControlVelocity_A == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
    }
    else if (ControlVelocity_A < 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        ControlVelocity_A = -ControlVelocity_A;
    }

	if(ControlVelocity_B > 0)
	{
        DL_GPIO_setPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
        
    }
    else if (ControlVelocity_B == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
    }
    else if (ControlVelocity_B < 0) 
	{
		DL_GPIO_clearPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
		DL_GPIO_setPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
        ControlVelocity_B = -ControlVelocity_B;
    }

	if(ControlVelocity_C > 0)
	{
        DL_GPIO_setPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
    }
    else if (ControlVelocity_C == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
    }
    else if (ControlVelocity_C < 0) 
	{
		DL_GPIO_clearPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
        ControlVelocity_C = -ControlVelocity_C;
    }

	if(ControlVelocity_D > 0)
	{
        DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
    }
    else if (ControlVelocity_D == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
    }
    else if (ControlVelocity_D < 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        ControlVelocity_D = -ControlVelocity_D;
    }
    // ===== 每 50ms 打印 PID 输出（调试用） =====
    //{//独立作用域
    //    static uint32_t last_print = 0;
    //    if (tick_ms - last_print >= 50)
    //    {
    //        last_print = tick_ms;
    //        char buf[60];
    //        sprintf(buf, "A=%d B=%d C=%d D=%d\r\n",
    //            ControlVelocity_A, ControlVelocity_B,
    //            ControlVelocity_C, ControlVelocity_D);
    //        USART_SendString((unsigned char*)buf);
    //    }
    //}

    DL_TimerG_setCaptureCompareValue(PWMA_INST, ControlVelocity_A, GPIO_PWMA_C0_IDX);
	DL_TimerG_setCaptureCompareValue(PWMB_INST, ControlVelocity_B, GPIO_PWMB_C1_IDX);
	DL_TimerG_setCaptureCompareValue(PWMC_INST, ControlVelocity_C, GPIO_PWMC_C1_IDX);
	DL_TimerG_setCaptureCompareValue(PWMD_INST, ControlVelocity_D, GPIO_PWMD_C1_IDX);
}

void car_run(int base_speed_pct, int differential)//diff > 0 左转
{
	int base_speed = base_speed_pct * 4;   /* 0-100% → 0-400mm/s */
	int left_speed = base_speed - differential / 2;
	int right_speed = base_speed + differential / 2;
	if(left_speed > 400)left_speed = 400;
	else if(left_speed < -400)left_speed = -400;
	if(right_speed > 400)right_speed = 400;
	else if(right_speed < -400)right_speed = -400;
    MOTOR_CONTROL(right_speed, right_speed,
                        left_speed, left_speed);//pi增量控制,ControlVelocity_A = Velocity_A(right_speed, get_encoder_count('A'));
}
void car_stop()
{
	car_run(0, 0);
}