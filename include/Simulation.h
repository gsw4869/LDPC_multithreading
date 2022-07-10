#ifndef _SIMULATION_CUH_
#define _SIMULATION_CUH_

#include "define.h"
#include "struct.h"

void Simulation_CPU(LDPCCode *H, AWGNChannel *AWGN, Simulation *SIM, VN *Variablenode_0, CN *Checknode_0, int *Modulate_sym, int *CodeWord_sym, int thread_id);

int Statistic(Simulation *SIM, int *CodeWord_Frames, int *D, LDPCCode *H,int thread_id);

void Get_H(LDPCCode *H, VN *Variablenode, CN *Checknode);

void free_VN_CN(LDPCCode *H, VN *Variablenode, CN *Checknode);

#endif
