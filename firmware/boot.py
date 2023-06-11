# -*- coding: utf-8 -*-
#
# Copyright (C) 2023 Lawrence Sebald
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
import usb_hid, usb_midi, usb_cdc
import bitmap_keyboard

usb_midi.disable()

usb_hid.enable((
    bitmap_keyboard.BITMAP_KEYBOARD,
    usb_hid.Device.MOUSE
))

usb_cdc.enable(console=True, data=True)
