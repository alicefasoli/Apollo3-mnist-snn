#include <stdio.h>

#include <time.h>
#include <math.h>
#include <string.h>

#include "parameters.h"
#include "neuron.h"
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
    int i, j, k, m, n, tm, n_test;

    double predicted_class = 0.0;
    double cont_predicted_correct = 0.0;

    FILE *f_weights;
    f_weights = fopen("weights.txt", "r");

    FILE *f_labels;
    f_labels = fopen("labels.txt", "r");

    if (f_labels == NULL || f_weights == NULL)
    {
        return 1;
    }

    int *neuron_labels = (int *)calloc(N_SECOND_LAYER, sizeof(int));
    double *synapse = (double *)calloc(N_SECOND_LAYER * N_FIRST_LAYER, sizeof(double));

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
    fclose(f_labels);

    int read_weights = 0;
    int record_row_weights = 0;
    int record_column_weights = 0;
    do
    {
        read_weights = fscanf(f_weights, "%lf,", &synapse[(record_row_weights * N_FIRST_LAYER) + record_column_weights]);
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
    fclose(f_weights);

    // Initialize matrix
    double train[N_FIRST_LAYER * (t + 1)];
    double actual_img[PIXEL][PIXEL];

    int *count_spikes = (int *)calloc(N_SECOND_LAYER, sizeof(int));
    double *active_pot = (double *)calloc(N_SECOND_LAYER, sizeof(double));
    double *pot = (double *)calloc(PIXEL * PIXEL, sizeof(double));

    Neuron *output_layer[N_SECOND_LAYER]; // Creating hidden layer of neurons
    for (int i = 0; i < N_SECOND_LAYER; i++)
    {
        output_layer[i] = initial();
    }

    load_mnist(); // Load mnist dataset

    for (n_test = 0; n_test < NUM_TEST; n_test++)
    {
        printf("Input image: \n");
        for (i = 0; i < PIXEL; i++) // loop for pixel in axes x
        {
            printf("[");
            for (j = 0; j < PIXEL; j++) // loop for pixel in axes y
            {
                actual_img[i][j] = test_image[n_test][(i * PIXEL) + j];
                printf("%2.f ", actual_img[i][j]);
            }
            printf("]\n");
        }

        for (int i = 0; i < N_SECOND_LAYER; i++)
        {
            reset(output_layer[i]);
            count_spikes[i] = 0;
            active_pot[i] = 0.0;
        }

        // Receptive field convolution
        double min = 0.0;
        double max = -10000.0;
        double sum;
        for (i = 0; i < PIXEL; i++) // loop for receptive field convolution
        {
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
        double dotProduct, argmax_active;
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
                    }
                }
                active_pot[i] = output_layer[i]->p;
            }

            argmax_active = active_pot[0];
            for (j = 0; j < N_SECOND_LAYER; j++)
            {
                if (argmax_active < active_pot[j])
                {
                    argmax_active = active_pot[j];
                    winner = j;
                }
            }

            for (i = 0; i < N_SECOND_LAYER; i++)
            {
                if (i == winner && active_pot[i] > output_layer[i]->p_th)
                {
                    hyperpolarization(output_layer[i], tm);
                    output_layer[i]->p_th = output_layer[i]->p_th - 1.0;
                    count_spikes[i] = count_spikes[i] + 1;

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
        printf("Num : %d\n", n_test);
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

    // Free lists
    for (j = 0; j < N_SECOND_LAYER; j++)
    {
        free(output_layer[j]);
    }

    free(neuron_labels);
    free(synapse);

    free(active_pot);
    free(count_spikes);

    free(pot);

    return 0;
}
