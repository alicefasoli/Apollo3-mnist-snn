#include <stdio.h>
#include <stdlib.h>

#include "neuron.h"
#include "list.h"
#include "mnist.h"

#define INPUT_LAYER_NUM_X 28
#define HIDDEN_LAYER_NUM_X 10
#define OUTPUT_LAYER_NUM 10

#define N_TEST_IMAGE 15

struct Neuron *inputLayer[INPUT_LAYER_NUM_X][INPUT_LAYER_NUM_X];
struct Neuron *exc_hiddenLayer[HIDDEN_LAYER_NUM_X][HIDDEN_LAYER_NUM_X];
struct Neuron *inh_hiddenLayer[HIDDEN_LAYER_NUM_X][HIDDEN_LAYER_NUM_X];
struct Neuron *outputLayer[OUTPUT_LAYER_NUM];

void createNeuronsPerLayer(int n, struct Neuron* layer[n][n], int duration)
{
    for(int y = 0; y < n; y++){
        for(int x = 0; x < n; x++){
            layer[y][x] = createNeuron();
            initList(layer[y][x], duration);
        }
    }  
}

void printNeuronsLayer(int n, struct Neuron** layer)
{
    for(int i = 0; i < n; i++){
        printf("LIF %d : [", i);
        printNeuron(layer[i]);
        printf("]\n");
    }  
}

void freeNeuronsPerLayer(int n, struct Neuron* layer[n][n])
{
    for(int y = 0; y < n; y++){
        for(int x = 0; x < n; x++){
            destroyNeuron(layer[y][x]);
        }
    }  
}

void freeNeuronsOutputLayer(int n, struct Neuron* layer[n])
{
    for(int y = 0; y < n; y++){
        destroyNeuron(layer[y]);
    }  
}

struct Neuron** propagateSignal(struct Neuron** layerFrom, struct Neuron** layerTo, int layerFromNum, int layerToNum, int duration)
{
    float* neurons_stimulus[layerToNum];
    for(int i = 0; i < layerToNum; i++){
        neurons_stimulus[i] = (float*) calloc(duration, sizeof(float));
    }

    for(int i = 0; i < layerToNum; i++){
        for(int j = 0; j < layerFromNum; j++){
            for(int t = 0; t < duration; t++){
                neurons_stimulus[i][t] = neurons_stimulus[i][t] + layerFrom[j]->spike[i];
            }
        }
    }

    // Run spikes through layer
    for(int i = 0; i < layerToNum; i++){
        spikeGenerator(neurons_stimulus[i], duration, layerTo[i]);
    }

    return layerTo;
}

void printSpikesPerLayer(int n, struct Neuron** layer, int duration)
{
    // Print generated spikes for output layer
    for(int j = 0; j < n; j++){
        for(int i = 0; i < duration; i++){
            if(layer[j]->spike[i] > 0.0){
                printf("Neuron {%d} : Fire at time %d s with value [%2.3f]\n", j, i, layer[j]->spike[i]);
            }
        }
    }
}

int main()
{
    int T = 2000; // total time to simulate (msec)
    float dt = 1.25; // simulation timestep
    int time = (int)T/dt;
    
    load_mnist();

    // Print a mnist test image
    // for(int i=0; i < (28*28); i++){
    //     // printf("%1.1f ", test_image[N_TEST_IMAGE][i]);
    //     if(test_image[N_TEST_IMAGE][i] > 0.0){
    //         printf("@ ");
    //     }else{
    //         printf("- ");
    //     }
    //     if( (i+1) % 28 ==0 ) putchar('\n');
    // }

    createNeuronsPerLayer(INPUT_LAYER_NUM_X, inputLayer, time);
    // printNeuronsLayer(INPUT_LAYER_NUM, inputLayer);

    // Create stimulus for the input layer using Poisson distribution
    int count = 0;
    float n_random;
    for(int pos_y = 0; pos_y < INPUT_LAYER_NUM_X; pos_y++){
        for(int pos_x = 0; pos_x < INPUT_LAYER_NUM_X; pos_x++){
            count++;
            for(int i=0; i < time; i++){
                n_random = (float)rand()/(float)(RAND_MAX/1);
                if( (test_image[N_TEST_IMAGE][count]*dt) > n_random){
                    inputLayer[pos_y][pos_x]->spike[time] = 1.0;
                }
            }
        }
    }

    createNeuronsPerLayer(HIDDEN_LAYER_NUM_X, exc_hiddenLayer, time);
    createNeuronsPerLayer(HIDDEN_LAYER_NUM_X, inh_hiddenLayer, time);

    // Connection all-to-all
    

    // Connection one-to-one from excitatory to inhibitory layer

    // Create output layer
    for(int i = 0; i < OUTPUT_LAYER_NUM; i++){
        outputLayer[i] = createNeuron();
        initList(outputLayer[i], time);
    }

    freeNeuronsOutputLayer(OUTPUT_LAYER_NUM, outputLayer);
    freeNeuronsPerLayer(HIDDEN_LAYER_NUM_X, inh_hiddenLayer);
    freeNeuronsPerLayer(HIDDEN_LAYER_NUM_X, exc_hiddenLayer);
    freeNeuronsPerLayer(INPUT_LAYER_NUM_X, inputLayer);

    return 0; 
}
