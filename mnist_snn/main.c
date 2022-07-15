#include <stdio.h>
#include <time.h>

#include "neuron.h"
#include "stdp.h"
#include "parameters.h"
#include "mnist.h"

double interp(double x, double xp[2], double fp[2])
{
    double x_interpoled;
    double yi = fp[0] + ((x - xp[0]) / (xp[1] - xp[0])) * (fp[1] - fp[0]);

    if (yi < fp[0])
    {
        x_interpoled = fp[0];
    }
    else if (yi > fp[1])
    {
        x_interpoled = fp[1];
    }
    else
    {
        x_interpoled = yi;
    }

    return x_interpoled;
}

int main()
{
    int i, j, k, m, n, x, y, n_train, tm, t1;

    clock_t start, end;
    double time_used;

    FILE *f_weights;
    f_weights = fopen("weights.txt", "rw+");

    FILE *f_labels;
    f_labels = fopen("labels.txt", "rw+");

    double train[N_FIRST_LAYER * (t + 1)];
    double actual_img[PIXEL][PIXEL];
    double pot[PIXEL * PIXEL];

    // Initialize matrix
    Neuron *output_layer[N_SECOND_LAYER]; // Creating hidden layer of neurons
    int label_neuron[N_SECOND_LAYER];     // Creating labels corresponding to neuron

    double *synapse = (double *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(double));
    int *synapse_memory = (int *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(int));

    int *count_spikes = (int *)calloc(N_SECOND_LAYER, sizeof(int));
    double *active_pot = (double *)calloc(N_SECOND_LAYER, sizeof(double));

    for (i = 0; i < N_SECOND_LAYER; i++)
    {
        output_layer[i] = initial();
        label_neuron[i] = -1;
        for (j = 0; j < N_FIRST_LAYER; j++)
        {
            synapse[(i * N_FIRST_LAYER) + j] = 1.0;
        }
    }

    int img_to_train[N_SECOND_LAYER];
    int cont[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    load_mnist();

    int n_img = 0;
    do
    {
        if (cont[0] < FOTO && train_label[n_img] == 0)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[0]++;
        }
        else if (cont[1] < FOTO && train_label[n_img] == 1)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[1]++;
        }
        else if (cont[2] < FOTO && train_label[n_img] == 2)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[2]++;
        }
        else if (cont[3] < FOTO && train_label[n_img] == 3)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[3]++;
        }
        else if (cont[4] < FOTO && train_label[n_img] == 4)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[4]++;
        }
        else if (cont[5] < FOTO && train_label[n_img] == 5)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[5]++;
        }
        else if (cont[6] < FOTO && train_label[n_img] == 6)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[6]++;
        }
        else if (cont[7] < FOTO && train_label[n_img] == 7)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[7]++;
        }
        else if (cont[8] < FOTO && train_label[n_img] == 8)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[8]++;
        }
        else if (cont[9] < FOTO && train_label[n_img] == 9)
        {
            img_to_train[cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9]] = n_img;
            cont[9]++;
        }

        n_img++;

    } while (cont[0] + cont[1] + cont[2] + cont[3] + cont[4] + cont[5] + cont[6] + cont[7] + cont[8] + cont[9] != N_SECOND_LAYER);

    int train_img = 0;

    for (n_train = 0; n_train < N_SECOND_LAYER; n_train++) // NUM_TRAIN
    {
        train_img = img_to_train[n_train];

        start = clock();
        printf("%d.png [%d]: \n", train_img, n_train);

        for (i = 0; i < PIXEL; i++) // loop for pixel in axes x
        {
            printf("[");
            for (j = 0; j < PIXEL; j++) // loop for pixel in axes y
            {
                actual_img[i][j] = train_image[train_img][(i * PIXEL) + j];
                printf("%2.f ", actual_img[i][j]);
            }
            printf("]\n");
        }

        // Receptive field convolution
        double min = 0.0;
        double max = -10000.0;
        double sum;
        for (i = 0; i < PIXEL; i++) // loop for receptive field convolution
        {
            // printf("[");
            for (j = 0; j < PIXEL; j++)
            {
                sum = 0.0;
                for (m = 0; m < 5; m++)
                {
                    for (n = 0; n < 5; n++)
                    {
                        if ((i + (double)ran[m]) >= 0 && (i + (double)ran[m]) <= (PIXEL - 1) && (j + (double)ran[n]) >= 0 && (j + (double)ran[n]) <= (PIXEL - 1))
                        {
                            sum = sum + w[ox + ran[m]][oy + ran[n]] * (actual_img[i + ran[m]][j + ran[n]] / 255.0);
                        }
                    }
                }
                pot[(i * PIXEL) + j] = sum;

                if (min > pot[(i * PIXEL) + j])
                {
                    min = pot[(i * PIXEL) + j];
                }

                if (max < pot[(i * PIXEL) + j])
                {
                    max = pot[(i * PIXEL) + j];
                }
            }
            // printf("]\n");
        }
        // printf("Min pot : %2.1f\n", min);
        // printf("Max pot : %2.1f\n", max);

        // Spike train encoding
        double xp[2];
        double fp[2] = {1.0, 50.0};
        double freq, time_period, time_of_spike;
        for (i = 0; i < N_FIRST_LAYER; i++) // loop for potential list
        {
            xp[0] = min;
            xp[1] = max;
            freq = interp(pot[i], xp, fp);

            time_period = ceil((double)t / freq);

            time_of_spike = time_period;

            for (j = 0; j < (t + 1); j++)
            {
                train[(i * (t + 1)) + j] = 0.0;
            }

            if (pot[i] > 0)
            {
                while (time_of_spike < t + 1)
                {
                    train[(i * (t + 1)) + (int)time_of_spike] = 1.0;
                    time_of_spike = time_of_spike + time_period;
                }
            }
        }

        int winner = 0;

        for (k = 0; k < N_SECOND_LAYER; k++)
        {
            count_spikes[k] = 0;
            active_pot[k] = 0.0;
        }

        double dotProduct;
        for (tm = 0; tm < (t + 1); tm++)
        {
            for (i = 0; i < N_SECOND_LAYER; i++)
            {
                if (output_layer[i]->t_rest < tm)
                {
                    dotProduct = 0.0;
                    for (j = 0; j < N_FIRST_LAYER; j++)
                    {
                        dotProduct = dotProduct + (synapse[(i * N_FIRST_LAYER) + j] * train[(j * (t + 1)) + tm]);
                    }
                    output_layer[i]->p = output_layer[i]->p + dotProduct;

                    if (output_layer[i]->p > p_rest)
                    {
                        output_layer[i]->p = output_layer[i]->p - p_drop;
                        if (output_layer[i]->p_th > p_th)
                        {
                            output_layer[i]->p_th = output_layer[i]->p_th - p_th_drop;
                        }
                    }
                    active_pot[i] = output_layer[i]->p;
                }
            }

            double argmax_active = active_pot[0];
            for (i = 0; i < N_SECOND_LAYER; i++)
            {
                if (argmax_active < active_pot[i])
                {
                    argmax_active = active_pot[i];
                    winner = i;
                }
            }

            double rl_t;
            // Check for spikes and update weights
            for (i = 0; i < N_SECOND_LAYER; i++)
            {
                if (i == winner && active_pot[i] > output_layer[i]->p_th)
                {
                    hyperpolarization(output_layer[i], tm);
                    output_layer[i]->p_th = output_layer[i]->p_th - 1.0;
                    count_spikes[i] = count_spikes[i] + 1;
                    for (j = 0; j < N_FIRST_LAYER; j++)
                    {
                        for (t1 = 0; t1 > (t_back - 1); t1--)
                        {
                            if (0 <= tm + t1 && tm + t1 < t + 1)
                            {
                                if (train[(j * (t + 1)) + (t + t1)] == 1.0)
                                {
                                    rl_t = rl(t1);
                                    synapse[(i * N_FIRST_LAYER) + j] = update(synapse[(i * N_FIRST_LAYER) + j], rl_t);
                                    synapse_memory[(i * N_FIRST_LAYER) + j] = 1;
                                    break;
                                }
                            }
                        }
                        if (synapse_memory[(i * N_FIRST_LAYER) + j] != 1)
                        {
                            rl_t = rl(1);
                            synapse[(i * N_FIRST_LAYER) + j] = update(synapse[(i * N_FIRST_LAYER) + j], rl_t);
                        }
                    }
                    for (k = 0; k < N_SECOND_LAYER; k++)
                    {
                        if (k != winner)
                        {
                            if (output_layer[k]->p > output_layer[k]->p_th)
                            {
                                count_spikes[k] = count_spikes[k] + 1;
                            }
                            inhibit(output_layer[k], tm);
                        }
                    }
                    break;
                }
            }
        }

        for (i = 0; i < N_SECOND_LAYER; i++)
        {
            reset(output_layer[i]);
        }

        label_neuron[winner] = train_label[train_img];
        printf("Label neuron : %d\n", label_neuron[winner]);

        end = clock();
        time_used = (double)(end - start) / CLOCKS_PER_SEC;

        int argmax_count = count_spikes[0];
        int learning_neuron = 0;
        for (i = 0; i < N_SECOND_LAYER; i++)
        {
            if (argmax_count < count_spikes[i])
            {
                argmax_count = count_spikes[i];
                learning_neuron = i;
            }
        }
        printf("Learning neuron : %d\n", learning_neuron);
        printf("Learning duration : %.3f sec\n", time_used);
    }

    // printf("Reconstructed images : \n");
    // for (i = 0; i < N_SECOND_LAYER; i++)
    // {
    //     // if (label_neuron[i] == -1)
    //     // {
    //     //     for (j = 0; j < N_FIRST_LAYER; j++)
    //     //     {
    //     //         synapse[(i * N_FIRST_LAYER) + j] = 0.0;
    //     //     }
    //     // }

    //     // Reconstruct weights
    //     double weight_matrix[PIXEL][PIXEL];
    //     double img[PIXEL][PIXEL];
    //     printf("%d.png : \n", img_to_train[i]);
    //     for (x = 0; x < PIXEL; x++)
    //     {
    //         for (y = 0; y < PIXEL; y++)
    //         {
    //             weight_matrix[x][y] = synapse[(i * N_FIRST_LAYER) + (x * PIXEL) + y];
    //             xp[0] = w_min;
    //             xp[1] = w_max;
    //             fp[0] = 0.0;
    //             fp[1] = 255.0;
    //             img[x][y] = interp(weight_matrix[x][y], xp, fp);
    //             if (img[x][y] > 25.0)
    //             {
    //                 printf("@ ");
    //             }
    //             else
    //             {
    //                 printf("- ");
    //             }
    //         }
    //         printf("\n");
    //     }
    // }

    for (i = 0; i < N_SECOND_LAYER; i++)
    {
        fprintf(f_labels, "%d,", label_neuron[i]);
        for (j = 0; j < N_FIRST_LAYER; j++)
        {
            fprintf(f_weights, "%f,", synapse[(i * N_FIRST_LAYER) + j]);
        }
        fprintf(f_weights, "\n");
    }

    fclose(f_labels);
    fclose(f_weights);

    // Free lists
    for (i = 0; i < N_SECOND_LAYER; i++)
    {
        free(output_layer[i]);
    }

    free(synapse);
    free(synapse_memory);

    free(count_spikes);
    free(active_pot);

    return 0;
}