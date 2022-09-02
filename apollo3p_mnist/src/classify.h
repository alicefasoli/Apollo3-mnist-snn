#ifndef __CLASSIFY_H__
#define __CLASSIFY_H__

#include "network_values.h"
#include "parameters.h"
#include "actual_image.h"

#include <math.h>

typedef struct Neuron
{
    double p;
    double p_th;

    int t_rest;
    int t_ref;
} Neuron;

double 
interp(double x, double xp[2], double fp[2])
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

int
classify_image(){
		int i, j, k, m, n, tm;
		int return_value;
	
		double train[N_FIRST_LAYER * (T + 1)];
		double pot[PIXEL * PIXEL];

		int count_spikes[N_SECOND_LAYER];
		double active_pot[N_SECOND_LAYER];
	
		Neuron output_layer[N_SECOND_LAYER]; // Creating hidden layer of neurons
    for (i = 0; i < N_SECOND_LAYER; i++)
    {
				// Initialize neurons
        output_layer[i].p = p_rest;
				output_layer[i].p_th = p_th;
				output_layer[i].t_rest = -1;
				output_layer[i].t_ref = 15;
				//
				active_pot[i] = 0.0;
				count_spikes[i] = 0;
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
										if ((i + ran[m]) >= 0 && (i + ran[m]) <= (PIXEL - 1) && (j + ran[n]) >= 0 && (j + ran[n]) <= (PIXEL - 1))
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
		
		// Spike train encoding
		double xp[2];
		double fp[2] = {1.0, 50.0};
		double freq, time_period, time_of_spike;
		for (i = 0; i < N_FIRST_LAYER; i++) // loop for potential list
		{
				xp[0] = min;
				xp[1] = max;
				freq = interp(pot[i], xp, fp);

				time_period = ceil((double)T / freq);

				time_of_spike = time_period;

				for (j = 0; j < (T + 1); j++)
				{
						train[(i * (T + 1)) + j] = 0.0;
				}

				if (pot[i] > 0)
				{
						while (time_of_spike < T + 1)
						{
								train[(i * (T + 1)) + (int)time_of_spike] = 1.0;
								time_of_spike = time_of_spike + time_period;
						}
				}
		}
		
		int winner = 0;
		double dotProduct;
		double argmax_active;
		// long double tmp;
		for (tm = 0; tm < (T + 1); tm++)
		{
				for (i = 0; i < N_SECOND_LAYER; i++)
				{
						if (output_layer[i].t_rest < tm)
						{
								dotProduct = 0.0;
								for (j = 0; j < N_FIRST_LAYER; j++)
								{
										dotProduct = dotProduct + (synapse[(i * N_FIRST_LAYER) + j] * train[(j * (T + 1)) + tm]);
								}
								// output_layer[i].p = output_layer[i].p + dotProduct;

								if (output_layer[i].p > p_rest)
								{
										output_layer[i].p = output_layer[i].p - p_drop;
								}
						}
						active_pot[i] = output_layer[i].p;
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
						if (i == winner && active_pot[i] > output_layer[i].p_th)
						{
								// Hyperpolarization
								output_layer[i].p = p_hyperpolarization;
								output_layer[i].t_rest = tm + output_layer[i].t_ref;
								//
								output_layer[i].p_th = output_layer[i].p_th - 1.0;
								count_spikes[i] = count_spikes[i] + 1;

								for (k = 0; k < N_SECOND_LAYER; k++)
								{
										if (k != winner)
										{
												if (output_layer[k].p > output_layer[k].p_th)
												{
														count_spikes[k] = count_spikes[k] + 1;
												}
												// Inhibition
												output_layer[i].p = p_inhibit;
												output_layer[i].t_rest = tm + output_layer[i].t_ref;
												//
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
		
		int predicted_label = neuron_labels[learning_neuron];
		if(predicted_label == actual_label)// Correct classified
		{ 
			return_value = predicted_label;
		}else // Wrong classified
		{ 
			return_value = 16;
		}
		
		return return_value;
}


#endif
