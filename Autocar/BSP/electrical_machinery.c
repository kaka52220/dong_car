#include "electrical_machinery.h"
#include "ti_msp_dl_config.h"
//#include "uart.h"
//#include "draw.h"
#include "interrupt.h"

int MAX=0;
int MIN=0;

float Velcity_Kp=7,  Velcity_Ki=5,  Velcity_Kd;

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
    int ControlVelocity_A = Velocity_A(TargetVelocity_A, get_encoder_count('A'));
    int ControlVelocity_B = Velocity_B(TargetVelocity_B, get_encoder_count('B'));
    int ControlVelocity_C = Velocity_C(TargetVelocity_C, get_encoder_count('C'));
    int ControlVelocity_D = Velocity_D(TargetVelocity_D, get_encoder_count('D'));
    
	if(ControlVelocity_A > 0)
	{
        DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
    }
    else if (ControlVelocity_A == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
    }
    else if (ControlVelocity_A < 0) 
	{
        DL_GPIO_setPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
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
        DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
    }
    else if (ControlVelocity_D == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
    }
    else if (ControlVelocity_D < 0) 
	{
        DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
        ControlVelocity_D = -ControlVelocity_D;
    }
    DL_TimerG_setCaptureCompareValue(PWMA_INST, 999-ControlVelocity_A, GPIO_PWMA_C0_IDX);
	DL_TimerG_setCaptureCompareValue(PWMB_INST, 999-ControlVelocity_B, GPIO_PWMB_C1_IDX);
	DL_TimerG_setCaptureCompareValue(PWMC_INST, 999-ControlVelocity_C, GPIO_PWMC_C1_IDX);
	DL_TimerG_setCaptureCompareValue(PWMD_INST, 999-ControlVelocity_D, GPIO_PWMD_C1_IDX);
}

void car_run(int base_speed, int differential)//diff > 0 左转
{
	int left_speed = base_speed - differential / 2;
	int right_speed = base_speed + differential / 2;
	if(left_speed > 100)left_speed = 100;
	else if(left_speed < -100)left_speed = -100;
	if(right_speed > 100)right_speed = 100;
	else if(right_speed < -100)right_speed = -100;
    MOTOR_CONTROL(right_speed, right_speed,
                        left_speed, left_speed);//pi增量控制,ControlVelocity_A = Velocity_A(right_speed, get_encoder_count('A'));
}
void car_stop()
{
	car_run(0, 0);
}