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

        if(!tud_hid_ready())
            return;

        report.x = read_stick();
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
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

#define TASK_LED_UPDATE     1
#define TASK_NFC_INIT       2
#define TASK_NFC_POLL       3

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
                    if(nfc_initted) {
                    }

                    break;
            }
        }

        if(gpio_get(NFC_IRQ) == false) {
            printf("IRQ on core 1!\n");
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

