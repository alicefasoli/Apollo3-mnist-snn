#ifndef NEURON_H
#define NEURON_H

typedef struct Neuron{
    float dt;       // Simulation time step
	float t_rest;   // Initial refractory time

    float *Vm;      // Neuron potential (mV) list
    int *time;      // Time duration for the neuron list
    float *spike;   // Output spikes for the neuron list
	
    float t;        // Neuron time step
    float Rm;       // Resistance (kOhm)
    float Cm;       // Capacitance (uF)
    float tau_m;    // Time constant
    float tau_ref;  // Refractory period
    float Vth;      // Spike threshold
    float V_spike;  // Spike delta (V)

    int p;          // Potential
    int p_rest;     // Resting potential: neurons have a small negative electical charge of -70 mV

    void (*spikeGenerator)(float*, struct Neuron*);
}Neuron;

Neuron *createNeuron(void);
void initList(Neuron*, int);

void destroyNeuron(Neuron *);
void printNeuron(Neuron *);

void spikeGenerator(float*, int, Neuron*);

void 

#endif
