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
import usb_cdc
import struct
import traceback

class AimeIO:
    def __init__(self, leds, num_wad_leds, nfc):
        self._io = usb_cdc.data
        self._leds = leds
        self._wad_leds = num_wad_leds
        self._nfc = nfc

        if self._io is None:
            raise Exception("usb_cdc data is not enabled!")

        self._io.timeout = 0
        self._inbuf = bytearray(256)
        self._inbuf_pos = 0
        self._cmd_len = 0

        if self._nfc is not None:
            self._nfc.SAM_configuration()

    def _process_cmd(self, cmd, len):
        print("Got command: " + str(cmd) + " length: " + str(len))
        if cmd == 0:
            self._read_nfc()
        elif cmd == 1:
            self._set_nfc_led()
        elif cmd == 2:
            self._set_button_leds()

    def _clear_cmd(self, len):
        if self._inbuf_pos == len:
            self._inbuf_pos = 0
            return

        for i in range(self._inbuf_pos - len):
            self._inbuf[i] = self._inbuf[len + i]

        self._inbuf_pos = self._inbuf_pos - len

    def _read_nfc(self):
        # Can't get anything if we don't have an NFC reader...
        if self._nfc is None:
            print("No NFC reader attached!")
            self._io.write(b'\x00\x02')
            return

        try:
            tgt = self._nfc.felica_poll(timeout=0)
            if tgt is not None:
                print("Got FeliCa: " + str(tgt))
                self._io.write(b'\x00\x0A')
                self._io.write(str(tgt[0]))
                return
        except:
            traceback.print_exc()

        try:
            tgt = self._nfc.read_passive_target(timeout=0)
            if tgt is not None:
                print("Got non-FeliCa card: " + str(tgt))

                if len(tgt) < 8:
                    data = bytearray(b'\x1b\xad\xc0\xde\xca\xfe\xde\xad')
                    len = 0
                    for i in tgt:
                        data[len] = i
                    tgt = data
                elif len(tgt) > 8:
                    tgt = tgt[:8]

                print("Returning code: " + str(tgt))
                self._io.write(b'\x00\x0A')
                self._io.write(str(tgt))
                return
        except:
            traceback.print_exc()

        # Didn't get anything, return that fact.
        self._io.write(b'\x00\x02')

    def _set_nfc_led(self):
        col = struct.unpack('BBB', self._inbuf[2:5])
        self._leds[8 + 2 * self._wad_leds] = col
        self._leds.show()

    def _set_button_leds(self):
        for i in range(8):
            base = 2 + i * 3
            self._leds[i] = struct.unpack('BBB', self._inbuf[base:base + 3])

        left = struct.unpack('BBB', self._inbuf[26:29])
        right = struct.unpack('BBB', self._inbuf[29:32])
        for i in range(self._wad_leds):
            self._leds[8 + i] = left
            self._leds[8 + self._wad_leds + i] = right

        self._leds.show()

    def poll(self):
        if self._io.in_waiting != 0:
            data = self._io.read()
            cmdlen = None

            for b in data:
                self._inbuf[self._inbuf_pos] = b
                self._inbuf_pos += 1

            while self._inbuf_pos >= 2:
                if self._inbuf_pos >= 2:
                    (cmd, cmdlen) = struct.unpack('BB', self._inbuf[0:2])

                if self._inbuf_pos >= cmdlen:
                    self._process_cmd(cmd, cmdlen)
                    self._clear_cmd(cmdlen)
                else:
                    break
