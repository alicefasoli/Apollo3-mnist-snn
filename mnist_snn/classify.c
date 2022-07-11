#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "parameters.h"
#include "neuron.h"
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
    FILE *f_weights;
    f_weights = fopen("weights.txt", "r");

    FILE *f_labels;
    f_labels = fopen("labels.txt", "r");

    if (f_labels == NULL || f_weights == NULL)
    {
        return 1;
    }

    int *neuron_labels = (int *)calloc(N_SECOND_LAYER, sizeof(int));
    float *synapse = (float *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(float));

    int read_labels = 0;
    int record_labels = 0;
    do
    {
        read_labels = fscanf(f_labels, "%d,", &neuron_labels[record_labels]);
        if (read_labels != 1 && !feof(f_labels))
        {
            printf("File label format incorrect\n");
            return 1;
        }
        record_labels++;
    } while (!feof(f_labels));

    printf("Labels : %d\n", record_labels);

    fclose(f_labels);

    int read_weights = 0;
    int record_row_weights = 0;
    int record_column_weights = 0;
    do
    {
        read_weights = fscanf(f_weights, "%f,", &synapse[(record_row_weights * N_FIRST_LAYER) + record_column_weights]);
        if (read_weights != 1 && !feof(f_weights))
        {
            printf("File format incorrect\n");
            return 1;
        }
        record_column_weights++;
        if (record_column_weights % N_FIRST_LAYER == 0)
        {
            record_row_weights++;
            record_column_weights = 0;
        }
    } while (!feof(f_weights));

    printf("Column : %d\n", record_column_weights);
    printf("Row : %d\n", record_row_weights);

    fclose(f_weights);

    // for (int i = 0; i < N_SECOND_LAYER; i++)
    // {
    //     // printf("%d \n", neuron_labels[i]);
    //     printf("[");
    //     for (int j = 0; j < N_FIRST_LAYER; j++)
    //     {
    //         printf("%f,", synapse[(i * N_FIRST_LAYER) + j]);
    //     }
    //     printf("]\n");
    // }

    float train[N_FIRST_LAYER * (t + 1)];
    float actual_img[PIXEL][PIXEL];

    float *pot = (float *)calloc(PIXEL * PIXEL, sizeof(float));
    float *temp = (float *)calloc(t + 1, sizeof(float));
    
    Neuron *output_layer[N_SECOND_LAYER]; // Creating hidden layer of neurons
    // float *synapse = (float *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(float));
    for (int i = 0; i < N_SECOND_LAYER; i++)
    {
        output_layer[i] = initial();
        // for (int j = 0; j < N_FIRST_LAYER; j++)
        // {
        //     synapse[(i * N_FIRST_LAYER) + j] = learned_weights[(i * N_FIRST_LAYER) + j];
        // }
    }

    clock_t start, end;
    double time_used;

    load_mnist();

    int n_test, lpPx_x, lpPx_y, lpPx_x_rf, lpPx_y_rf, m, n, lp_pot, tmp, lpTime_tmp, lp_free, j, tm, lp_first, lp_dotProd, lp_active, lp_second, lp_third, lp_count;

    int winner, learning_neuron, argmax_count;

    float min, max, sum, dotProduct, argmax_active;
    float xp[2], fp[2], freq, time_period, time_of_spike;

    double rl_t;

    double predicted_class = 0.0;
    double cont_predicted_correct = 0.0;

    int *count_spikes = (int *)calloc(N_SECOND_LAYER, sizeof(int));
    float *active_pot = (float *)calloc(N_SECOND_LAYER, sizeof(float));

    // printf("Reconstructed images : \n");
    // for (int lp_reconst = 0; lp_reconst < N_SECOND_LAYER; lp_reconst++)
    // {
    //     // Reconstruct weights
    //     float weight_matrix[PIXEL][PIXEL];
    //     float img[PIXEL][PIXEL];
    //     printf("%d.png: \n", lp_reconst);
    //     for (int x = 0; x < PIXEL; x++)
    //     {
    //         for (int y = 0; y < PIXEL; y++)
    //         {
    //             weight_matrix[x][y] = synapse[(lp_reconst * N_FIRST_LAYER) + (x * PIXEL) + y];
    //             xp[0] = w_min;
    //             xp[1] = w_max;
    //             fp[0] = 0.0;
    //             fp[1] = 255.0;
    //             img[x][y] = interp(weight_matrix[x][y], xp, fp);
    //             if (img[x][y] > 200.0)
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

    for (n_test = 100; n_test < 300; n_test++)
    {
        start = clock();
        printf("%d : \n", n_test);

        for (j = 0; j < N_SECOND_LAYER; j++)
        {
            count_spikes[j] = 0;
            active_pot[j] = 0.0;
        }

        for (lpPx_x = 0; lpPx_x < PIXEL; lpPx_x++) // loop for pixel in axes x
        {
            // printf("[");
            for (lpPx_y = 0; lpPx_y < PIXEL; lpPx_y++) // loop for pixel in axes y
            {
                // actual_img[lpPx_x][lpPx_y] = test_image[n_test][(lpPx_x * PIXEL) + lpPx_y];
                if (test_image[n_test][(lpPx_x * PIXEL) + lpPx_y] == 1.0)
                {
                    actual_img[lpPx_x][lpPx_y] = 255.0;
                }
                else
                {
                    actual_img[lpPx_x][lpPx_y] = test_image[n_test][(lpPx_x * PIXEL) + lpPx_y] * 256.0;
                }
                // printf("%2.f ", actual_img[lpPx_x][lpPx_y]);
            }
            // printf("]\n");
        }

        for (int i = 0; i < N_SECOND_LAYER; i++)
        {
            reset(output_layer[i]);
        }

        // Receptive field convolution
        min = 0.0;
        max = -10000.0;
        for (lpPx_x_rf = 0; lpPx_x_rf < PIXEL; lpPx_x_rf++) // loop for receptive field convolution
        {
            // printf("[");
            for (lpPx_y_rf = 0; lpPx_y_rf < PIXEL; lpPx_y_rf++)
            {
                sum = 0.0;
                for (m = 0; m < 5; m++)
                {
                    for (n = 0; n < 5; n++)
                    {
                        if ((lpPx_x_rf + (float)ran[m]) >= 0 && (lpPx_x_rf + (float)ran[m]) <= (PIXEL - 1) && (lpPx_y_rf + (float)ran[n]) >= 0 && (lpPx_y_rf + (float)ran[n]) <= (PIXEL - 1))
                        {
                            sum = sum + w[ox + ran[m]][oy + ran[n]] * (actual_img[lpPx_x_rf + ran[m]][lpPx_y_rf + ran[n]] / 255.0);
                        }
                    }
                }
                pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf] = sum;
                // printf("%2.4f ", pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf]);

                if (min > pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf])
                {
                    min = pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf];
                }

                if (max < pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf])
                {
                    max = pot[(lpPx_x_rf * PIXEL) + lpPx_y_rf];
                }
            }
            // printf("]\n");
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
            }
        }

        winner = 0;
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
                    }
                }
                active_pot[lp_first] = output_layer[lp_first]->p;
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

            for (lp_second = 0; lp_second < N_SECOND_LAYER; lp_second++)
            {
                if (lp_second == winner)
                {
                    if (active_pot[lp_second] > output_layer[lp_second]->p_th)
                    {
                        count_spikes[lp_second] = count_spikes[lp_second] + 1;
                        output_layer[lp_second]->p_th = output_layer[lp_second]->p_th - 1.0;
                        hyperpolarization(output_layer[lp_second], tm);
                        for (lp_third = 0; lp_third < N_SECOND_LAYER; lp_third++)
                        {
                            if (lp_third != lp_second)
                            {
                                if (output_layer[lp_third]->p > output_layer[lp_third]->p_th)
                                {
                                    inhibit(output_layer[lp_third], tm);
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

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

        printf("Predicted class : %d\n", neuron_labels[learning_neuron]);
        predicted_class = predicted_class + 1.0;
        printf("Actual class : %d\n", test_label[n_test]);
        if (neuron_labels[learning_neuron] == test_label[n_test])
        {
            cont_predicted_correct = cont_predicted_correct + 1.0;
        }
    }

    double accurancy = -1.0;
    if (predicted_class != 0.0)
    {
        accurancy = (cont_predicted_correct / predicted_class) * 100.0;
    }
    printf("Accurancy : %2.1f \n", accurancy);

    // Free neuron list
    for (lp_free = 0; lp_free < N_SECOND_LAYER; lp_free++)
    {
        free(output_layer[lp_free]);
    }

    free(neuron_labels);
    // free(synapse);

    // free(prediction_count);

    free(active_pot);
    free(count_spikes);

    free(pot);

    free(temp);
    free(synapse);

    return 0;
}
