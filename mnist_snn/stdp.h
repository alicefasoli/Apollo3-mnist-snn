#ifndef __STDP_H__
#define __STDP_H__

#include "parameters.h"
#include <math.h>

/*
    This file implements STDP curve and weight update rule
*/

// Reinforcement learning curve
float rl(int t)
{
    if (t > 0)
    {
        return -A_plus * (expf(-(float)t / tau_plus) - stdp_offset);
    }
    else if (t <= 0)
    {
        return A_minus * (expf((float)t / tau_minus) - stdp_offset);
    }
}

// Update weights
float update(float w, float del_w)
{
    if (del_w < 0.0)
    {
        return w + sigma * del_w * powf((w - fabsf(w_min)), mu);
    }
    else if (del_w > 0.0)
    {
        return w + sigma * del_w * powf((w_max - w), mu);
    }
}

#endif
