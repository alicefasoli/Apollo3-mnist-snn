#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#define N_FIRST_LAYER 784
#define N_SECOND_LAYER 800
#define PIXEL 28

#include <stdio.h>
#include <stdlib.h>

typedef enum
{
    False,
    True
} boolean;

/*
    This file contains all the parameters of the network
*/

// Simulation parameters
int t = 350;
int t_back = -5;
int epoch = 1;

// Input parameters
float p_rest = -70.0;

// Neuron parameters
float p_inhibit = -100.0;
float p_th = -55.0;
float p_hyperpolarization = -90.0;
float p_drop = 0.8;
float p_th_drop = 0.4;

double w_min = 0.00001;
double w_max = 1.0; // np.max(synapse_init)

// STDP parameters
double stdp_offset = 0.0;
double sigma = 0.01;
double A_plus = 0.8;
// double A_plus = 0.6;
double A_minus = 0.8;
// double A_minus = 0.3;
double tau_plus = 5.0;
// double tau_plus = 8.0;
double tau_minus = 5.0;
// double tau_minus = 3.0;
double mu = 0.9;

// Receptive field parameters
float w[5][5] = {{-0.5, -0.125, 0.25, -0.125, -0.25},
                 {-0.125, 0.25, 0.625, 0.25, -0.125},
                 {0.25, 0.625, 1.0, 0.625, 0.25},
                 {-0.125, 0.25, 0.625, 0.25, -0.125},
                 {-0.5, -0.125, 0.25, -0.125, -0.5}};

float ran[5] = {-2.0, -1.0, 0.0, 1.0, 2.0};
int ox = 2;
int oy = 2;

#endif