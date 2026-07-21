#include "draw.h"
#include "electrical_machinery.h"
#include "clock.h"
#include "stdio.h"
#include "interrupt.h"
#include "ti_msp_dl_config.h"
#include "line_follower.h"
//#include "uart.h"    /* 项目中无此文件，已注释 */
#include "mpu6050/mpu6050.h"
#include "mpu6050/mpu6050_service.h"

//目录级数
#define DIRECTORY 2

bool   stop_flag = true;

float pid_rate = 0.1;
float kp = 18.0;
float ki = 0.2;
float kd = 0.5;

int8_t id[DIRECTORY][2];
int8_t directory_flag = 0;//当前处于几级目录:-2为二级控制,-1为二级显示,0为主目录,1为二级目录
uint8_t base_speed = 30;   /* 速度百分比 0-100, 默认50% */
int8_t differential = 0;
uint8_t number_of_turns = 1;
uint8_t xspeed = 50;      /* 速度百分比 0-100, 默认50% */
//存储目录信息
char*** directory[]=
{
    (char**[]){//一级目录
        (char*[]){"速度控制","限速","圈数","PID","速度PI","MPU",NULL}
    },
    (char**[]){//二级目录
        //一级目录下目录一对应二级目录
        (char*[]){"ON","OFF","speed+","speed-","trun+","trun-",NULL},
        (char*[]){"MIN+","MIN-","MAX+","MAX-",NULL},
        (char*[]){"0","1","2","3","4","5","!1!",NULL},
        (char*[]){"rate+","rate-","kp+","kp-","ki+","ki-","kd+","kd-",NULL},
        (char*[]){"kp+","kp-","ki+","ki-",NULL},
        (char*[]){NULL}  /* MPU 纯显示，无子项 */
    },
};
//存储每级目录项目数
int8_t* directory_num[]=
{
    //一级目录项目数
    (int8_t[]){6,-1},
    //二级目录项目数
    (int8_t[]){6,4,7,8,4,0,-1}
};

void xianshuc(void){
    switch (id[1][0]+id[1][1]) {
    case 0:
        MIN += 10;
        break;
    case 1:
        MIN -= 10;
        break;
    case 2:
        MAX += 10;
        break;
    case 3:
        MAX -= 10;
        break;
    default:
        break;
    }
}

/*
 * [已注释] Calibration_waiting - 依赖MPU6050的roll/yaw变量
 * 如需恢复，请先加入MPU6050模块
void Calibration_waiting(u8g2_t *u8g2)
{   char oled_buffer[32];
    unsigned long times=1;
    int8_t ij[2]={0,0};
    while(times<=18118){
        u8g2_ClearBuffer(u8g2);
        do{
            u8g2_SetFontMode(u8g2, 0); 
            u8g2_SetFontDirection(u8g2, 0);
            u8g2_SetFont(u8g2, u8g2_font_unifont_st16); 

            u8g2_DrawUTF8(u8g2, 4, 62, "R:");
            u8g2_DrawUTF8(u8g2, 66, 62, "Y:");
            sprintf((char *)oled_buffer, "%-6.1f", roll);
            u8g2_DrawUTF8(u8g2, 20, 62, oled_buffer);
            sprintf((char *)oled_buffer, "%-6.1f", yaw);
            u8g2_DrawUTF8(u8g2, 82, 62, oled_buffer);

            char* kks[4]={"陀螺仪校准中","陀螺仪校准中.","陀螺仪校准中..","陀螺仪校准中..."};
            u8g2_DrawUTF8(u8g2, 6, 20, kks[ij[1]/5%4]);
            ij[1]+=1;
            u8g2_DrawBox(u8g2,0,32,ij[0],15);
            if(ij[1]%2==0)ij[0]+=2;
        }while (u8g2_NextPage(u8g2));
        mspm0_get_clock_ms(&times);
    }
    BEEP(1);
    delay_ms(1000);
    BEEP(0);
    DL_GPIO_clearPins(BEEP_PORT, BEEP_beep_PIN);
}
*/

void quanshukongzi(void){
    switch (id[1][0]+id[1][1]) {
        case 0:
            number_of_turns = 0;
            break;
        case 1:
            number_of_turns = 1;
            break;
        case 2:
            number_of_turns = 2;
            break;
        case 3:
            number_of_turns = 3;
            break;
        case 4:
            number_of_turns = 4;
            break;
        case 5:
            number_of_turns = 5;
            break;
        case 6:
            number_of_turns = 2;
            break;
        default:
            break;
    }
}

void SPEED_CONTROL(void){
    switch (id[1][0]+id[1][1]) {
        case 0:  /* ON   — 启动 */
            stop_flag = false;
            base_speed = xspeed;
            break;
        case 1:  /* OFF  — 停车 */
            stop_flag = true;
            break;
        case 2:  /* speed+  步长5% */
            xspeed += 5;
            if (xspeed > 100) xspeed = 100;
            break;
        case 3:  /* speed-  步长5% */
            if (xspeed >= 5) xspeed -= 5;
            else xspeed = 0;
            break;
        case 4:  /* trun+  预留 */
            break;
        case 5:  /* trun-  预留 */
            break;
        default:
            break;
    }
}

void PID_CONTROL(void){
    switch (id[1][0]+id[1][1]) {
        case 0:
            pid_rate += 0.1;
            break;
        case 1:
            pid_rate -= 0.1;
            if (pid_rate < 0)pid_rate = 0;
            break;
        case 2:
            kp += pid_rate;
            break;
        case 3:
            kp -= pid_rate;
            if(kp < 0)kp = 0;
            break;
        case 4:
            ki += pid_rate;
            break;
        case 5:
            ki -= pid_rate;
            if(ki < 0)ki = 0;
            break;
        case 6:
            kd += pid_rate;
            break;
        case 7:
            kd -= pid_rate;
            if(kd < 0)kd = 0;
        default:
            break;
    }
}

/*
 * [已注释] other_control - 控制云台(PTZ)和激光模块，项目中无此硬件
 * 如需恢复，请确保UART已配置且printf已重定向
void other_control(void){
    switch (id[1][0]+id[1][1]) {
    case 0:
        printf("PTZ_MODE=0\r\n");
        break;
    case 1:
        printf("PTZ_MODE=1\r\n");
        break;
    case 2:
        printf("PTZ_MODE=2\r\n");
        break;
    case 3:
        printf("LASER_MODE=0\r\n");
        break;
    case 4:
        printf("LASER_MODE=1\r\n");
        break;
    case 5:
        printf("LASER_MODE=2\r\n");
        break;
    default:
        break;
    }
}
*/

void VELOCITY_PI_CONTROL(void){
    switch (id[1][0]+id[1][1]) {
        case 0:
            Velcity_Kp += 0.1f;
            break;
        case 1:
            Velcity_Kp -= 0.1f;
            if (Velcity_Kp < 0) Velcity_Kp = 0;
            break;
        case 2:
            Velcity_Ki += 0.1f;
            break;
        case 3:
            Velcity_Ki -= 0.1f;
            if (Velcity_Ki < 0) Velcity_Ki = 0;
            break;
        default:
            break;
    }
}

void HOME_directory(u8g2_t *u8g2) {
    int lasttopid,lastnowid;
     //二级控制
    if(directory_flag == 2 
    && (id[0][0]+id[0][1] == 0 
    || id[0][0]+id[0][1] == 1 
    || id[0][0]+id[0][1] == 2
    || id[0][0]+id[0][1] == 3
    || id[0][0]+id[0][1] == 4
    || id[0][0]+id[0][1] == 5
    || id[0][0]+id[0][1] == 6)){
        if(id[0][0]+id[0][1]==0)SPEED_CONTROL();
        if(id[0][0]+id[0][1]==1)xianshuc();
        if(id[0][0]+id[0][1]==2)quanshukongzi();
        if(id[0][0]+id[0][1]==3)PID_CONTROL();
        if(id[0][0]+id[0][1]==4)VELOCITY_PI_CONTROL();
        if(id[0][0]+id[0][1]==5);  /* MPU: 纯显示，不执行任何操作 */
        directory_flag = 1;
    }
    else if(directory_flag == 2)directory_flag = 1;
    if(directory_flag <= 0){
        lasttopid=0;
        lastnowid=0;
    }
    else{
        lasttopid = id[directory_flag-1][0];
        lastnowid = id[directory_flag-1][1];
    }
    
    // 设置字体
    u8g2_SetFontMode(u8g2, 1);
    if(directory_num[directory_flag][lasttopid+lastnowid]!=0)u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_unifont_st16);
    
    char oled_buffer[32];
    if(directory_flag == 1)
    {
        if(id[0][0]+id[0][1]==0){
            u8g2_DrawUTF8(u8g2, 55, 30, "speed:");
            sprintf((char *)oled_buffer, "%d%%", xspeed);
            u8g2_DrawUTF8(u8g2, 100, 30, oled_buffer);
        }
        if(id[0][0]+id[0][1]==1){
            u8g2_DrawUTF8(u8g2, 60, 30, "MIN:");
            sprintf((char *)oled_buffer, "%d", MIN);
            u8g2_DrawUTF8(u8g2, 90, 30, oled_buffer);
            u8g2_DrawUTF8(u8g2, 60, 46, "MAX:");
            sprintf((char *)oled_buffer, "%d", MAX);
            u8g2_DrawUTF8(u8g2, 90, 46, oled_buffer);
        }
        if(id[0][0]+id[0][1]==2){
            u8g2_DrawUTF8(u8g2, 80, 30, "quanshu:");
            sprintf((char *)oled_buffer, "%d", number_of_turns);
            u8g2_DrawUTF8(u8g2, 100, 46, oled_buffer);
        }
        if(id[0][0]+id[0][1]==3){
            u8g2_DrawUTF8(u8g2, 65, 14, "rate:");
            sprintf((char *)oled_buffer, "%.1f", pid_rate);
            u8g2_DrawUTF8(u8g2, 100, 14, oled_buffer);
            u8g2_DrawUTF8(u8g2, 75, 30, "kp:");
            sprintf((char *)oled_buffer, "%.1f", kp);
            u8g2_DrawUTF8(u8g2, 100, 30, oled_buffer);
            u8g2_DrawUTF8(u8g2, 75, 46, "ki:");
            sprintf((char *)oled_buffer, "%.1f", ki);
            u8g2_DrawUTF8(u8g2, 100, 46, oled_buffer);
            u8g2_DrawUTF8(u8g2, 75, 62, "kd:");
            sprintf((char *)oled_buffer, "%.1f", kd);
            u8g2_DrawUTF8(u8g2, 100, 62, oled_buffer);
        }
        if(id[0][0]+id[0][1]==4){
            u8g2_DrawUTF8(u8g2, 40, 30, "Kp:");
            sprintf((char *)oled_buffer, "%.1f", Velcity_Kp);
            u8g2_DrawUTF8(u8g2, 70, 30, oled_buffer);
            u8g2_DrawUTF8(u8g2, 40, 46, "Ki:");
            sprintf((char *)oled_buffer, "%.1f", Velcity_Ki);
            u8g2_DrawUTF8(u8g2, 70, 46, oled_buffer);
        }
        if(id[0][0]+id[0][1]==5){
            u8g2_DrawUTF8(u8g2, 0, 30, "R:");
            sprintf((char *)oled_buffer, "%.1f", mpu6050_get_roll());
            u8g2_DrawUTF8(u8g2, 20, 30, oled_buffer);
            u8g2_DrawUTF8(u8g2, 0, 46, "P:");
            sprintf((char *)oled_buffer, "%.1f", mpu6050_get_pitch());
            u8g2_DrawUTF8(u8g2, 20, 46, oled_buffer);
            u8g2_DrawUTF8(u8g2, 0, 62, "Y:");
            sprintf((char *)oled_buffer, "%.1f", mpu6050_get_yaw());
            u8g2_DrawUTF8(u8g2, 20, 62, oled_buffer);
        }
    }

    // 动态渲染菜单项（根据滚动位置调整）
    u8g2_SetFont(u8g2, u8g2_font_unifont_st16);
    if(directory_flag==0)u8g2_DrawUTF8(u8g2, 36, 14, "目录");
    if(directory_flag==1)u8g2_DrawUTF8(u8g2, 30, 14, directory[0][0][lasttopid+lastnowid]);
    u8g2_SetFont(u8g2, u8g2_font_unifont_st16);
    if(directory_num[directory_flag][lasttopid+lastnowid] > 0)
    {
        for(int8_t i = id[directory_flag][0]; i < id[directory_flag][0]+3; i++)
        {
            u8g2_DrawUTF8(u8g2, 0, 30 + 16 * i  - id[directory_flag][0] * 16, directory[directory_flag][lasttopid+lastnowid][i]);
        }

        // 选中框
        u8g2_SetDrawColor(u8g2, 2);
        u8g2_DrawBox(u8g2, 0, 16 + id[directory_flag][1] * 16, 128, 16); 
        u8g2_SetDrawColor(u8g2, 1);
    }
    
    
    // 滚动逻辑
    if(nextway==1||nextway==-1){
        if(directory_num[directory_flag][lasttopid+lastnowid]<3 && nextway== 1 && (id[directory_flag][1]==directory_num[directory_flag][lasttopid+lastnowid]-1))id[directory_flag][1]=-1;
        if(nextway== 1 && (id[directory_flag][1]==2) && ((id[directory_flag][0])+2<=directory_num[directory_flag][lasttopid+lastnowid]-2)) (id[directory_flag][0])++;
        else if(nextway== 1 && (id[directory_flag][1]==2) && ((id[directory_flag][0])+2==directory_num[directory_flag][lasttopid+lastnowid]-1)){
            id[directory_flag][0]=0;
            id[directory_flag][1]=-1;
        }
        if(nextway==-1 && (id[directory_flag][1]==0) && ((id[directory_flag][0])>0)) (id[directory_flag][0])--;
        else if(nextway==-1 && (id[directory_flag][1]==0) && ((id[directory_flag][0])==0)){
            if(directory_num[directory_flag][lasttopid+lastnowid]>=3){id[directory_flag][1]=3;id[directory_flag][0]=directory_num[directory_flag][lasttopid+lastnowid]-3;}
            else {id[directory_flag][1]=directory_num[directory_flag][lasttopid+lastnowid];}
        }
        if((id[directory_flag][1]) + nextway < 3 && (id[directory_flag][1]) + nextway < directory_num[directory_flag][lasttopid+lastnowid]  && (id[directory_flag][1]) + nextway >= 0)(id[directory_flag][1]) += nextway;
    }
    if(nextway==2&&directory_flag<DIRECTORY)directory_flag+=1;
    if(nextway==3&&directory_flag!=0){
        id[directory_flag][0]=0;
        id[directory_flag][1]=0;
        directory_flag-=1;
    }  
    nextway = 0;  // 重置方向 
}

void OLED_SHOW(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);  // 清空显示缓冲区
    u8g2_FirstPage(u8g2);   // 开始分页渲染
    do {
        HOME_directory(u8g2);
    } while(u8g2_NextPage(u8g2)); // 分页渲染完成
}