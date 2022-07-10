#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <conio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
//#include <direct.h>
#include "struct.h"
#include "Simulation.h"
#include <thread>
#include <mutex>
#include <vector>
#include <random>

#define Matrixfile "Tanner_PON_LDPC.txt"
#define Export_to_File 0 // 0不输出，1输出
#define Export_error_location 0
#define Export_error_frames 0

#define factor_NMS (0.75)

#define Z 40 //前向打孔

// LDPC译码器相关参数
#define maxIT 30 // LDPC译码器最大迭代次数.其中对Q值赋初值用了一次迭代

#define THREAD_NUM 0

// AWGN参数

#define ix_define 173
#define iy_define 173
#define iz_define 173

#define Add_noise 1 // 0--No; 1--Yes
#define snrtype 0   // 0--Eb/No; 1--Es/No

//仿真参数
#define startSNR 2.5 //目前用作crossover probability了
#define stepSNR 0.1
#define stopSNR 3.6

#define leastErrorFrames 20 // 最少错误帧数
#define leastTestFrames 200 // 最少仿真帧数
#define refreshStep 10
#define displayStep 1000000 // 定义将译码结果写入相应txt文件的频率

// CUDA c相应参数

#define PI (3.1415926)
#define Message_CW 0 // 提前终止和统计时只看信息位还是看整个码字:0->只看信息位;1->看整个码字

#endif