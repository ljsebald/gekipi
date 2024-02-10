# GekiPi Firmware

A controller for games such as O.N.G.E.K.I. based around the Raspberry Pi Pico.

## USB Devices

This firmware acts as three USB devices when plugged into a PC:

1. A USB CDC device - This is used for debugging purposes and will probably be
   removed at some point.
2. A USB HID Gamepad - All controls are routed through this device, and it
   should appear as any other gamepad to your PC. The gamepad device has two
   axes (only the X axis is used for the stick) and 12 buttons.
3. A USB HID Input/Output device - This is used for controling the LEDs and
   for interacting with the PN532 to read NFC cards. See below for how to
   interact with this device.

## Interaction with HID I/O Device

The HID I/O device for interacting with the LEDs and NFC works on a very simple
command/response format. Commands are a single byte, followed by any data that
the command needs. The following commands are currently supported:

1. LED Update (command = 0x01)
   Command will be immediately followed by the full set of LED colors in RGB
   order. Ordering follows the LED wiring of the board. 1 byte is used per
   color component, so there should be 33 bytes of data total after the command.
2. Begin NFC Polling (command = 0x02, no data)
3. Request NFC Data (command = 0x03, no data)
4. Request control data (command = 0x04, no data)

Each command will respond with a response in the following format:

* 1 byte: command code (this will match the command sent)
* 1 byte: return code (>= 0 for success, < 0 for failure)
* n bytes: any data

Commands 0x01 and 0x02 should always return with a return code of 0 with no
accompanying data unless the command sent is malformed. Command 0x03 will
respond with slightly different data depending on what type of NFC card was
detected. If a MIFARE card was detected (return code 1), the data returned will
be the UID of the card (which may be 4, 7, or 10 bytes long). If a FeliCa card
is detected, the data returned will be the IDm of the card (8 bytes), followed
by the PMm of the card (8 bytes), and the system code of the card (2 bytes). If
polling is still in progress, the return code will be 0. If no card was detected
after the polling period completed, the return code will be 0xfe.
