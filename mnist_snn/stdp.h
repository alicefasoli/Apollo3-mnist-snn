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
        // float tau = (float)t / tau_plus;
        return -A_plus * (expf(-(float)t / tau_plus) - stdp_offset);
    }
    else if (t <= 0)
    {
        // float tau = (float)t / tau_minus;
        return A_minus * (expf((float)t / tau_minus) - stdp_offset);
    }
}

// Update weights
float update(float w, float del_w)
{
    float p;
    if (del_w < 0.0)
    {
        p = powf((w - fabsf(w_min)), mu);
        // printf("p [del_w-] : %f ", p);
        return w + sigma * del_w * p;
    }
    else if (del_w > 0.0)
    {
        p = powf((w_max - w), mu);
        // printf("p [del_w+] : %f ", p);
        return w + sigma * del_w * p;
    }

    // to do fix
}

// float rl(int t)
// {
//     if (t <= -2)
//     {
//         float tau = (float)t / tau_minus;
//         return A_minus * (expf(tau));
//     }
//     else if (t > -2 && t < 2)
//     {
//         return 0.0;
//     }
//     else if (t >= 2)
//     {
//         float tau = (float)t / tau_plus;
//         return A_plus * (expf(tau));
//     }
// }

// // Update weights
// float update(float w, float del_w)
// {
//     if (del_w <= 0.0)
//     {
//         return w + sigma * del_w * (w - w_min);
//     }
//     else if (del_w > 0.0)
//     {
//         return w + sigma * del_w * (w_max - w);
//     }
// }

#endif

// rl(1) = -0.654985