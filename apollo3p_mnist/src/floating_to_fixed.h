#ifndef __FLOATING_TO_FIXED_H__
#define __FLOATING_TO_FIXED_H__

#include "apollo3p.h"
#include "network_values.h"
#include "parameters.h"
#include <math.h>

#define FIXED_POINT_FRACTIONAL_BITS 5

double fixed_to_double(uint16_t input);
uint16_t double_to_fixed(double input);

inline double fixed_to_double(uint16_t input)
{
    return ((double)input / (double)(1 << FIXED_POINT_FRACTIONAL_BITS));
}

inline uint16_t double_to_fixed(double input)
{
    return (uint16_t)(round(input * (1 << FIXED_POINT_FRACTIONAL_BITS)));
}

void transform_weights(){
		int i, j;
		for (i = 0; i < N_SECOND_LAYER; i++)
    {
			for (j = 0; j < N_FIRST_LAYER; j++)
			{
			}
		}

}

#endif
