#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits>
//#include <conio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
//#include <direct.h>
#include "define.h"
#include "struct.h"
#include "LDPC_Decoder.h"
#include "float.h"
#include "error_frames.h"


/*
 * 用于寻找该校验节点在其相连的变量节点中，所对应的L_v2c中的下标
 * CNnum：第几个校验节点
 * index_in_linkVNs：寻找的变量节点对应该校验节点相连的第几个
 */
int index_in_VN(CN *Checknode, int CNnum, int index_in_linkVNS, VN *Variablenode)
{
	for (int i = 0; i < Variablenode[Checknode[CNnum].linkVNs[index_in_linkVNS]].weight; i++)
	{
		if (Variablenode[Checknode[CNnum].linkVNs[index_in_linkVNS]].linkCNs[i] == CNnum)
		{
			return i;
		}
	}
	printf("index_in_VN error\n");
	exit(0);
}

/*
 * 用于寻找该变量节点在其相连的校验节点中，所对应的L_c2v中的下标
 * VNnum：第几个变量节点
 * index_in_linkCNs：寻找的校验节点对应该变量节点相连的第几个
 */
int index_in_CN(VN *Variablenode, int VNnum, int index_in_linkCNS, CN *Checknode)
{
	for (int i = 0; i < Checknode[Variablenode[VNnum].linkCNs[index_in_linkCNS]].weight; i++)
	{
		if (Checknode[Variablenode[VNnum].linkCNs[index_in_linkCNS]].linkVNs[i] == VNnum)
		{
			return i;
		}
	}
	printf("index_in_CN error\n");
	exit(0);
}

/*
 * 用于找到L_v2c中最小和次小的，并计算总的符号相乘
 */
void findmin_submin(CN *Checknode, VN *Variablenode, float &L_min, float &L_submin, int &sign, int row)
{
	L_min = FLT_MAX;
	L_submin = FLT_MAX;
	sign = 1;
	for (int i = 0; i < Checknode[row].weight; i++)
	{
		if (fabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_submin)
		{
			if (fabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]) < L_min)
			{
				L_submin = L_min;
				L_min = fabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
			}
			else
			{
				L_submin = fabs(Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)]);
			}
		}
		if (Variablenode[Checknode[row].linkVNs[i]].L_v2c[index_in_VN(Checknode, row, i, Variablenode)] < 0)
		{
			sign = sign * -1;
		}
	}
}

/*
 * 解调
 */
void Demodulate(LDPCCode *H, Simulation *SIM, AWGNChannel *AWGN, VN *Variablenode, float *Modulate_sym_Channelout)
{
	for (int s = 0; s < H->Variablenode_num; s++)
	{
		Variablenode[s].L_ch = 2 * Modulate_sym_Channelout[s] / (AWGN->sigma * AWGN->sigma);
		if (s < 2 * Z)
		{
			Variablenode[s].L_ch = 0;
		}
	}
}


int Decoding_LNMS(LDPCCode* H, VN* Variablenode, CN* Checknode, int* DecodeOutput)
{
	for (int col = 0; col < H->Variablenode_num; col++)
	{
		for (int d = 0; d < Variablenode[col].weight; d++)
		{
			Variablenode[col].L_v2c[d] = Variablenode[col].L_ch;
		}
		Variablenode[col].LLR = Variablenode[col].L_ch;
	}
	for (int row = 0; row < H->Checknode_num; row++)
	{
		for (int d = 0; d < Checknode[row].weight; d++)
		{
			Checknode[row].L_c2v[d] = 0;
		}
	}

	int iter_number = 0;
	bool decode_correct = true;
	while (iter_number++ < maxIT)
	{

		for (int col = 0; col < H->Variablenode_num; col++)
		{
			if (Variablenode[col].LLR > 0)
			{
				DecodeOutput[col] = 0;
			}
			else
			{
				DecodeOutput[col] = 1;
			}
		}

		decode_correct = true;
		int sum_temp = 0;
		for (int row = 0; row < H->Checknode_num; row++)
		{
			for (int i = 0; i < Checknode[row].weight; i++)
			{
				sum_temp = sum_temp ^ DecodeOutput[Checknode[row].linkVNs[i]];
			}
			if (sum_temp)
			{
				decode_correct = false;
				break;
			}
		}
		if (decode_correct)
		{
			// H->iteraTime = iter_number - 1;
			return iter_number - 1;
		}

		float L_min = 0;
		float L_submin = 0;
		int sign = 1;

		// message from check to var
		for (int row = 0; row < H->Checknode_num; row++)
		{
			for (int dc = 0; dc < Checknode[row].weight; dc++)
			{
				int col = Checknode[row].linkVNs[dc];
				int dv = index_in_VN(Checknode, row, dc, Variablenode);
				Variablenode[col].L_v2c[dv] = Variablenode[col].LLR - Checknode[row].L_c2v[dc];
			}
			// find max and submax
			findmin_submin(Checknode, Variablenode, L_min, L_submin, sign, row);

			for (int dc = 0; dc < Checknode[row].weight; dc++)
			{
				if (fabs(Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)]) != L_min)
				{
					if (Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)] >= 0)
					{
						Checknode[row].L_c2v[dc] = sign * L_min;
					}
					else
					{
						Checknode[row].L_c2v[dc] = -sign * L_min;
					}
				}
				else
				{
					if (Variablenode[Checknode[row].linkVNs[dc]].L_v2c[index_in_VN(Checknode, row, dc, Variablenode)] >= 0)
					{
						Checknode[row].L_c2v[dc] = sign * L_submin;
					}
					else
					{
						Checknode[row].L_c2v[dc] = -sign * L_submin;
					}
				}
					Checknode[row].L_c2v[dc] *= factor_NMS; //修正
			}
			for (int dc = 0; dc < Checknode[row].weight; dc++)
			{
				int col = Checknode[row].linkVNs[dc];
				int dv = index_in_VN(Checknode, row, dc, Variablenode);
				Variablenode[col].LLR = Variablenode[col].L_v2c[dv] + Checknode[row].L_c2v[dc];
			}
		}
	}
	// H->iteraTime = iter_number - 1;
	return iter_number - 1;
}
