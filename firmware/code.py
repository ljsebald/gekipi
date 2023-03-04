# -*- coding: utf-8 -*-
import digitalio
import board
import usb_hid
from analogio import AnalogIn
from adafruit_hid.mouse import Mouse
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS
from adafruit_hid.keycode import Keycode

# Switch over the power mode to clean up ADC noise.
power_mode = digitalio.DigitalInOut(board.GP23)
power_mode.switch_to_output(True)

# Adjust the GPIO pins listed here to your setup
#encoder_A = board.GP10
#encoder_B = board.GP11
potentiometer = board.GP26

#          Left R     Left G     Left B     Left Menu  Right R    Right G    Right B    Right Menu Left P     Right P    Test        Service
keys =   [ board.GP5, board.GP6, board.GP7, board.GP9, board.GP0, board.GP1, board.GP2, board.GP4, board.GP8, board.GP3, board.GP14, board.GP15 ]
keymap = [ Keycode.A, Keycode.S, Keycode.D, Keycode.F, Keycode.J, Keycode.K, Keycode.L, Keycode.O, Keycode.U, Keycode.E, Keycode.F6, Keycode.F7 ]

###############################################################################
#                Nothing should have to be changed below here.                #
#    With one minor exception if your potentiometer is setup differently.     #
###############################################################################
pot = AnalogIn(potentiometer)
mouse = Mouse(usb_hid.devices)
kbd = Keyboard(usb_hid.devices)
keypin = []
lastpressed = []
last_pos = None

val = (pot.value >> 6)
analogwindow = [val] * 32
analogsum = val * 32
analogidx = 0

# Init the GPIO used for the keys
for k in keys:
    pin = digitalio.DigitalInOut(k)
    pin.direction = digitalio.Direction.INPUT
    pin.pull = digitalio.Pull.UP
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
            # Pick the line that makes the mouse move the right way.
            mouse.move(diff)
            #mouse.move(-diff)
            last_pos = pos
    else:
        last_pos = pos

    # Handle keyboard emulation from the various switches.
    keyspressed = []
    for i in range(len(keypin)):
        # Is the key pressed? If so, it's pulled to ground
        if i != 8 and i != 9 and not keypin[i].value:
            keyspressed.append(keymap[i])
        elif (i == 8 or i == 9) and keypin[i].value:
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