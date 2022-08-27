#ifndef __STDP_H__
#define __STDP_H__

#include "parameters.h"
#include <math.h>

// Reinforcement learning curve
double rl(int t)
{
    if (t > 0)
    {
        return A * (exp(-(double)t / tau));
    }
    else if (t <= 0)
    {
        return A * (exp((double)t / tau));
    }
}

// Update weights
double update(double w, double del_w)
{
    if (del_w < 0.0)
    {
        return w + sigma * del_w * pow((w - fabs(w_min)), mu);
    }
    else if (del_w > 0.0)
    {
        return w + sigma * del_w * pow((w_max - w), mu);
    }
}

#endif
