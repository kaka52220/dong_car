#include "draw.h"
#include "electrical_machinery.h"
#include "clock.h"
#include "stdio.h"
#include "interrupt.h"
#include "ti_msp_dl_config.h"
#include "line_follower.h"
#include "uart.h"
#include "mpu6050.h"

//目录级数
#define DIRECTORY 2

int id[DIRECTORY][2];
int directory_flag = 0;//当前处于几级目录:-2为二级控制,-1为二级显示,0为主目录,1为二级目录

int car_speed = 0;
bool stop_flag = true;
int now_speed = 0;
int quanshu = 1;

//存储目录信息
char*** directory[]=  
{
    (char**[]){//一级目录
        (char*[]){"速度控制","限速","圈数","PID","中边",NULL}
    },
    (char**[]){//二级目录
        //一级目录下目录一对应二级目录
        (char*[]){"speed+","speed-","ON","OFF",NULL},
        (char*[]){"MIN+","MIN-","MAX+","MAX-",NULL},
        (char*[]){"0","1","2","3","4","5","!1!",NULL},
        (char*[]){"P+","P-","I+","I-","D+","D-",NULL},
        (char*[]){"z1+","z1-","z2+","z2-","z3+","z3-","z4+","z4-",NULL}
    },
};
//存储每级目录项目数
int* directory_num[]=
{
    //一级目录项目数
    (int[]){5,-1},
    //二级目录项目数
    (int[]){4,4,7,6,8,-1}
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

void Calibration_waiting(u8g2_t *u8g2)
{   char oled_buffer[32];
    unsigned long times=1;
    int ij[2]={0,0};
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
    DL_GPIO_setPins(BEEP_PORT, BEEP_PIN_21_PIN);
    mspm0_delay_ms(1000);
    DL_GPIO_clearPins(BEEP_PORT, BEEP_PIN_21_PIN);
}

void quanshukongzi(void){
    l_num = 0;
    r_num = 0;
    switch (id[1][0]+id[1][1]) {
    case 0:
        quanshu = 0;
        break;
    case 1:
        quanshu = 1;
        break;
    case 2:
        quanshu =2;
        break;
    case 3:
        quanshu = 3;
        break;
    case 4:
        quanshu = 4;
        break;
    case 5:
        quanshu =5;
        break;
    case 6:
        quanshu = 1;
        printf("no1isrun");
        break;
    default:
        break;
    }
}

int xspeed = 180;

void CAR_control(void){
    switch (id[1][0]+id[1][1]) {
    case 0:
        xspeed += 10;
        break;
    case 1:
        xspeed -= 10;
        break;
    case 2:
        stop_flag = false;
        car_speed = xspeed;
        now_speed = car_speed;
        car_begin_yaw = yaw;
        break;
    case 3:
        stop_flag = true;
        break;
    default:
        break;
    }
}

void PID_control(void){
    switch (id[1][0]+id[1][1]) {
    case 0:
        P += 1;
        break;
    case 1:
        P -= 1;
        break;
    case 2:
        I += 1;
        break;
    case 3:
        I -= 1;
        break;
    case 4:
        D += 1;
        break;
    case 5:
        D -= 1;
        break;
    default:
        break;
    }
}

void zb_control(void){
    switch (id[1][0]+id[1][1]) {
    case 0:
        correction_factor1 += 0.1;
        break;
    case 1:
        correction_factor1 -= 0.1;
        break;
    case 2:
        correction_factor2 += 0.1;
        break;
    case 3:
        correction_factor2 -= 0.1;
        break;
    case 4:
        correction_factor3 += 0.1;
        break;
    case 5:
        correction_factor3 -= 0.1;
        break;
    case 6:
        correction_factor4 += 0.1;
        break;
    case 7:
        correction_factor4 -= 0.1;
        break;
    default:
        break;
    }
}

void HOME_directory(u8g2_t *u8g2) {
    int lasttopid,lastnowid;
     //二级控制
    if(directory_flag == 2 && (id[0][0]+id[0][1] == 0 || id[0][0]+id[0][1] == 1 || id[0][0]+id[0][1] == 2||id[0][0]+id[0][1]==3||id[0][0]+id[0][1]==4)){
        if(id[0][0]+id[0][1]==0)CAR_control();
        if(id[0][0]+id[0][1]==1)xianshuc();
        if(id[0][0]+id[0][1]==2)quanshukongzi();
        if(id[0][0]+id[0][1]==3)PID_control();
        if(id[0][0]+id[0][1]==4)zb_control();
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
            u8g2_DrawUTF8(u8g2, 80, 30, "speed:");
            sprintf((char *)oled_buffer, "%d", xspeed);
            u8g2_DrawUTF8(u8g2, 100, 46, oled_buffer);
        }
        if(id[0][0]+id[0][1]==1){
            u8g2_DrawUTF8(u8g2, 60, 30 , "MIN:");
            sprintf((char *)oled_buffer, "%d", MIN);
            u8g2_DrawUTF8(u8g2, 90, 30, oled_buffer);
            u8g2_DrawUTF8(u8g2, 60, 46 , "MAX:");
            sprintf((char *)oled_buffer, "%d", MAX);
            u8g2_DrawUTF8(u8g2, 90, 46, oled_buffer);
        }
        if(id[0][0]+id[0][1]==2){
            u8g2_DrawUTF8(u8g2, 80, 30, "quanshu:");
            sprintf((char *)oled_buffer, "%d", quanshu);
            u8g2_DrawUTF8(u8g2, 100, 46, oled_buffer);
        }
        if(id[0][0]+id[0][1]==3){
            u8g2_DrawUTF8(u8g2, 60, 30 , "P:");
            sprintf((char *)oled_buffer, "%-6.1f", P);
            u8g2_DrawUTF8(u8g2, 90, 30, oled_buffer);
            u8g2_DrawUTF8(u8g2, 60, 46 , "I:");
            sprintf((char *)oled_buffer, "%-6.1f", I);
            u8g2_DrawUTF8(u8g2, 90, 46, oled_buffer);
            u8g2_DrawUTF8(u8g2, 60, 62 , "D:");
            sprintf((char *)oled_buffer, "%-6.1f", D);
            u8g2_DrawUTF8(u8g2, 90, 62, oled_buffer);
        }
        if(id[0][0]+id[0][1]==4){
            u8g2_DrawUTF8(u8g2, 30, 30 , "z1:");
            sprintf((char *)oled_buffer, "%-6.1f", correction_factor1);
            u8g2_DrawUTF8(u8g2, 50, 30, oled_buffer);
            u8g2_DrawUTF8(u8g2, 80, 30 , "z2:");
            sprintf((char *)oled_buffer, "%-6.1f", correction_factor2);
            u8g2_DrawUTF8(u8g2, 100, 30, oled_buffer);
            u8g2_DrawUTF8(u8g2, 30, 46 , "z3:");
            sprintf((char *)oled_buffer, "%-6.1f", correction_factor3);
            u8g2_DrawUTF8(u8g2, 50, 46, oled_buffer);
            u8g2_DrawUTF8(u8g2, 80, 46 , "z4:");
            sprintf((char *)oled_buffer, "%-6.1f", correction_factor4);
            u8g2_DrawUTF8(u8g2, 100, 46, oled_buffer);
        }
    }

    // 动态渲染菜单项（根据滚动位置调整）
    u8g2_SetFont(u8g2, u8g2_font_unifont_st16);
    if(directory_flag==0)u8g2_DrawUTF8(u8g2, 36, 14, "目录");
    if(directory_flag==1)u8g2_DrawUTF8(u8g2, 30, 14, directory[0][0][lasttopid+lastnowid]);
    u8g2_SetFont(u8g2, u8g2_font_unifont_st16);
    for(int i = id[directory_flag][0]; i < id[directory_flag][0]+3; i++)
    {
        u8g2_DrawUTF8(u8g2, 0, 30 + 16 * i  - id[directory_flag][0] * 16, directory[directory_flag][lasttopid+lastnowid][i]);
    }

    // 选中框
    u8g2_SetDrawColor(u8g2, 2);
    u8g2_DrawBox(u8g2, 0, 16 + id[directory_flag][1] * 16, 128, 16); 
    u8g2_SetDrawColor(u8g2, 1);
    
    
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

void DIRECTORY_control(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);  // 清空显示缓冲区
    u8g2_FirstPage(u8g2);   // 开始分页渲染
    do {
        HOME_directory(u8g2);
    } while(u8g2_NextPage(u8g2)); // 分页渲染完成
}