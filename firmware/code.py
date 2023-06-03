# -*- coding: utf-8 -*-
import digitalio
import board
import usb_hid
from analogio import AnalogIn
from adafruit_hid.mouse import Mouse
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS
from adafruit_hid.keycode import Keycode
from bitmap_keyboard import BitmapKeyboard

# Adjust the GPIO pins listed here to your setup
potentiometer = board.GP28
pot_positive  = True       # Set to False if your potentiometer value lowers as the stick goes right

#          Left R     Left G     Left B     Left Menu  Right R    Right G    Right B    Right Menu Left WAD   Right WAD  Test        Service
keys =   [ board.GP0, board.GP1, board.GP2, board.GP6, board.GP3, board.GP4, board.GP5, board.GP7, board.GP8, board.GP9, board.GP22, board.GP21 ]
keymap = [ Keycode.A, Keycode.S, Keycode.D, Keycode.F, Keycode.J, Keycode.K, Keycode.L, Keycode.O, Keycode.U, Keycode.E, Keycode.F6, Keycode.F7 ]
press =  [ False,     False,     False,     False,     False,     False,     False,     False,     True,      True,      False,      False      ]

###############################################################################
#                Nothing should have to be changed below here.                #
###############################################################################
# Switch over the power mode to clean up ADC noise.
power_mode = digitalio.DigitalInOut(board.SMPS_MODE)
power_mode.switch_to_output(True)

# Init the HID stuff and set up the potentiometer.
pot = AnalogIn(potentiometer)
mouse = Mouse(usb_hid.devices)
kbd = BitmapKeyboard(usb_hid.devices)
keypin = []
lastpressed = []
last_pos = None

val = (pot.value >> 6)
analogwindow = [val] * 32
analogsum = val * 32
analogidx = 0

# Init the GPIO used for the buttons
for i in range(len(keys)):
    k = keys[i]
    pin = digitalio.DigitalInOut(k)
    pin.direction = digitalio.Direction.INPUT

    if press[i] == False:
        pin.pull = digitalio.Pull.UP
    else:
        pin.pull = digitalio.Pull.DOWN

    keypin.append(pin)

# Main loop
while True:
    # Handle mouse emulation based on a potentiometer...
    # XXXX: I don't really like this 32 element moving average...
    #       but at least it irons out a lot of the noise, and it
    #       is extremely simple.
    val = (pot.value >> 6)
    analogsum -= analogwindow[analogidx]
    analogwindow[analogidx] = val
    analogsum += val
    analogidx = (analogidx + 1) & 0x1f

    pos = analogsum >> 5
    if last_pos is not None:
        diff = last_pos - pos
        if diff > 4 or diff < -4:
            if pot_positive:
                mouse.move(diff)
            else:
                mouse.move(-diff)
            last_pos = pos
    else:
        last_pos = pos

    # Handle keyboard emulation from the various switches.
    keyspressed = []
    for i in range(len(keypin)):
        # Is the key pressed?
        if keypin[i].value == press[i]:
            keyspressed.append(keymap[i])

    sl = set(lastpressed)
    sk = set(keyspressed)
    rel = list(sl - sk)
    pre = list(sk - sl)

    if len(rel) != 0:
        kbd.release(*rel)
    if len(pre) != 0:
        kbd.press(*pre)

    lastpressed = keyspressed
