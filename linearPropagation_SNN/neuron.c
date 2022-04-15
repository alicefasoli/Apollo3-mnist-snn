#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "neuron.h"
#include "list.h"

Neuron *neuron_Create(void)
{
    Neuron *n = (struct Neuron*) malloc(sizeof(struct Neuron));
    if(n != 0){
        n->dt = 0.0125; // simulation time step
        n->t_rest = 0.0; // initial refractory time

        n->Vm = (float*) calloc(100, sizeof(float));
        n->time = (int*) calloc(100, sizeof(int));
        n->spike = (float*) calloc(100, sizeof(float));

        n->t = 0.0; // Neuron time step
        n->Rm = 1.0; // Resistance (kOhm)
        n->Cm = 10.0; // Capacitance (uF)
        n->tau_m = n->Rm * n->Cm; // Time constant (R*Cm)
        n->tau_ref = 4.0; //Refractory period
        n->Vth = 0.75;  //Spike threshold
        n->V_spike = 1.0; // Spike delta (V)
    }
    return n;
}

void neuron_Destroy(Neuron *n)
{
    
    free(n->Vm);
    free(n->time);
    free(n->spike);
    free(n);
}

void printNeuron(Neuron *n)
{
    struct Neuron *ptr;
    printf("[ ");

    printf("(%2.4f, %2.3f, %2.3f, %2.3f, %2.3f, %2.3f, %2.3f, %2.3f, %2.3f)", 
            n->dt, n->t_rest, n->t, n->Rm, n->Cm,
            n->tau_m, n->tau_ref, n->Vth, n->V_spike);

    printf(" ]");
}

void spikeGenerator(float *neuron_input, int duration, Neuron *neuron)
{
    free(neuron->Vm);
    free(neuron->time);
    free(neuron->spike);

    neuron->Vm = (float*) calloc(duration, sizeof(float));
    neuron->time = (int*) calloc(duration, sizeof(int));
    neuron->spike = (float*) calloc(duration, sizeof(float));

    float *Vm = (float*) calloc(duration, sizeof(float)); 
    float *spikes = (float*) calloc(duration, sizeof(float)); 

    int time[duration];
    for(int i = neuron->t; i < (neuron->t + duration); i++){
        time[i] = i;
    } 

    Vm[duration-1] = neuron->Vm[duration-1]; 

    for(int i = 0 ; i < duration; i++){
        if(neuron->t > neuron->t_rest){
            Vm[i] = Vm[i-1] + (-Vm[i-1] + neuron_input[i-1] *neuron->Rm)/ neuron->tau_m * neuron->dt;
            if(Vm[i] >= neuron->Vth){
                spikes[i] = spikes[i] + neuron->V_spike;
                neuron->t_rest = neuron->t + neuron->tau_ref;
            }
        }
        neuron->t = neuron->t + neuron->dt;
    }

    // Save values
    for(int i = 0; i < duration; i++){
        neuron->Vm[i] = Vm[i];
        neuron->spike[i] = spikes[i];
        neuron->time[i] = time[i];
    }

    free(Vm);
    free(spikes);
}
