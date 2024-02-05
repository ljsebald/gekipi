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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "pico/util/queue.h"

#include "usb_descriptors.h"
#include "gekipi-config.h"
#include "ws2812.pio.h"
#include "pn532_pico.h"

static uint32_t led_values[TOTAL_LEDS];
#define LED_RGB(r, g, b) ((r << 16) | (g << 24) | (b << 8))

static queue_t mpq;
static PN532 nfc;

static bool nfc_initted = false;
static uint8_t nfc_report[15];

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

    /* Set up i2c for the NFC reader */
    i2c_init(NFC_I2C, 100 * 1000);
    gpio_set_function(NFC_SDA, GPIO_FUNC_I2C);
    gpio_set_function(NFC_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(NFC_SDA);
    gpio_pull_up(NFC_SCL);

    gpio_init(NFC_IRQ);
    gpio_set_dir(NFC_IRQ, GPIO_IN);
    gpio_pull_up(NFC_IRQ);

    PN532_I2C_Init(&nfc, NFC_I2C, PN532_NO_PIN, PN532_NO_PIN);
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

    if(board_millis() - start_ms < interval_ms)
        return;

    start_ms += interval_ms;
    state = read_buttons();

    if(tud_suspended() && state) {
        tud_remote_wakeup();
    }
    else {
        gekipi_gamepad_report_t report = { .x = 0, .y = 0, .buttons = state };

        if(!tud_hid_n_ready(GAMEPAD_INSTANCE))
            return;

        report.x = read_stick();
        tud_hid_n_report(GAMEPAD_INSTANCE, 0, &report, sizeof(report));
    }
}

uint16_t tud_hid_get_report_cb(uint8_t iface, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
    (void)iface;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

#define TASK_LED_UPDATE     1
#define TASK_NFC_INIT       2
#define TASK_NFC_POLL       3

#define REPORT_LED_UPDATE   1
#define REPORT_NFC_POLL     2
#define REPORT_NFC_REQUEST  3

void tud_hid_set_report_cb(uint8_t intf, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    uint8_t tmp;

    if(intf == NFC_INSTANCE && report_id == 0 && report_type == 0) {
        if(!tud_hid_n_ready(NFC_INSTANCE) || bufsize < 1)
            return;

        switch(buffer[0]) {
            case REPORT_LED_UPDATE:
                printf("Got led update of length %d\n", (int)bufsize);
                break;

            case REPORT_NFC_POLL:
                printf("Got NFC poll request\n");
                tmp = TASK_NFC_POLL;
                queue_add_blocking(&mpq, &tmp);
                break;

            case REPORT_NFC_REQUEST:
                printf("Got NFC request of length %d\n", (int)bufsize);
                break;

            default:
                printf("Got unknown set report: %d\n", (int)buffer[0]);
        }
    }
}

void nfc_init(void) {
    int err;
    uint8_t version[4];

    printf("Initializing PN532...\n");

    err = PN532_GetFirmwareVersion(&nfc, version);
    if(err != PN532_STATUS_OK) {
        printf("Failed to get PN532 firmware version\n");
        return;
    }

    printf("Firmware version: %02x%02x%02x%02x\n", version[0], version[1],
           version[2], version[3]);

    err = PN532_SamConfiguration(&nfc);
    if(err != PN532_STATUS_OK) {
        printf("Failed to set SAM configuration\n");
        return;
    }

    nfc_initted = true;
}

/* Use the second core to update the LEDs so that we don't disturb input by
   doing it in the main core. Is this a waste of the core? Probably. But it's
   there, so I might as well... */
void core1_task(void) {
    int i;
    uint8_t task;
    bool polling_nfc = false, polling_felica = false;
    uint32_t poll_timer, last_switch, tmp;

    for(;;) {
        if(queue_try_remove(&mpq, &task)) {
            switch(task) {
                case TASK_LED_UPDATE:
                    for(i = 0; i < TOTAL_LEDS; ++i) {
                        pio_sm_put_blocking(pio0, 0, led_values[i]);
                    }

                    break;

                case TASK_NFC_INIT:
                    if(!nfc_initted) {
                        nfc_init();
                    }

                    break;

                case TASK_NFC_POLL:
                    printf("Beginning NFC poll....\n");
                    if(nfc_initted) {
                        polling_nfc = true;
                        poll_timer = board_millis() + 30 * 1000;
                        last_switch = 0;
                    }

                    break;
            }
        }

        if(polling_nfc) {
            if(gpio_get(NFC_IRQ) == false) {
                printf("IRQ on core 1!\n");

                if(polling_felica) {
                    uint8_t idm[8], pmm[8];
                    uint16_t syscode;

                    printf("Got a FeliCa card!\n");
                    PN532_FelicaGet(&nfc, idm, pmm, &syscode, 1);
                    printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
                           idm[0], idm[1], idm[2], idm[3], idm[4], idm[5],
                           idm[6], idm[7]);
                }
                else {
                    uint8_t idm[7];

                    printf("Got a MiFare card!\n");
                    PN532_MifareGet(&nfc, idm, 1);
                    printf("%02x %02x %02x %02x %02x %02x %02x\n",
                           idm[0], idm[1], idm[2], idm[3], idm[4], idm[5],
                           idm[6]);
                }

                polling_nfc = false;
            }
            else {
                tmp = board_millis();

                if(tmp > poll_timer) {
                    printf("Polling aborted.\n");
                    polling_nfc = false;
                }
                else if(tmp > last_switch + 5000) {
                    if(polling_felica) {
                        printf("Beginning mifare listen\n");
                        PN532_MifareListen(&nfc, PN532_MIFARE_ISO14443A, 1);
                    }
                    else {
                        printf("Beginning felica listen\n");
                        PN532_FelicaListen(&nfc, FELICA_POLL_SYSTEM_CODE_ANY,
                                           FELICA_POLL_SYSTEM_CODE, 1);
                    }
                    polling_felica = !polling_felica;
                    last_switch = tmp;
                }
            }
        }
    }
}

int main(void) {
    uint offset;
    int i;
    uint8_t data;

    stdio_init_all();
    board_init();
    tusb_init();
    init_gpios();

    queue_init(&mpq, 1, 10);

    multicore_launch_core1(core1_task);

    offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, LED_PIN, 800000, false);

    /* Init the LEDs to an initial state... */
    led_values[0] = LED_RGB(128, 0, 0);
    led_values[1] = LED_RGB(0, 128, 0);
    led_values[2] = LED_RGB(0, 0, 128);
    led_values[3] = LED_RGB(128, 0, 0);
    led_values[4] = LED_RGB(0, 128, 0);
    led_values[5] = LED_RGB(0, 0, 128);
    led_values[6] = LED_RGB(96, 0, 0);
    led_values[7] = LED_RGB(128, 128, 0);
    led_values[TOTAL_LEDS - 1] = LED_RGB(128, 128, 128);

    for(i = 0; i < NUM_WAD_LEDS; ++i) {
        led_values[8 + i] = LED_RGB(128, 0, 128);
        led_values[8 + i + NUM_WAD_LEDS] = LED_RGB(128, 0, 128);
    }

    data = TASK_LED_UPDATE;
    queue_add_blocking(&mpq, &data);

    data = TASK_NFC_INIT;
    queue_add_blocking(&mpq, &data);

    for(;;) {
        tud_task();
        hid_task();
    }

    return 0;
}

