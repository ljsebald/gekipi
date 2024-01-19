#ifndef GEKIPI_CONFIG_H
#define GEKIPI_CONFIG_H

#include "pico.h"

static const uint gpio_pins[] = {
/*  LR  LG  LB  RR  RG  RB  LM  RM  LW  RW   S   T */
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 21, 22
};

static const int gpio_pressed[] = {
/*  LR  LG  LB  RR  RG  RB  LM  RM  LW  RW   S   T */
     0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0
};

static const int gpio_pulls[] = {
/*  LR  LG  LB  RR  RG  RB  LM  RM  LW  RW   S   T */
     1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1
};

#endif /* !GEKIPI_CONFIG_H */
