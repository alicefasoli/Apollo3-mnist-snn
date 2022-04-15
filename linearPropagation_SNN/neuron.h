#ifndef NEURON_H
#define NEURON_H

typedef struct Neuron{
    float dt; // simulation time step
	float t_rest; // initial refractory time

    float *Vm; // Neuron potential (mV) list
    int *time; //Time duration for the neuron list
    float *spike; // Output spikes for the neuron list
	
    float t; // Neuron time step
    float Rm; // Resistance (kOhm)
    float Cm; // Capacitance (uF)
    float tau_m; // Time constant
    float tau_ref; //Refractory period
    float Vth; // Spike threshold
    float V_spike; // Spike delta (V)

    void (*spikeGenerator)(float*, struct Neuron*);
}Neuron;

Neuron *neuron_Create(void);
// void populateDefaultList(Neuron*, int);

void neuron_Destroy(Neuron *);

void printNeuron(Neuron *);

void spikeGenerator(float*, int, Neuron*);

#endif
