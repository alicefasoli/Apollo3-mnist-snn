#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "apollo3p.h"
#include <core_cm4.h>

#include "parameters.h"

#include "actual_image.h"
#include "network_values.h"

#include "math.h"

#define EVB_LED0 10
#define EVB_LED1 30
#define EVB_LED2 15
#define EVB_LED3 14
#define EVB_LED4 17

static double train[N_FIRST_LAYER * (T + 1)];
double pot[PIXEL * PIXEL];

int count_spikes[N_SECOND_LAYER];
double active_pot[N_SECOND_LAYER];

typedef struct
{
	double p;
	double p_th;

	int t_rest;
	int t_ref;
} Neuron;

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

void hyperpolarization(Neuron *n, int _t)
{
    n->p = p_hyperpolarization;
    n->t_rest = _t + n->t_ref;
}

void inhibit(Neuron *n, int _t)
{
    n->p = p_inhibit;
    n->t_rest = _t + n->t_ref;
}

//
// Keep the LED pattern array in FLASH instead of SRAM.
//
static const uint8_t led_pattern[44] =
    {
        //
        // Binary count up
        //
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 16,

        //
        // Rotote left pattern
        //
        0x01, 0x02, 0x04, 0x08, 0x10, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00,

        //
        // Rotate right pattern
        //
        0x10, 0x08, 0x04, 0x02, 0x01, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00,

        //
        // Springing pattern
        //
        0x15, 0x0A, 0x15, 0x0A, 0x15, 0x0A, 0x15, 0x0A, 0x15, 0x00
		};
		
//*****************************************************************************
//
// Set or clear an LED.
// The LEDs on the EVB are common anode (anodes tied high).
// They are turned on when the transistor is turned on (i.e. high).
// Therefore, they are turned on when the GPIO is set.
// Likewise, they are turned off when the GPIO is cleared.
//
// NOTE: Polarity was opposite with the old, and now deprecated, EVK.
//
//*****************************************************************************
static void
LED_on(uint32_t ui32LED, bool bOn)
{
    uint32_t ui32Mask;

    if (ui32LED <= 31)
    {
        ui32Mask = 0x01 << (ui32LED - 0);

        if (bOn)
        {
            GPIO->WTSA = ui32Mask;
        }
        else
        {
            GPIO->WTCA = ui32Mask;
        }
    }
    else if (ui32LED <= 49)
    {
        ui32Mask = 0x01 << (ui32LED - 32);

        if (bOn)
        {
            GPIO->WTSB = ui32Mask;
        }
        else
        {
            GPIO->WTCB = ui32Mask;
        }
    }
    else
    {
        // ERROR.
        while (1)
            ;
    }

} // LED_on()

//*****************************************************************************
//
// Set a PADREG FNCSEL field.
//
//*****************************************************************************
static void
padreg_funcsel_set(uint32_t ui32GPIOnum, uint32_t ui32Func)
{
    uint32_t ui32Shift;
    uint32_t volatile *pui32Reg;

    //
    // Unlock writes to the GPIO and PAD configuration registers.
    //
    GPIO->PADKEY = 0x73;

    //
    // Determine and configure the PADREG.
    // Each PADREG configures 4 GPIOs, each field divided into bytes.
    //
    ui32Shift = (ui32GPIOnum & 0x3) * 8;

    //
    // Since there are 4 GPIOs per PADREG, the GPIO number also happens to
    // be a byte offset to the PADREG address. Since we have a word ptr, we
    // need to make it a word offset (so divide by 4).
    //
    pui32Reg = &(GPIO->PADREGA) + (ui32GPIOnum / 4);

    //
    // Set the padreg value given by the caller.
    //
    *pui32Reg &= ~(0xFF << ui32Shift);
    *pui32Reg |= ui32Func << ui32Shift;

    //
    // Lock PAD configuration registers.
    //
    GPIO->PADKEY = 0;
}

//*****************************************************************************
//
// Write a GPIO CFG field.
//
//*****************************************************************************
static void
gpio_cfg_set(uint32_t ui32GPIOnum, uint32_t ui32CFG)
{
    uint32_t ui32Shift;
    uint32_t volatile *pui32Reg;

    //
    // Unlock writes to the GPIO and PAD configuration registers.
    //
    GPIO->PADKEY = 0x73;

    //
    // Configure the GPIO as push pull outputs for use with an LED.
    // Each GPIOCFG configures 8 GPIOs, each divided into nibbles.
    //
    ui32Shift = (ui32GPIOnum & 0x7) * 4;
    pui32Reg = &(GPIO->CFGA) + (ui32GPIOnum / 8);
    *pui32Reg &= ~(0xF << ui32Shift);

    //
    // The OUTCFG field lsb is 1 bit into the CFG field.
    // OUTCFG is 2 bits wide.
    //
    *pui32Reg |= ((ui32CFG & 0x3) << ui32Shift);

    //
    // Lock PAD configuration registers.
    //
    GPIO->PADKEY = 0;

} // gpio_cfg_set()

//*****************************************************************************
//
// Configure a GPIO for use with an LED.
//
//*****************************************************************************
void LED_gpio_cfg(uint32_t ui32GPIOnum)
{
    if (ui32GPIOnum > 49)
    {
        //
        // Error
        //
        while (1)
            ;
    }

    //
    // Configure the PAD for GPIO (FUNCSEL=3).
    // The 8-bit PADREG bitfields are generally:
    //  [7:6] = Pullup resistor selection.
    //  [5:3] = Function select (3 = GPIO).
    //  [2]   = Drive strength
    //  [1]   = INPEN
    //  [0]   = Pullup enable
    //
    padreg_funcsel_set(ui32GPIOnum, 0x18);

    //
    // Configure the GPIO output for PUSHPULL (OUTCFG=1).
    // CFG is 4 bits:
    // [3]   = INTD, 0=INTLH, 1=INTHL
    // [2:1] = OUTCFG. 0=DIS, 1=PUSHPULL, 2=open drain, 3=tristate
    // [0]   = READ
    //
    gpio_cfg_set(ui32GPIOnum, 0x2);

} // LED_gpio_cfg()

void GPIO_init()
{
    //
    // Configure pads for LEDs as GPIO functions.
    //
    //
    LED_gpio_cfg(EVB_LED0);
    LED_gpio_cfg(EVB_LED1);
    LED_gpio_cfg(EVB_LED2);
    LED_gpio_cfg(EVB_LED3);
#ifdef EVB_LED4
    LED_gpio_cfg(EVB_LED4);
#endif // EVB_LED4

    //
    // While we're at it, also configure GPIO41 for SWO.
    //
    padreg_funcsel_set(41, 2);
}

void setSleepMode(uint32_t bSetDeepSleep)
{
    if (bSetDeepSleep)
    {
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    }
    else
    {
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }

} // setSleepMode()

void all(bool flag)
{
    //
    // Initialize the LEDs to all on.
    //
    LED_on(EVB_LED0, flag);
    LED_on(EVB_LED1, flag);
    LED_on(EVB_LED2, flag);
    LED_on(EVB_LED3, flag);
#ifdef EVB_LED4
    LED_on(EVB_LED4, flag);
#endif // EVB_LED4
}

void activate(uint32_t value)
{
    uint32_t ui32Value;

    ui32Value = led_pattern[(value - 1) & 0x0000003F];

    LED_on(EVB_LED0, (bool)(ui32Value & 0x00000001));
    LED_on(EVB_LED1, (bool)(ui32Value & 0x00000002));
    LED_on(EVB_LED2, (bool)(ui32Value & 0x00000004));
    LED_on(EVB_LED3, (bool)(ui32Value & 0x00000008));
#ifdef EVB_LED4
    LED_on(EVB_LED4, (bool)(ui32Value & 0x00000010));
#endif // EVB_LED4
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int main(void)
{
    GPIO_init();
		int i, j, k, m, n, tm;

    // Initialize matrix
		int return_value;

		Neuron neu0;
		neu0.p = p_rest;
		neu0.p_th = p_th;
		neu0.t_rest = -1;
		neu0.t_ref = 15;

		Neuron neu1;
		neu1.p = p_rest;
		neu1.p_th = p_th;
		neu1.t_rest = -1;
		neu1.t_ref = 15;

		Neuron neu2;
		neu2.p = p_rest;
		neu2.p_th = p_th;
		neu2.t_rest = -1;
		neu2.t_ref = 15;

		Neuron neu3;
		neu3.p = p_rest;
		neu3.p_th = p_th;
		neu3.t_rest = -1;
		neu3.t_ref = 15;

		Neuron neu4;
		neu4.p = p_rest;
		neu4.p_th = p_th;
		neu4.t_rest = -1;
		neu4.t_ref = 15;

		Neuron neu5;
		neu5.p = p_rest;
		neu5.p_th = p_th;
		neu5.t_rest = -1;
		neu5.t_ref = 15;

		Neuron neu6;
		neu6.p = p_rest;
		neu6.p_th = p_th;
		neu6.t_rest = -1;
		neu6.t_ref = 15;

		Neuron neu7;
		neu7.p = p_rest;
		neu7.p_th = p_th;
		neu7.t_rest = -1;
		neu7.t_ref = 15;

		Neuron neu8;
		neu8.p = p_rest;
		neu8.p_th = p_th;
		neu8.t_rest = -1;
		neu8.t_ref = 15;

		Neuron neu9;
		neu9.p = p_rest;
		neu9.p_th = p_th;
		neu9.t_rest = -1;
		neu9.t_ref = 15;
		
		Neuron *output_layer[10]; // Creating hidden layer of neurons
		output_layer[0] = &neu0;
		output_layer[1] = &neu1;
		output_layer[2] = &neu2;
		output_layer[3] = &neu3;
		output_layer[4] = &neu4;
		output_layer[5] = &neu5;
		output_layer[6] = &neu6;
		output_layer[7] = &neu7;
		output_layer[8] = &neu8;
		output_layer[9] = &neu9;

    for (int i = 0; i < N_SECOND_LAYER; i++)
    {
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

		// Spike train encoding
		double xp[2];
		double fp[2] = {1.0, 50.0};
		double freq, time_period, time_of_spike;
		for (i = 0; i < N_FIRST_LAYER; i++) // loop for potential list
		{
				xp[0] = min;
				xp[1] = max;
				freq = interp(pot[i], xp, fp);

				time_period = ceil(100.0 / freq);

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
        
		int predicted_label = neuron_labels[learning_neuron];
		
		if(actual_label == predicted_label){
			if(predicted_label == 0){
				return_value = 0;
			}else if(predicted_label == 1){
				return_value = 1;
			}else if(predicted_label == 2){
				return_value = 2;
			}else if(predicted_label == 3){
				return_value = 3;
			}else if(predicted_label == 4){
				return_value = 4;
			}else if(predicted_label == 5){
				return_value = 5;
			}else if(predicted_label == 6){
				return_value = 6;
			}else if(predicted_label == 7){
				return_value = 7;
			}else if(predicted_label == 8){
				return_value = 8;
			}else if(predicted_label == 9){
				return_value = 9;
			}
		}else{
			return_value = 16;
		}
		
	
    switch (return_value)
    {
    case 0:
        activate(10); // Number 0
        break;
    case 1:
        activate(1); // Number 1
        break;
    case 2:
        activate(2); // Number 2
        break;
    case 3:
        activate(3); // Number 3
        break;
    case 4:
        activate(4); // Number 4
        break;
    case 5:
        activate(5); // Number 5
        break;
    case 6:
        activate(6); // Number 6
        break;
    case 7:
        activate(7); // Number 7
        break;
    case 8:
        activate(8); // Number 8
        break;
    case 9:
        activate(9); // Number 9
        break;
    case 16:
        activate(16); // Wrong classification
        break;
    default:
        all(true);
        break;
    }
		
		while (1)
    {
        //
        // Sleep here
        //
        __WFI();
    }
		
}
