#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#include "usb_descriptors.h"
#include "gekipi-config.h"

static void init_gpios(void) {
    int i;

    adc_init();

    for(i = 0; i < 12; ++i) {
        gpio_init(gpio_pins[i]);
        gpio_set_dir(gpio_pins[i], GPIO_IN);
        if(gpio_pulls[i])
            gpio_pull_up(gpio_pins[i]);
        else
            gpio_pull_down(gpio_pins[i]);
    }

    /* Enable PWM mode for the SMPS to (hopefully) limit ADC noise */
    gpio_init(23);
    gpio_set_dir(23, GPIO_OUT);
    gpio_put(23, 1);

    /* Set up the ADC for the stick */
    adc_gpio_init(STICK_GPIO);
}

static inline uint16_t read_buttons(void) {
    int i;
    bool st;
    uint16_t rv = 0;
    uint32_t all = gpio_get_all();

    for(i = 0; i < 12; ++i) {
        if((all & (1 << gpio_pins[i])) == gpio_pressed[i])
            rv |= (1 << i);
    }

    return rv;
}

static inline uint16_t read_stick(void) {
    adc_select_input(STICK_ADC_CHANNEL);
    return (uint16_t)adc_read();
}

static void hid_task(void) {
    const uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;
    uint16_t state;

    // Refresh at most once every 1ms.
    if(board_millis() - start_ms < interval_ms)
        return;

    start_ms += interval_ms;
    state = read_buttons();

    if(tud_suspended() && state) {
        tud_remote_wakeup();
    }
    else {
        gekipi_gamepad_report_t report = { .x = 0, .y = 0, .buttons = state };

        if(!tud_hid_ready())
            return;

        report.x = read_stick();
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
    }
}

uint16_t tud_hid_get_report_cb(uint8_t iface, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    // TODO not Implemented
    (void)iface;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t iface, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void)iface;

    if(report_type != HID_REPORT_TYPE_OUTPUT)
        return;

    if(report_id != REPORT_ID_INOUT)
        return;

    // echo back anything we received from host
    tud_hid_report(0, buffer, bufsize);
}

int main(void) {
    stdio_init_all();
    board_init();
    tusb_init();
    init_gpios();

    for(;;) {
        tud_task();
        hid_task();
    }

    return 0;
}

