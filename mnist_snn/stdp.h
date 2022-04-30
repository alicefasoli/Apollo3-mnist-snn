#ifndef __STDP_H__
#define __STDP_H__

#include <math.h>

/*
    This file implements STDP curve and weight update rule
*/

// STDP Parameters
float w_min = 0.00001;
float w_max = 1;

int stdp_offset = 0;
float sigma = 0.01;
int A_plus = 0.8;
int A_minus = 0.8;
int tau_plus = 5;
int tau_minus = 5;
float mu = 0.9;

int reiforcment_learning_curve(int t){
    if(t > 0){
        return -A_plus*(exp(-float(t)/tau_plus)-stdp_offset);
    }else{
        return A_minus*(exp(-float(t)/tau_minus)-stdp_offset);
    }
}

int weight_update(float w, int del_w){
    if(del_w < 0){
        return w + sigma*del_w*pow(w-abs(w_min), mu);
    }else if (del_w > 0){
        return w + sigma*del_w*pow(w_max-w, mu);
    }
}

#endif