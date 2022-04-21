#include <stdio.h>
#include <stdlib.h>

#include "neuron.h"
#include "list.h"
#include "mnist.h"

#define NUM_LAYER 28
#define NUM_NEURON 28

struct Neuron *neurons[NUM_LAYER][NUM_NEURON]; 

void printMatrix()
{
    for(int i=0; i < NUM_LAYER; i++){
        for(int j = 0; j < NUM_NEURON; j++){
            if(neurons[i][j] != NULL){
                printf("\nLIF(%d/%d) : ", i, j);
                printNeuron(neurons[i][j]);
            }
        }
        printf("]\n");
    }
}

struct Neuron* create_neurons(int x, int y)
{
    int layer, neuron;
    for(layer = 0; layer < y; layer++){
        for(neuron = 0; neuron < x; neuron++){
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
    int T = 20; // total time to simulate (msec)
    float dt = 0.0125; // simulation timestep
    int time = (int)T/dt;
    
    load_mnist();

    // Print a mnist test image
    // for(int i=0; i < (28*28); i++){
    //     printf("%1.1f ", test_image[0][i]);
    //     if( (i+1) % 28 ==0 ) putchar('\n');
    // }
    
    int stride[2] = {4,2};
    int stride_size = stride[0] + stride[1];
    int len_x = 28;
    int len_y = 28;

    create_neurons(len_x, len_y);
    // printMatrix();

    int start_x = 5;
    int start_y = 5;

    int pos = 0;

    for(int y = 0; y < len_y; y++){
        pos = y * 28;
        for(int x = 0; x < len_x; x++){
            pos++;
            float *stimulus = (float*) calloc(time, sizeof(float)); 
            for(int i = 0; i < time; i++){
                stimulus[i] = test_image[0][pos];
            }
            // printf("[%1.1f , %1.1f, ... , %1.1f, %1.1f]\n", stimulus[0], stimulus[1], stimulus[time-2], stimulus[time-1]);
            spikeGenerator(stimulus, time, neurons[y][x]);
            free(stimulus);
        }
    }

    // Print generated spikes
    // for(int y = 0; y < len_y; y++){
    //     for(int x = 0; x < len_x; x++){
    //         for(int i = 0; i < time; i++){
    //             if(neurons[y][x]->spike[i] > 0.0){
    //                 printf("Neuron {%d/%d} : Fire at position [%d] with value [%2.3f]\n", y, x, i, neurons[y][x]->spike[i]);
    //             }
    //         }
    //     }
    // }

    int l2Neurons_x = (int)len_x/stride[0];
    int l2Neurons_y = (int)len_y/stride[0];

    struct Neuron *layer2[l2Neurons_x][l2Neurons_y]; 
    for(int y=0; y < l2Neurons_y; y++){
        for(int x=0; x < l2Neurons_x; x++){
            layer2[y][x] = neuron_Create();
        }
    }

    float* l2Neurons_stimulus[l2Neurons_y][l2Neurons_x];
    for(int y = 0; y < l2Neurons_y; y++){
        for(int x = 0; x < l2Neurons_x; x++){
            l2Neurons_stimulus[y][x] = (float*) calloc(time, sizeof(float));
        }
    }

    int mult_factor = 49; // Number of neurons for each pixel

    int l2x = 0;
    int l2y = 0;

    int x = 0;
    int y = 0;

    for(int ry = 0; ry < len_y; ry+=stride[0]){
        l2x = 0;
        for(int rx = 0; rx < len_x; rx+=stride[0]){
            // printf("\nGenerating stimulus for L2 neurons %d/%d\n", l2y, l2x);
            for(int ny = 0; ny < stride[0]; ny++){
                for(int nx = 0; nx < stride[0]; nx++){
                    x = rx + nx;
                    y = ry + ny;
                    for(int i = 0; i < time; i++){
                        l2Neurons_stimulus[l2y][l2x][i] = l2Neurons_stimulus[l2y][l2x][i] + neurons[y][x]->spike[i]*mult_factor;
                    }
                }
            }
            // printf("Adding stimulus for l2 neuron %d/%d, duration=%d\n",l2y, l2x, time);
            l2x++;
        }
        l2y++;
    }

    // Print l2 Neurons stimulus
    // for(int i=0; i < l2Neurons_y; i++){
    //     for(int j = 0; j < l2Neurons_x; j++){
    //         if(l2Neurons_stimulus[i][j] != NULL){
    //             printf("Neuron %d/%d:\n", i,j);
    //             printOnlyWhenFire(l2Neurons_stimulus[i][j], time);
    //             printf("\n");
    //         }
    //     }
    // }

    // Run spikes through L2 Neurons
    for(int y = 0; y < l2Neurons_y; y++){
        for(int x = 0; x < l2Neurons_x; x++){
            spikeGenerator(l2Neurons_stimulus[y][x], time, layer2[y][x]);
        }
    }

    // Print layer 2 spike
    // for(int y = 0; y < l2Neurons_y; y++){
    //     for(int x = 0; x < l2Neurons_x; x++){
    //         if(layer2[y][x] != NULL){
    //             printf("Neuron %d/%d:\n", y, x);
    //             printOnlyWhenFire(layer2[y][x]->spike, time);
    //             printf("\n");
    //         }
    //     }
    // }



    freeNeurons();

    return 0; 
}
