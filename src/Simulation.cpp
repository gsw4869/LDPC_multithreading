#include "Simulation.h"
#include "LDPC_Encoder.h"
#include "LDPC_Decoder.h"
#include <assert.h>
#include <fstream>
#include <string>
#include "define.h"

std::mutex mtx;

/*
 * 仿真函数
 * AWGN:AWGNChannel类变量，包含噪声种子等
 *
 */
void Simulation_CPU(LDPCCode *H, AWGNChannel *AWGN1, Simulation *SIM, VN *Variablenode_0, CN *Checknode_0, int *Modulate_sym, int *CodeWord_sym, int thread_id)
{

	float *Modulate_sym_Channelout;

	VN *Variablenode = Variablenode_0 + thread_id * H->Variablenode_num;

	CN *Checknode = Checknode_0 + thread_id * H->Checknode_num;

	Modulate_sym_Channelout = (float *)malloc(H->bit_length * sizeof(float)); //经过信道后

	int *DecodeOutput;
	DecodeOutput = (int *)malloc(H->Variablenode_num * sizeof(int)); //译码输出
	memset(DecodeOutput, 0, H->Variablenode_num * sizeof(int));

	AWGNChannel *AWGN;
	AWGN = (AWGNChannel *)malloc(sizeof(AWGNChannel));
	AWGN->seed[0] = ix_define + thread_id;
	AWGN->seed[1] = iy_define + thread_id;
	AWGN->seed[2] = iz_define + thread_id;
	AWGN->sigma = AWGN1->sigma;

	while (SIM->num_Error_Frames < leastErrorFrames || SIM->num_Frames < leastTestFrames)
	{
		// mtx.lock();

		AWGNChannel_CPU(H, AWGN, Modulate_sym_Channelout, Modulate_sym);

		Demodulate(H, SIM, AWGN, Variablenode, Modulate_sym_Channelout);

		int iter_num = Decoding_LNMS(H, Variablenode, Checknode, DecodeOutput);

		mtx.lock();

		SIM->num_Frames += 1;

		SIM->Total_Iteration += iter_num;

		int error = Statistic(SIM, CodeWord_sym, DecodeOutput, H, thread_id);
		if (Export_error_frames)
		{
			if (error)
			{
				FILE *fp_H;
				if (NULL == (fp_H = fopen("error_frames.txt", "a")))
				{
					printf("can not open file: error_frames.txt\n");
					exit(0);
				}
				fprintf(fp_H, "{");
				for (int i = 0; i < H->Variablenode_num - 1; i++)
				{
					fprintf(fp_H, "%d,", (int)(Variablenode[i].L_ch));
				}

				fprintf(fp_H, "%d}\n", (int)(Variablenode[H->Variablenode_num - 1].L_ch));
				fclose(fp_H);
			}
		}

		mtx.unlock();
	}
	free(AWGN);
	free(DecodeOutput);
	free(Modulate_sym_Channelout);
}

/*
 * 统计函数，统计仿真结果
 */
int Statistic(Simulation *SIM, int *CodeWord_Frames, int *D, LDPCCode *H, int thread_id)
{
	int index1;
	int Error_msgBit = 0;
	int flag = 0;

	for (index1 = 0; index1 < H->Variablenode_num; index1++)
	{
		Error_msgBit = (D[index1] != CodeWord_Frames[index1]) ? Error_msgBit + 1 : Error_msgBit;
		if (D[index1] != CodeWord_Frames[index1])
		{

			if (Export_error_location)
			{
				FILE *fp_H;
				if (NULL == (fp_H = fopen("error_location.txt", "a")))
				{
					printf("can not open file: error_location.txt\n");
					exit(0);
				}
				if (flag == 0)
				{
					fprintf(fp_H, "%.2f\t", SIM->SNR);
				}
				fprintf(fp_H, "%5d,%5d,%5d\t\t\t", index1 + 1, ((index1 + 1) % 256 ? ((index1 + 1) / 256 + 1) : (index1 + 1) / 256), ((index1 + 1) % 256 ? (index1 + 1) % 256 : 256));
				fclose(fp_H);

				flag = 1;
			}
		}
	}

	if (Export_error_location)
	{
		if (Error_msgBit)
		{
			FILE *fp_H;
			if (NULL == (fp_H = fopen("error_location.txt", "a")))
			{
				printf("can not open file: error_location.txt\n");
				exit(0);
			}
			fprintf(fp_H, "\n");
			fclose(fp_H);
		}
	}
	SIM->num_Error_Bits += Error_msgBit;
	SIM->num_Error_Frames = (Error_msgBit != 0) ? SIM->num_Error_Frames + 1 : SIM->num_Error_Frames;

	if (Error_msgBit > 2)
	{
		SIM->num_Error_Bits_1 += Error_msgBit;
		SIM->num_Error_Frames_1 = SIM->num_Error_Frames_1 + 1;
	}
	// SIM->num_Error_Frames = (Error_msgBit!= 0 || D[index0 + CW_Len * Num_Frames_OneTime] == 0) ? SIM->num_Error_Frames + 1 : SIM->num_Error_Frames;
	// SIM->num_Alarm_Frames = (Error_msgBit[index0] == 0 && D[index0 + CW_Len * Num_Frames_OneTime] == 0) ? SIM->num_Alarm_Frames + 1 : SIM->num_Alarm_Frames;
	// SIM->num_False_Frames = (Error_msgBit[index0] != 0 && D[index0 + CW_Len * Num_Frames_OneTime] == 1) ? SIM->num_False_Frames + 1 : SIM->num_False_Frames;
	// SIM->Total_Iteration += H->iteraTime;

	if (SIM->num_Frames % refreshStep == 0)
	{
		if (SIM->num_Error_Frames == 0)
		{
			SIM->BER = ((double)1 / (double)(SIM->num_Frames)) / (double)(H->Variablenode_num);
			SIM->FER = (double)1 / (double)SIM->num_Frames;
		}
		else
		{
			SIM->BER = ((double)SIM->num_Error_Bits / (double)(SIM->num_Frames)) / (double)(H->Variablenode_num);
			SIM->FER = (double)SIM->num_Error_Frames / (double)SIM->num_Frames;
		}

		if (SIM->num_Error_Frames_1 == 0)
		{
			SIM->BER_1 = ((double)1 / (double)(SIM->num_Frames)) / (double)(H->Variablenode_num);
			SIM->FER_1 = (double)1 / (double)SIM->num_Frames;
		}
		else
		{
			SIM->BER_1 = ((double)SIM->num_Error_Bits_1 / (double)(SIM->num_Frames)) / (double)(H->Variablenode_num);
			SIM->FER_1 = (double)SIM->num_Error_Frames_1 / (double)SIM->num_Frames;
		}

		SIM->AverageIT = (double)SIM->Total_Iteration / (double)SIM->num_Frames;
		printf(" %.2f %10ld  %6ld  %6ld  %6.4e  %6.4e  %6.2f  %6ld  %6ld  %6.4e  %6.4e\r", SIM->SNR, SIM->num_Frames, SIM->num_Error_Frames, SIM->num_Error_Bits, SIM->FER, SIM->BER, SIM->AverageIT, SIM->num_Error_Frames_1, SIM->num_Error_Bits_1, SIM->FER_1, SIM->BER_1);
		if ((SIM->num_Frames % displayStep == 0) && Export_to_File)
		{
			FILE *fp_H;
			if (NULL == (fp_H = fopen("results.txt", "a")))
			{
				printf("can not open file: results.txt\n");
				exit(0);
			}
			fprintf(fp_H, " %.2f %10ld  %6ld  %6ld  %6.4e  %6.4e  %6.2f  %6ld  %6ld  %6.4e  %6.4e\n", SIM->SNR, SIM->num_Frames, SIM->num_Error_Frames, SIM->num_Error_Bits, SIM->FER, SIM->BER, SIM->AverageIT, SIM->num_Error_Frames_1, SIM->num_Error_Bits_1, SIM->FER_1, SIM->BER_1);
			fclose(fp_H);
		}
	}

	if (SIM->num_Error_Frames >= leastErrorFrames && SIM->num_Frames >= leastTestFrames)
	{
		SIM->BER = ((double)SIM->num_Error_Bits / (double)(SIM->num_Frames)) / (double)(H->Variablenode_num);
		SIM->FER = (double)SIM->num_Error_Frames / (double)SIM->num_Frames;
		SIM->AverageIT = (double)SIM->Total_Iteration / (double)SIM->num_Frames;
		if (thread_id == 0)
		{
			printf(" %.2f %10ld  %6ld  %6ld  %6.4e  %6.4e  %6.2f  %6ld  %6ld  %6.4e  %6.4e\n", SIM->SNR, SIM->num_Frames, SIM->num_Error_Frames, SIM->num_Error_Bits, SIM->FER, SIM->BER, SIM->AverageIT, SIM->num_Error_Frames_1, SIM->num_Error_Bits_1, SIM->FER_1, SIM->BER_1);
			if (Export_to_File)
			{
				FILE *fp_H;
				if (NULL == (fp_H = fopen("results.txt", "a")))
				{
					printf("can not open file: results.txt\n");
					exit(0);
				}
				fprintf(fp_H, " %.2f %10ld  %6ld  %6ld  %6.4e  %6.4e  %6.2f  %6ld  %6ld  %6.4e  %6.4e\n", SIM->SNR, SIM->num_Frames, SIM->num_Error_Frames, SIM->num_Error_Bits, SIM->FER, SIM->BER, SIM->AverageIT, SIM->num_Error_Frames_1, SIM->num_Error_Bits_1, SIM->FER_1, SIM->BER_1);
				fclose(fp_H);
			}
		}
	}
	return ((Error_msgBit != 0) ? 1 : 0);
}

/*
H:校验矩阵
Weight_Checknode:按顺序记录每个校验节点的重量
Weight_Variablenode:按顺序记录每个变量节点的重量
Address_Variablenode:变量节点相连的校验节点的序号
Address_Checknode:校验节点相连的变量节点的序号
*/
void Get_H(LDPCCode *H, VN *Variablenode, CN *Checknode)
{
	int index1;

	FILE *fp_H;

	if (NULL == (fp_H = fopen(Matrixfile, "r")))
	{
		printf("can not open file: %s\n", Matrixfile);
		exit(0);
	}

	fscanf(fp_H, "%d", &H->Variablenode_num); // 变量节点个数（行数）
	// Variablenode=(VN *)malloc(H->Variablenode_num*sizeof(VN));

	fscanf(fp_H, "%d", &H->Checknode_num); // 校验节点个数（列数）
	// Checknode=(CN *)malloc(H->Checknode_num*sizeof(CN));

	H->rate = (float)(H->Variablenode_num - H->Checknode_num) / H->Variablenode_num;

	H->bit_length = H->Variablenode_num;

	fscanf(fp_H, "%d", &H->maxWeight_variablenode); //变量节点相连的校验节点的个数

	fscanf(fp_H, "%d", &H->maxWeight_checknode); //校验节点相连的变量节点的个数
	int threadNum =
		THREAD_NUM ? THREAD_NUM : std::thread::hardware_concurrency();

	for (int i = 0; i < H->Variablenode_num; i++)
	{
		fscanf(fp_H, "%d", &index1);

		for (int j = 0; j < threadNum; j++)
		{
			Variablenode[j * H->Variablenode_num + i].weight = index1;
			Variablenode[j * H->Variablenode_num + i].linkCNs = (int *)malloc(Variablenode[i].weight * sizeof(int));   //每个变量节点相连的校验节点
			Variablenode[j * H->Variablenode_num + i].L_v2c = (float *)malloc(Variablenode[i].weight * sizeof(float)); //变量节点传给各个相连的校验节点的值
		}
	}

	for (int i = 0; i < H->Checknode_num; i++)
	{
		fscanf(fp_H, "%d", &index1);

		for (int j = 0; j < threadNum; j++)
		{
			Checknode[j * H->Checknode_num + i].weight = index1;
			Checknode[j * H->Checknode_num + i].linkVNs = (int *)malloc(Checknode[i].weight * sizeof(int));	  //每个校验节点相连的变量节点
			Checknode[j * H->Checknode_num + i].L_c2v = (float *)malloc(Checknode[i].weight * sizeof(float)); //校验节点传给各个相连的变量节点的值
		}
	}

	for (int i = 0; i < H->Variablenode_num; i++)
	{
		for (int j = 0; j < Variablenode[i].weight; j++)
		{
			fscanf(fp_H, "%d", &index1);
			for (int t = 0; t < threadNum; t++)
			{
				Variablenode[t * H->Variablenode_num + i].linkCNs[j] = index1 - 1;
			}
		}
	}

	for (int i = 0; i < H->Checknode_num; i++)
	{
		for (int j = 0; j < Checknode[i].weight; j++)
		{
			fscanf(fp_H, "%d", &index1);
			for (int t = 0; t < threadNum; t++)
			{
				Checknode[t * H->Checknode_num + i].linkVNs[j] = index1 - 1;
			}
		}
	}

	fclose(fp_H);
}

void free_VN_CN(LDPCCode *H, VN *Variablenode, CN *Checknode)
{
	int threadNum =
		THREAD_NUM ? THREAD_NUM : std::thread::hardware_concurrency();
	for (int i = 0; i < H->Variablenode_num * threadNum; i++)
	{
		free(Variablenode[i].linkCNs);
		free(Variablenode[i].L_v2c);
	}

	for (int i = 0; i < H->Checknode_num * threadNum; i++)
	{
		free(Checknode[i].linkVNs);
		free(Checknode[i].L_c2v);
	}
}