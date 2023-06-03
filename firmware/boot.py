# -*- coding: utf-8 -*-
import usb_hid
import bitmap_keyboard

usb_hid.enable((
    bitmap_keyboard.BITMAP_KEYBOARD,
    usb_hid.Device.MOUSE
))
