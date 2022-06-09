#ifndef __NEURON_H__
#define __NEURON_H__

#include "parameters.h"

typedef struct Neuron
{
    double p;
    double p_th;

    int t_rest;
    int t_ref;

    void (*hyperpolarization)(struct Neuron *, int);
    void (*inhibit)(struct Neuron *, int);
} Neuron;

Neuron *initial(void)
{
    Neuron *n = (struct Neuron *)malloc(sizeof(struct Neuron));
    if (n != 0)
    {
        n->p_th = p_th;
        n->p = p_rest;
        n->t_rest = -1;
        n->t_ref = 15; //(us)
    }
    return n;
}

void destroyNeuron(Neuron *n)
{
    free(n);
}

void reset(struct Neuron *n)
{
    n->p_th = p_th;
    n->p = p_rest;
    n->t_rest = -1;
    n->t_ref = 15; //(us)
}

void hyperpolarization(struct Neuron *n, int t)
{
    n->p = p_hyperpolarization;
    n->t_rest = t + n->t_ref;
}

void inhibit(struct Neuron *n, int t)
{
    n->p = p_inhibit;
    n->t_rest = t + n->t_ref;
}

#endif
