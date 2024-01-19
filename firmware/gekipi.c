#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/stdlib.h"
#include "usb_descriptors.h"

int main(void) {
    stdio_init_all();
    board_init();
    tusb_init();
    
    for(;;) {
        tud_task();
    }

    return 0;
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

