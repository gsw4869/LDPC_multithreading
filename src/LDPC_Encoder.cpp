#include "LDPC_Encoder.h"
#include "struct.h"

std::default_random_engine e(time(0));			   //定义随机数引擎
std::uniform_real_distribution<double> dd(0, 1.0); //浮点型分布

/*
 * BPSK调制
 */
void Modulate(LDPCCode *H, int *Modulate_sym, int *CodeWord_sym)
{
	for (int s = 0; s < H->bit_length; s++)
	{
		Modulate_sym[s] = 1 - 2 * CodeWord_sym[s];
	}
}
/*
 * 二进制对称信道
 * per:翻转概率
 */
void BSCChannel_CPU(LDPCCode *H, AWGNChannel *AWGN, float *Modulate_sym_Channelout, int *Modulate_sym, float per)
{
	for (int index0 = 0; index0 < H->bit_length; index0++)
	{
		// if (dd(e) < per) //
		if (RandomModule(AWGN->seed) < per)
		{
			Modulate_sym_Channelout[index0] = -Modulate_sym[index0];
		}
		else
		{
			Modulate_sym_Channelout[index0] = Modulate_sym[index0];
		}
	}
}
/*
 * CodeWord：原始码组
 * Channel_Out：经过BPSK调制的输出信号
 */
void AWGNChannel_CPU(LDPCCode *H, AWGNChannel *AWGN, float *Modulate_sym_Channelout, int *Modulate_sym)
{
	int index0, len;
	float u1, u2, temp;
	len = H->bit_length;
	for (index0 = 0; index0 < len; index0++)
	{

		u1 = RandomModule(AWGN->seed);
		u2 = RandomModule(AWGN->seed);

		temp = (float)sqrt((float)(-2) * log((float)1 - u1));
		Modulate_sym_Channelout[index0] = (AWGN->sigma) * cos(2 * PI * u2) * temp + Modulate_sym[index0]; //产生高斯白噪声信号(https://www.cnblogs.com/tsingke/p/6194737.html)
																										  // printf("%f ", Modulate_sym_Channelout[index0]);
	}
	// exit(0);
}

float RandomModule(int *seed)
{
	float temp = 0.0;
	seed[0] = (seed[0] * 249) % 61967;
	seed[1] = (seed[1] * 251) % 63443;
	seed[2] = (seed[2] * 252) % 63599;
	temp = (((float)seed[0]) / ((float)61967)) + (((float)seed[1]) / ((float)63443)) + (((float)seed[2]) / ((float)63599));
	temp -= (int)temp;
	return (temp);
}
