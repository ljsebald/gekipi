# GekiPi

A controller for games such as O.N.G.E.K.I. based around the Raspberry Pi Pico.

Please note that this is still a Work-In-Progress and isn't really meant for
anyone to actually try to use just yet.

## License

The code in the firmware directory is covered by the MIT License. See
`COPYING.MIT` for more information.

The hardware design in the board directory is covered by the CERN Open Hardware
License Version 2 - Strongly Reciprocal (CERN-OHL-S). See `cern_ohl_s_v2.txt`
for more information.

## Dependencies

The firmware for GekiPi currently only depends on the base Raspberry Pi Pico
SDK and its TinyUSB library.

## Pinouts and GPIO assignments:

Buttons (other than the WAD and Test/Service button) have the following pinout:
1. +5v
2. LED In
3. Signal (active low)
4. LED Out
5. GND

Test/Service have the following pinout:
1. Signal (active low)
2. GND

The WADs have the following pinout:
1. +5v
2. +3.3v
3. LED In
4. Signal (active high, +3.3v)
5. LED Out
6. GND

The NFC connector has the following pinout:
1. +5v
2. +3.3v
3. LED In
4. SDA
5. SCL
6. GND

The Stick connector has the following pinout:
1. +3.3v
2. Analog Signal
3. Analog GND

The LEDs are wired in the following order (all are single LEDs except the WADs):
1. Left Red
2. Left Green
3. Left Blue
4. Right Red
5. Right Green
6. Right Blue
7. Left Menu
8. Right Menu
9. Left WAD
10. Right WAD
11. NFC

The number of LEDs per WAD can be set in the code and defaults to 6.

GPIO pins are assigned as follows on the Pico:
- GPIO0: Left Red
- GPIO1: Left Green
- GPIO2: Left Blue
- GPIO3: Right Red
- GPIO4: Right Green
- GPIO5: Right Blue
- GPIO6: Left Menu
- GPIO7: Right Menu
- GPIO8: Left WAD
- GPIO9: Right WAD
- GPIO16: NFC SDA
- GPIO17: NFC SCL
- GPIO20: LED Output (to 74AHCT125, then Left Red)
- GPIO21: Service
- GPIO22: Test
- GPIO28/ADC2: Stick

All other ADC channels available on a standard Pi Pico board are wired to the
Analog connector (along with +3.3v and Analog GND).

All other GPIO pins (other than those on the Analog connector) are available on
the GPIO connector (along with +3.3v, +5v, and GND)
