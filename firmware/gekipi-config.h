/*
    GekiPi -- An arcade-style controller firmware

    Copyright (C) 2023-2024 Lawrence Sebald

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#ifndef GEKIPI_CONFIG_H
#define GEKIPI_CONFIG_H

#include "pico.h"
#include "hardware/i2c.h"

#define USB_VID     0x1209
#define USB_PID     0x63C1

/* What GPIO each button uses */
static const uint gpio_pins[] = {
/*  LR  LG  LB  RR  RG  RB  LM  RM  LW  RW   S   T */
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 21, 22
};

/* What level implies pressed. 0 = Low, 1 = High */
static const int gpio_pressed[] = {
/*  LR  LG  LB  RR  RG  RB  LM  RM  LW  RW   S   T */
     0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0
};

/* Set to > 0 for pull up, < 0 for pull down, = 0 for no pull */
static const int gpio_pulls[] = {
/*  LR  LG  LB  RR  RG  RB  LM  RM  LW  RW   S   T */
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
};

#define STICK_GPIO          28
#define STICK_ADC_CHANNEL   (STICK_GPIO - 26)
#define STICK_INVERTED      1

/* Number of LEDs in each WAD */
#define NUM_WAD_LEDS        6

/* Total number of LEDs in the controller. One per button + one for NFC +
   the above number per WAD */
#define TOTAL_LEDS          (9 + (NUM_WAD_LEDS * 2))

#define LED_PIN             20

#define NFC_I2C             i2c0
#define NFC_SDA             16
#define NFC_SCL             17
#define NFC_IRQ             18

#endif /* !GEKIPI_CONFIG_H */
