#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "apollo3p.h"
#include <core_cm4.h>

#include "classify.h"
#include "time.h"

#define EVB_LED0 10
#define EVB_LED1 30
#define EVB_LED2 15
#define EVB_LED3 14
#define EVB_LED4 17

//
// Keep the LED pattern array in FLASH instead of SRAM.
//
static const uint8_t led_pattern[44] =
    {
        //
        // Binary count up
        //
        1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 16,

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
        0x15, 0x0A, 0x15, 0x0A, 0x15, 0x0A, 0x15, 0x0A, 0x15, 0x00};

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

void setSleepMode(int bSetDeepSleep)
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

    clock_t start, end;
    double time_used;

    start = clock();

    int classified_image;
    classified_image = classify_image();

    end = clock();
    time_used = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Time for inference: %lf\n", time_used);
    printf("Classification Done: %d", classified_image);

    switch (classified_image)
    {
    case 0:
        activate(0); // Number 0
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
