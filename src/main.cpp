#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "LDPC_Decoder.h"
#include "LDPC_Encoder.h"
#include "math.h"
#include "codeword.h"

int main()
{
	AWGNChannel *AWGN;
	AWGN = (AWGNChannel *)malloc(sizeof(AWGNChannel));
	Simulation *SIM;
	SIM = (Simulation *)malloc(sizeof(Simulation));

	CN *Checknode;	  // LDPC码校验节点
	VN *Variablenode; // LDPC码变量节点

	LDPCCode *H;
	H = (LDPCCode *)malloc(sizeof(LDPCCode));

	//	先读取行数和列数,分配空间
	FILE *fp_H;

	if (NULL == (fp_H = fopen(Matrixfile, "r")))
	{
		printf("can not open file: %s\n", Matrixfile);
		exit(0);
	}

	int threadNum =
		THREAD_NUM ? THREAD_NUM : std::thread::hardware_concurrency();

	fscanf(fp_H, "%d", &H->Variablenode_num); // 变量节点个数（行数）
	Variablenode = (VN *)malloc(H->Variablenode_num * threadNum * sizeof(VN));

	fscanf(fp_H, "%d", &H->Checknode_num); // 校验节点个数（列数）
	Checknode = (CN *)malloc(H->Checknode_num * threadNum * sizeof(CN));

	fclose(fp_H);
	//
	Get_H(H, Variablenode, Checknode); //初始化剩下的参数

	int *Modulate_sym;
	Modulate_sym = (int *)malloc(H->bit_length * sizeof(int)); //调制输出

	Modulate(H, Modulate_sym, CodeWord_sym); // BPSK调制

	for (SIM->SNR = startSNR; SIM->SNR <= stopSNR; SIM->SNR += stepSNR) //目前用作crossover probability了
	{
		// AWGN信道用，BSC中没用到
		AWGN->seed[0] = ix_define;
		AWGN->seed[1] = iy_define;
		AWGN->seed[2] = iz_define;
		AWGN->sigma = 0;

		H->rate = (float)(H->Variablenode_num - H->Checknode_num) / (H->Variablenode_num - 2*Z);
		if (snrtype == 0)
		{
			AWGN->sigma = (float)sqrt(0.5 / (H->rate * (pow(10.0, ((SIM->SNR) / 10.0))))); //(float)LDPC->msgLen / LDPC->codewordLen;
		}
		else if (snrtype == 1)
		{
			AWGN->sigma = (float)sqrt(0.5 / (pow(10.0, ((SIM->SNR) / 10.0))));
		}

		SIM->num_Frames = 0; // 重新开始统计
		SIM->num_Error_Frames = 0;
		SIM->num_Error_Bits = 0;
		SIM->Total_Iteration = 0;
		SIM->num_Error_Frames_1 = 0;
		SIM->num_Error_Bits_1 = 0;

		int threadNum =
			THREAD_NUM ? THREAD_NUM : std::thread::hardware_concurrency();
		std::vector<std::thread> threads_(threadNum);

		for (int i = 0; i < threadNum; i++)
		{
			threads_[i] = std::thread(
				Simulation_CPU, H, AWGN, SIM, Variablenode, Checknode, Modulate_sym, CodeWord_sym, i);
		}
		for (int i = 0; i < threadNum; i++)
		{
			threads_[i].join();
		}
		// usleep(1000000);
		for (int i = 0; i < threadNum; i++)
		{
			threads_[i].~thread();
		}

	}

	free_VN_CN(H, Variablenode, Checknode);
	free(AWGN);
	free(SIM);
	free(H);
	free(Checknode);
	free(Variablenode);
	free(Modulate_sym);
	return 0;
}