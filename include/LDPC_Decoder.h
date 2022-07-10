#ifndef _LDPC_DECODER_CUH
#define _LDPC_DECODER_CUH

#include "define.h"
#include "struct.h"

void Demodulate(LDPCCode *H, Simulation *SIM, AWGNChannel *AWGN, VN *Variablenode, float *Modulate_sym_Channelout);

int Decoding_LNMS(LDPCCode *H, VN *Variablenode, CN *Checknode, int *DecodeOutput);

#endif