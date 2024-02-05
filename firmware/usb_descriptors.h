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

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#define GAMEPAD_INSTANCE            0
#define NFC_INSTANCE                1

typedef struct TU_ATTR_PACKED {
    uint16_t x;
    uint16_t y;
    uint16_t buttons;
} gekipi_gamepad_report_t;

typedef enum {
    GEKIPI_BUTTON_LR        = TU_BIT(0),
    GEKIPI_BUTTON_LG        = TU_BIT(1),
    GEKIPI_BUTTON_LB        = TU_BIT(2),
    GEKIPI_BUTTON_RR        = TU_BIT(3),
    GEKIPI_BUTTON_RG        = TU_BIT(4),
    GEKIPI_BUTTON_RB        = TU_BIT(5),
    GEKIPI_BUTTON_LMENU     = TU_BIT(6),
    GEKIPI_BUTTON_RMENU     = TU_BIT(7),
    GEKIPI_BUTTON_LWAD      = TU_BIT(8),
    GEKIPI_BUTTON_RWAD      = TU_BIT(9),
    GEKIPI_BUTTON_SERVICE   = TU_BIT(10),
    GEKIPI_BUTTON_TEST      = TU_BIT(11),
} gekipi_gamepad_button_bm_t;


#define GEKIPI_HID_REPORT_DESC_GAMEPAD(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                 ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  )                 ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                 ,\
    /* Report ID if any */ \
    __VA_ARGS__ \
    /* 16 bit X, Y (min -32767, max 32767 ) */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP                  ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_X                     ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_Y                     ) ,\
    HID_LOGICAL_MIN_N  ( 0x0000, 2                               ) ,\
    HID_LOGICAL_MAX_N  ( 0xffff, 3                               ) ,\
    HID_REPORT_COUNT   ( 2                                       ) ,\
    HID_REPORT_SIZE    ( 16                                      ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ) ,\
    /* 12 bit Button Map */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_BUTTON                   ) ,\
    HID_USAGE_MIN      ( 1                                       ) ,\
    HID_USAGE_MAX      ( 12                                      ) ,\
    HID_LOGICAL_MIN    ( 0                                       ) ,\
    HID_LOGICAL_MAX    ( 1                                       ) ,\
    HID_REPORT_COUNT   ( 12                                      ) ,\
    HID_REPORT_SIZE    ( 1                                       ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ) ,\
    /* 4 bits padding */ \
    HID_REPORT_SIZE    ( 4                                       ) ,\
    HID_REPORT_COUNT   ( 1                                       ) ,\
    HID_INPUT          ( HID_CONSTANT | HID_ARRAY | HID_ABSOLUTE ) ,\
  HID_COLLECTION_END \

#endif /* !USB_DESCRIPTORS_H */
