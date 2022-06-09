#ifndef __STDP_H__
#define __STDP_H__

#include "parameters.h"
#include <math.h>

/*
    This file implements STDP curve and weight update rule
*/

// Reinforcement learning curve
double rl(int t)
{
    if (t > 0)
    {
        double rl_pos = -A_plus * (exp(-(double)(t / tau_plus) - stdp_offset));
        // printf("%d : %2.2f\n", t, rl_pos);
        return rl_pos;
    }
    else
    {
        double rl_neg = A_minus * (exp(-(double)(t / tau_minus) - stdp_offset));
        // printf("%d : %2.2f\n", t, rl_neg);
        return rl_neg;
    }
}

// Update weights
double update(double w, double del_w)
{
    if (del_w < 0.0)
    {
        return w + sigma * del_w * pow(w - abs(w_min), mu);
    }
    else if (del_w > 0.0)
    {
        return w + sigma * del_w * pow(w_max - w, mu);
    }
}

#endif