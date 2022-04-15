#include <stdio.h>
#include <stdlib.h>

#include "neuron.h"
#include "list.h"

#define NUM_LAYER 2
#define NUM_NEURON 2

struct Neuron *neurons[NUM_LAYER][NUM_NEURON]; 

void printMatrix()
{
    for(int i=0; i < NUM_LAYER; i++){
        printf("\tLayer %d : [ ", i);
        for(int j = 0; j < NUM_NEURON; j++){
            if(neurons[i][j] != NULL){
                printf("\n\t\tNeuron %d : ", j);
                printNeuron(neurons[i][j]);
            }
        }
        printf("]\n");
    }
}

void create_neurons()
{
    int layer, neuron;
    for(layer = 0; layer < NUM_LAYER; layer++){
        for(neuron = 0; neuron < NUM_NEURON; neuron++){
            neurons[layer][neuron] = neuron_Create();
        }
    }   
}

void freeNeurons()
{
    for(int i=0; i<NUM_LAYER; i++){
        for(int j=0; j<NUM_NEURON; j++){
            neuron_Destroy(neurons[i][j]);
        }
    }
}

int main()
{
    int T = 50;
    float dt = 0.0125;
    int time = (int)T/dt;
    float input = 1.0;

    float neuron_input[time];
    for(int i=0; i < time; i++){
        neuron_input[i] = input;
    }
    // printf("Neuron input: ");
    // printListFloat(neuron_input);

    create_neurons();
    
    printf("Neurons: \n");
    printMatrix(neurons);

    int stimulus_len = sizeof(neuron_input)/sizeof(float);
    // printf("Stimulus len: %d\n", stimulus_len);

    int layer = 0;
    for(int neuron = 0; neuron < NUM_NEURON; neuron++){
        int offset = rand() % 100;

        float *stimulus = (float*) calloc(stimulus_len, sizeof(float));
        int cont = 0;
        for(int i=offset; i < stimulus_len; i++){
            if(cont < stimulus_len-offset){
                stimulus[i] = neuron_input[cont];
                cont++;
            }
        }
        spikeGenerator(stimulus, stimulus_len, neurons[layer][neuron]);
        free(stimulus);
    }

    // Print Membrane Potential and time neuron[0][0]
    printf("Membrane potential to time of neuron 0/0: \n");
    printListFloat(neurons[0][0]->Vm, stimulus_len);
    // printListInt(neurons[0][0]->time, stimulus_len);

    //Print Spike and time neuron[0][0]
    printf("Output spike to time of neuron 0/0: \n");
    printOnlyWhenFire(neurons[0][0]->spike, stimulus_len);
    // printListFloat(neurons[0][0]->spike, stimulus_len);
    // printListInt(neurons[0][0]->time, stimulus_len);

    // Print Membrane Potential and time neuron[0][1]
    printf("Membrane potential to time of neuron 0/1: \n");
    printListFloat(neurons[0][1]->Vm, stimulus_len);
    // printListInt(neurons[0][1]->time, stimulus_len);

    //Print Spike and time neuron[0][1]
    printf("Output spike to time of neuron 0/1: \n");
    printOnlyWhenFire(neurons[0][1]->spike, stimulus_len);
    // printListFloat(neurons[0][1]->spike, stimulus_len);
    // printListInt(neurons[0][1]->time, stimulus_len);

    layer = 0;
    float *layer_spikes = (float*) calloc(stimulus_len, sizeof(float)); 
    for(int i = 0; i < NUM_NEURON; i++){
        for(int j = 0; j < stimulus_len; j++){
            layer_spikes[j] = layer_spikes[j] + neurons[layer][i]->spike[j];
        }
    }

    // Print layer_spikes
    // printListFloat(layer_spikes, stimulus_len);

    neurons[1][0] = neuron_Create();
    spikeGenerator(layer_spikes, stimulus_len, neurons[1][0]); 

    int start_time = 0;
    int end_time = stimulus_len;
    printf("Neuron[1][0] from %d to %d\n", start_time, end_time);

    printf("Input spikes for neuron 1/0: \n");
    printOnlyWhenFire(layer_spikes, stimulus_len);
    // printListFloat(layer_spikes, stimulus_len);

    // Print Membrane Potential and time neuron[1][0]
    printf("Membrane Potential for neuron 1/0: \n");
    printListFloat(neurons[1][0]->Vm, stimulus_len);
    // printListInt(neurons[1][0]->time, stimulus_len);

    //Print Spike and time neuron[1][0]
    printf("Output spike to time of neuron 1/0: \n");
    printOnlyWhenFire(neurons[1][0]->spike, stimulus_len);
    // printListFloat(neurons[1][0]->spike, stimulus_len);
    // printListInt(neurons[1][0]->time, stimulus_len);

    free(layer_spikes);
    freeNeurons();

    return 0; 
}
