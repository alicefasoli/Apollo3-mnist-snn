#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#include "neuron.h"
#include "stdp.h"
#include "parameters.h"
#include "mnist.h"

float interp(float x, float xp[2], float fp[2])
{
    float x_interpoled;

    float yi = fp[0] + ((x - xp[0]) / (xp[1] - xp[0])) * (fp[1] - fp[0]);
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
    int i, j, m, n, x, y, n_train, tmp, tm, t1, lpPx_x_rf, lpPx_y_rf, lp_pot, lpTime_tmp, lp_first, lp_dotProd, lp_active, lp_update_1, lp_initial, lp_update_2, lp_inhibit, lp_reset, lp_count, lp_third, lp_fourth, lp_fifth, lp_free; // for-loop variables
    int lpPx_x = 0;
    int lpPx_y = 0;

    FILE *f_weights;
    f_weights = fopen("weights.txt", "rw+");

    FILE *f_labels;
    f_labels = fopen("labels.txt", "rw+");

    float train[N_FIRST_LAYER * (t + 1)];
    float actual_img[PIXEL][PIXEL];
    float pot[PIXEL * PIXEL];

    // Initialize matrix
    Neuron *output_layer[N_SECOND_LAYER]; // Creating hidden layer of neurons
    int label_neuron[N_SECOND_LAYER];     // Creating labels corresponding to neuron

    float *synapse = (float *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(float));
    int *synapse_memory = (int *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(int));

    int *count_spikes = (int *)calloc(N_SECOND_LAYER, sizeof(int));
    float *active_pot = (float *)calloc(N_SECOND_LAYER, sizeof(float));

    float *temp = (float *)calloc(t + 1, sizeof(float));

    for (lp_initial = 0; lp_initial < N_SECOND_LAYER; lp_initial++)
    {
        output_layer[lp_initial] = initial();
        label_neuron[lp_initial] = -1;
        for (i = 0; i < N_FIRST_LAYER; i++)
        {
            synapse[(lp_initial * N_FIRST_LAYER) + i] = 1.0;
        }
    }

    int winner;

    clock_t start, end;
    float time_used;

    float sum, dotProduct, argmax_active, min, max;
    int learning_neuron, argmax_count;

    float xp[2], fp[2], freq, time_period, time_of_spike;

    float rl_t;

    int *img_to_train = (int *)calloc(N_SECOND_LAYER, sizeof(int));

    int cont_0 = 0;
    int cont_1 = 0;
    int cont_2 = 0;
    int cont_3 = 0;
    int cont_4 = 0;
    int cont_5 = 0;
    int cont_6 = 0;
    int cont_7 = 0;
    int cont_8 = 0;
    int cont_9 = 0;

    load_mnist();

    int n_img = 0;

    do
    {
        if (cont_0 < FOTO && train_label[n_img] == 0)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_0++;
        }
        else if (cont_1 < FOTO && train_label[n_img] == 1)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_1++;
        }
        else if (cont_2 < FOTO && train_label[n_img] == 2)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_2++;
        }
        else if (cont_3 < FOTO && train_label[n_img] == 3)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_3++;
        }
        else if (cont_4 < FOTO && train_label[n_img] == 4)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_4++;
        }
        else if (cont_5 < FOTO && train_label[n_img] == 5)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_5++;
        }
        else if (cont_6 < FOTO && train_label[n_img] == 6)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_6++;
        }
        else if (cont_7 < FOTO && train_label[n_img] == 7)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_7++;
        }
        else if (cont_8 < FOTO && train_label[n_img] == 8)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_8++;
        }
        else if (cont_9 < FOTO && train_label[n_img] == 9)
        {
            img_to_train[cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9] = n_img;
            cont_9++;
        }

        n_img++;

    } while (cont_0 + cont_1 + cont_2 + cont_3 + cont_4 + cont_5 + cont_6 + cont_7 + cont_8 + cont_9 != N_SECOND_LAYER);

    // printf("Resume: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", cont_0, cont_1, cont_2, cont_3, cont_4, cont_5, cont_6, cont_7, cont_8, cont_9);

    // for (int i = 0; i < N_SECOND_LAYER; i++)
    // {
    //     printf("%d ", img_to_train[i]);
    // }

    int train_img = 0;

    for (n_train = 0; n_train < N_SECOND_LAYER; n_train++) // NUM_TRAIN
    {
        train_img = img_to_train[n_train];

        start = clock();
        printf("%d.png [%d]: \n", train_img, n_train);

        for (lpPx_x = 0; lpPx_x < PIXEL; lpPx_x++) // loop for pixel in axes x
        {
            // printf("[");
            for (lpPx_y = 0; lpPx_y < PIXEL; lpPx_y++) // loop for pixel in axes y
            {
                // actual_img[lpPx_x][lpPx_y] = train_image[train_img][(lpPx_x * PIXEL) + lpPx_y];
                if (train_image[train_img][(lpPx_x * PIXEL) + lpPx_y] == 1.0)
                {
                    actual_img[lpPx_x][lpPx_y] = 255.0;
                }
                else
                {
                    actual_img[lpPx_x][lpPx_y] = train_image[train_img][(lpPx_x * PIXEL) + lpPx_y] * 256.0;
                }
                // printf("%2.f ", actual_img[lpPx_x][lpPx_y]);
            }
            // printf("]\n");
        }

        // Receptive field convolution
        min = 0.0;
        max = -10000.0;
        for (lpPx_x_rf = 0; lpPx_x_rf < PIXEL; lpPx_x_rf++) // loop for receptive field convolution
        {
            for (lpPx_y_rf = 0; lpPx_y_rf < PIXEL; lpPx_y_rf++)
            {
                sum = 0.0;
                for (m = 0; m < 5; m++)
                {
                    for (n = 0; n < 5; n++)
                    {
                        if ((lpPx_x_rf + ran[m]) >= 0 && (lpPx_x_rf + ran[m]) <= 27 && (lpPx_y_rf + ran[n]) >= 0 && (lpPx_y_rf + ran[n]) <= 27)
                        {
                            sum = sum + w[ox + m][oy + n] * actual_img[lpPx_x_rf + m][lpPx_y_rf + n];
                        }
                    }
                }
                pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf] = sum;

                if (min > pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf])
                {
                    min = pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf];
                }

                if (max < pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf])
                {
                    max = pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf];
                }
            }
        }
        // printf("Min pot : %2.1f\n", min);
        // printf("Max pot : %2.1f\n", max);

        // Spike train encoding
        for (lp_pot = 0; lp_pot < N_FIRST_LAYER; lp_pot++) // loop for potential list
        {
            xp[0] = min;
            xp[1] = max;
            fp[0] = 1.0;
            fp[1] = 50.0;
            freq = interp(pot[lp_pot], xp, fp);

            time_period = ceilf((float)t / freq);

            time_of_spike = time_period;

            for (tmp = 0; tmp < (t + 1); tmp++)
            {
                temp[tmp] = 0.0;
            }

            if (pot[lp_pot] > 0)
            {
                while (time_of_spike < t + 1)
                {
                    temp[(int)time_of_spike] = 1.0;
                    time_of_spike = time_of_spike + time_period;
                }
            }

            for (lpTime_tmp = 0; lpTime_tmp < (t + 1); lpTime_tmp++)
            {
                train[(lp_pot * (t + 1)) + lpTime_tmp] = temp[lpTime_tmp];
                // printf("%f ", train[(lp_pot * (t + 1)) + lpTime_tmp]);
            }
        }

        winner = 0;
        for (int q = 0; q < N_SECOND_LAYER; q++)
        {
            count_spikes[q] = 0;
            active_pot[q] = 0.0;
        }

        for (tm = 0; tm < (t + 1); tm++)
        {
            for (lp_first = 0; lp_first < N_SECOND_LAYER; lp_first++)
            {
                if (output_layer[lp_first]->t_rest < tm)
                {
                    dotProduct = 0.0;
                    for (lp_dotProd = 0; lp_dotProd < N_FIRST_LAYER; lp_dotProd++)
                    {
                        dotProduct = dotProduct + (synapse[(lp_first * N_FIRST_LAYER) + lp_dotProd] * train[(lp_dotProd * (t + 1)) + tm]);
                    }
                    output_layer[lp_first]->p = output_layer[lp_first]->p + dotProduct;

                    if (output_layer[lp_first]->p > p_rest)
                    {
                        output_layer[lp_first]->p = output_layer[lp_first]->p - p_drop;
                        if (output_layer[lp_first]->p_th > p_th)
                        {
                            output_layer[lp_first]->p_th = output_layer[lp_first]->p_th - p_th_drop;
                        }
                    }
                    active_pot[lp_first] = output_layer[lp_first]->p;
                }
            }

            argmax_active = active_pot[0];
            for (lp_active = 0; lp_active < N_SECOND_LAYER; lp_active++)
            {
                if (argmax_active < active_pot[lp_active])
                {
                    argmax_active = active_pot[lp_active];
                    winner = lp_active;
                }
            }

            // Check for spikes and update weights
            for (lp_update_1 = 0; lp_update_1 < N_SECOND_LAYER; lp_update_1++)
            {
                if (lp_update_1 == winner && active_pot[lp_update_1] > output_layer[lp_update_1]->p_th)
                {
                    hyperpolarization(output_layer[lp_update_1], tm);
                    output_layer[lp_update_1]->p_th = output_layer[lp_update_1]->p_th - 1.0;
                    count_spikes[lp_update_1] = count_spikes[lp_update_1] + 1;
                    for (lp_update_2 = 0; lp_update_2 < N_FIRST_LAYER; lp_update_2++)
                    {
                        for (t1 = 0; t1 > (t_back - 1); t1--)
                        {
                            if (0 <= tm + t1 && tm + t1 < t + 1)
                            {
                                if (train[(lp_update_2 * (t + 1)) + (t + t1)] == 1.0)
                                {
                                    rl_t = rl(t1);
                                    synapse[(lp_update_1 * N_FIRST_LAYER) + lp_update_2] = update(synapse[(lp_update_1 * N_FIRST_LAYER) + lp_update_2], rl_t);
                                    synapse_memory[(lp_update_1 * N_FIRST_LAYER) + lp_update_2] = 1;
                                    break;
                                }
                            }
                        }
                        if (synapse_memory[(lp_update_1 * N_FIRST_LAYER) + lp_update_2] != 1)
                        {
                            rl_t = rl(1);
                            synapse[(lp_update_1 * N_FIRST_LAYER) + lp_update_2] = update(synapse[(lp_update_1 * N_FIRST_LAYER) + lp_update_2], rl_t);
                        }
                    }
                    for (lp_inhibit = 0; lp_inhibit < N_SECOND_LAYER; lp_inhibit++)
                    {
                        if (lp_inhibit != winner)
                        {
                            if (output_layer[lp_inhibit]->p > output_layer[lp_inhibit]->p_th)
                            {
                                count_spikes[lp_inhibit] = count_spikes[lp_inhibit] + 1;
                            }
                            inhibit(output_layer[lp_inhibit], tm);
                        }
                    }
                    break;
                }
            }
        }

        for (lp_reset = 0; lp_reset < N_SECOND_LAYER; lp_reset++)
        {
            reset(output_layer[lp_reset]);
        }

        label_neuron[winner] = train_label[train_img];
        printf("Label neuron : %d\n", label_neuron[winner]);

        end = clock();
        time_used = (float)(end - start) / CLOCKS_PER_SEC;

        argmax_count = count_spikes[0];
        learning_neuron = 0;
        for (lp_count = 0; lp_count < N_SECOND_LAYER; lp_count++)
        {
            if (argmax_count < count_spikes[lp_count])
            {
                argmax_count = count_spikes[lp_count];
                learning_neuron = lp_count;
            }
        }
        printf("Learning neuron : %d\n", learning_neuron);
        printf("Learning duration : %.3f sec\n", time_used);
    }

    printf("Reconstructed images : \n");
    for (lp_third = 0; lp_third < N_SECOND_LAYER; lp_third++)
    {
        // if (label_neuron[lp_third] == -1)
        // {
        //     for (int lp_index = 0; lp_index < N_FIRST_LAYER; lp_index++)
        //     {
        //         synapse[(lp_third * N_FIRST_LAYER) + lp_index] = 0.0;
        //     }
        // }

        // Reconstruct weights
        float weight_matrix[PIXEL][PIXEL];
        float img[PIXEL][PIXEL];
        printf("%d.png : \n", img_to_train[lp_third]);
        for (x = 0; x < PIXEL; x++)
        {
            for (y = 0; y < PIXEL; y++)
            {
                weight_matrix[x][y] = synapse[(lp_third * N_FIRST_LAYER) + (x * PIXEL) + y];
                xp[0] = w_min;
                xp[1] = w_max;
                fp[0] = 0.0;
                fp[1] = 255.0;
                img[x][y] = interp(weight_matrix[x][y], xp, fp);
                if(img[x][y] > 25.0){
                    printf("@ ");
                }else{
                    printf("- ");
                }
            }
            printf("\n");
        }
    }

    for (lp_fourth = 0; lp_fourth < N_SECOND_LAYER; lp_fourth++)
    {
        fprintf(f_labels, "%d,", label_neuron[lp_fourth]);
        for (lp_fifth = 0; lp_fifth < N_FIRST_LAYER; lp_fifth++)
        {
            // if (label_neuron[lp_fourth] == -1)
            // {
            //     synapse[(lp_fourth * N_FIRST_LAYER) + lp_fifth] = 0.0;
            // }
            fprintf(f_weights, "%f,", synapse[(lp_fourth * N_FIRST_LAYER) + lp_fifth]);
        }
        fprintf(f_weights, "\n");
    }

    fclose(f_weights);
    fclose(f_labels);

    // Free neuron list
    for (lp_free = 0; lp_free < N_SECOND_LAYER; lp_free++)
    {
        free(output_layer[lp_free]);
    }

    free(synapse);
    free(synapse_memory);

    free(count_spikes);
    free(active_pot);

    free(temp);

    free(img_to_train);

    return 0;
}