#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#include "neuron.h"
#include "stdp.h"
#include "parameters.h"
#include "utils.h"
#include "mnist.h"

int main()
{
    int i, j, m, n, k, p, h, x, y, e, n_train, tm, t1; // for-loop variables

    FILE *f_weights;
    f_weights = fopen("weights.csv", "rw+");

    FILE *f_labels;
    f_labels = fopen("labels.csv", "rw+");

    double train[N_FIRST_LAYER * (t + 1)];
    double actual_img[PIXEL][PIXEL];
    double pot[PIXEL * PIXEL];

    // Initialize matrix
    Neuron *output_layer[N_SECOND_LAYER]; // Creating hidden layer of neurons
    int label_neuron[N_SECOND_LAYER];     // Creating labels corresponding to neuron

    double *synapse = (double *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(double));
    double *synapse_memory = (double *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(double));

    for (i = 0; i < N_SECOND_LAYER; i++)
    {
        output_layer[i] = initial();
        label_neuron[i] = -1;
    }

    // for (int j = 0; j < N_SECOND_LAYER * N_FIRST_LAYER; j++)
    // {
    //     synapse[j] = 1.0;
    // }

    int winner;

    clock_t start, end;
    double time_used;

    double sum, dotProduct, argmax_active, argmax_count, rl_t;
    int learning_neuron;

    double xp[2], fp[2], freq, time_period, time_of_spike;

    load_mnist();

    for (e = 0; e < epoch; e++)
    {
        printf("epoch : %d\n", e);
        for (n_train = 0; n_train < 2000; n_train++) // NUM_TRAIN
        {
            start = clock();
            printf("%d : \n", n_train);

            for (i = 0; i < PIXEL; i++)
            {
                printf("[");
                for (j = 0; j < PIXEL; j++)
                {
                    if (train_image[n_train][(i * PIXEL) + j] == 1.0)
                    {
                        actual_img[i][j] = 255.0;
                    }
                    else
                    {
                        actual_img[i][j] = train_image[n_train][(i * PIXEL) + j] * 256.0;
                    }
                    printf("%2.f ", actual_img[i][j]);
                }
                printf("]\n");
            }

            // Receptive field convolution
            for (i = 0; i < PIXEL; i++)
            {
                for (j = 0; j < PIXEL; j++)
                {
                    sum = 0.0;
                    for (m = 0; m < 5; m++)
                    {
                        for (n = 0; n < 5; n++)
                        {
                            if ((i + ran[m]) >= 0 && (i + ran[m]) <= 27 && (j + ran[n]) >= 0 && (j + ran[n]) <= 27)
                            {
                                sum = sum + w[ox + m][oy + n] * actual_img[i + m][j + n];
                            }
                        }
                    }
                    pot[(i * PIXEL) + j] = sum;
                    // printf("pot[%d][%d] = [%d] : %2.3f\n", i, j, (i * PIXEL) + j, pot[(i * PIXEL) + j]);
                }
            }

            // Spike train encoding
            for (k = 0; k < N_FIRST_LAYER; k++)
            {
                double *temp = (double *)calloc(t + 1, sizeof(double));

                xp[0] = min(pot);
                xp[1] = max(pot);
                fp[0] = 1.0;
                fp[1] = 50.0;
                freq = interp(pot[k], xp, fp);

                time_period = ceil(t / freq);

                time_of_spike = time_period;
                if (pot[k] > 0)
                {
                    while (time_of_spike < t + 1)
                    {
                        temp[(int)time_of_spike] = 1.0;
                        time_of_spike = time_of_spike + time_period;
                    }
                }

                for (j = 0; j < (t + 1); j++)
                {
                    train[(k * (t + 1)) + j] = temp[j];

                    // if (train[(k * (t + 1)) + j] > 0)
                    // {
                    //     printf("train[%d][%d] = [%d] : %2.3f\n", k, j, (k * (t + 1)) + j, train[(k * (t + 1)) + j]);
                    // }
                }

                free(temp);
            }

            winner = 0;

            // double count_spikes[N_SECOND_LAYER];
            // double active_pot[N_SECOND_LAYER];

            double *count_spikes = (double *)calloc(N_SECOND_LAYER, sizeof(double));
            double *active_pot = (double *)calloc(N_SECOND_LAYER, sizeof(double));

            for (tm = 0; tm < (t + 1); tm++)
            {
                for (j = 0; j < N_SECOND_LAYER; j++)
                {
                    // count_spikes[j] = 0.0;
                    // active_pot[j] = 0.0;

                    dotProduct = 0.0;
                    for (i = 0; i < N_FIRST_LAYER; i++)
                    {
                        synapse[(j * N_FIRST_LAYER) + i] = 1.0;
                        dotProduct = dotProduct + (synapse[(j * N_FIRST_LAYER) + i] * train[(i * (t + 1)) + tm]);
                        // if (dotProduct > 0)
                        // {
                        //     printf("dot product : %2.3f* %2.3f = %2.3f", synapse[(j * N_FIRST_LAYER) + i], train[(i * (t + 1)) + tm], dotProduct);
                        // }
                    }

                    if (output_layer[j]->t_rest < tm)
                    {
                        output_layer[j]->p = output_layer[j]->p + dotProduct; // + dotProduct1D(synapse_init, j, train, tm);

                        if (output_layer[j]->p > p_rest)
                        {
                            output_layer[j]->p = output_layer[j]->p - p_drop;
                            if (output_layer[j]->p_th > p_th)
                            {
                                output_layer[j]->p_th = output_layer[j]->p_th - p_th_drop;
                            }
                        }
                        active_pot[j] = output_layer[j]->p;
                        // printf("%2.f" , active_pot[j]);
                    }
                }
                // winner = argmax(active_pot, N_SECOND_LAYER);
                argmax_active = active_pot[0];
                for (i = 1; i < N_SECOND_LAYER; i++)
                {
                    if (argmax_active < active_pot[i])
                    {
                        argmax_active = active_pot[i];
                        winner = i;
                    }
                }

                // Check for spikes and update weights
                for (j = 0; j < N_SECOND_LAYER; j++)
                {
                    if (j == winner && active_pot[j] > output_layer[j]->p_th)
                    {
                        hyperpolarization(output_layer[j], tm);
                        output_layer[j]->p_th = output_layer[j]->p_th - 1.0;
                        count_spikes[j] = count_spikes[j] + 1.0;
                        for (h = 0; h < N_FIRST_LAYER; h++)
                        {
                            for (t1 = 0; t1 > t_back - 1; t1--)
                            {
                                if (0 <= tm + t1 && tm + t1 < t + 1)
                                {
                                    if (train[(h * (t + 1)) + (t + t1)] == 1.0)
                                    {
                                        rl_t = rl(t1);
                                        synapse[(j * N_FIRST_LAYER) + h] = update(synapse[(j * N_FIRST_LAYER) + h], rl_t);
                                        synapse_memory[(j * N_FIRST_LAYER) + h] = 1.0;
                                        break;
                                    }
                                }
                            }
                            if (synapse_memory[(j * N_FIRST_LAYER) + h] != 1.0)
                            {
                                rl_t = rl(1.0);
                                synapse[(j * N_FIRST_LAYER) + h] = update(synapse[(j * N_FIRST_LAYER) + h], rl_t);
                            }
                        }
                        for (p = 0; p < N_SECOND_LAYER; p++)
                        {
                            if (p != winner)
                            {
                                if (output_layer[p]->p > output_layer[p]->p_th)
                                {
                                    count_spikes[p] = count_spikes[p] + 1.0;
                                }
                                inhibit(output_layer[p], tm);
                            }
                        }
                        break;
                    }
                }
            }

            for (p = 0; p < N_SECOND_LAYER; p++)
            {
                reset(output_layer[p]);
            }

            label_neuron[winner] = train_label[n_train];
            printf("Label neuron : %d\n", label_neuron[winner]);

            end = clock();
            time_used = (double)(end - start) / CLOCKS_PER_SEC;

            // int mx = argmax(count_spikes, N_SECOND_LAYER);
            argmax_count = count_spikes[0];
            learning_neuron = 0;
            for (i = 1; i < N_SECOND_LAYER; i++)
            {
                if (argmax_count < count_spikes[i])
                {
                    argmax_count = count_spikes[i];
                    learning_neuron = i;
                }
            }
            printf("Learning neuron : %d\n", learning_neuron);
            printf("Learning duration : %.3f sec\n", time_used);

            printf("End train img number : %d\n", n_train);

            free(count_spikes);
            free(active_pot);
        }
    }

    for (i = 0; i < N_SECOND_LAYER; i++)
    {
        if (label_neuron[i] == -1)
        {
            for (int j = 0; j < N_FIRST_LAYER; j++)
            {
                synapse[(i * N_FIRST_LAYER) + j] = 0.0;
            }
        }

        // Reconstruct weights
        double weight_matrix[PIXEL][PIXEL];
        double img[PIXEL][PIXEL];

        for (x = 0; x < PIXEL; x++)
        {
            for (y = 0; y < PIXEL; y++)
            {
                weight_matrix[x][y] = synapse[(i * N_FIRST_LAYER) + (x * PIXEL) + y];
                xp[0] = w_min;
                xp[1] = w_max;
                fp[0] = 0.0;
                fp[1] = 255.0;
                img[x][y] = interp(weight_matrix[x][y], xp, fp);
            }
        }
    }

    for (i = 0; i < N_SECOND_LAYER; i++)
    {
        fprintf(f_labels, "%i,", label_neuron[i]);
        for (j = 0; j < N_FIRST_LAYER; j++)
        {
            fprintf(f_weights, "%f,", synapse[(i * N_FIRST_LAYER) + j]);
        }
        fprintf(f_weights, "\n");
    }

    fclose(f_weights);
    fclose(f_labels);

    // Free neuron list
    for (i = 0; i < N_SECOND_LAYER; i++)
    {
        free(output_layer[i]);
    }

    free(synapse);
    free(synapse_memory);

    return 0;
}