#ifndef _LDPC_ENCODER_CUH
#define _LDPC_ENCODER_CUH

#include "define.h"
#include "struct.h"

void Modulate(LDPCCode *H, int *Modulate_sym, int *CodeWord_sym);

void BSCChannel_CPU(LDPCCode *H, AWGNChannel *AWGN, float *Modulate_sym_Channelout, int *Modulate_sym, float per);

void AWGNChannel_CPU(LDPCCode *H, AWGNChannel *AWGN, float *Modulate_sym_Channelout, int *Modulate_sym);

float RandomModule(int *seed);

#endif