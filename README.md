# blinky

Blinking lights to tell time

# Build and flash

Build with:
```
mkdir build ; cd build ; cmake .. ; make
```

Program flash using stlink with:
```
st-flash --reset --format ihex write build/blinky.hex
```

Can program using Stm32CubeProgrammer with RTS=1, DTR=0, baud at 115200,
Parity Even, data bits 8, stop bits 1, flow control off.

Program flash using USB serial with:
```
stm32flash -w build/blinky.hex -v -b 115200 -m 8e1 -i "-dtr," /dev/cu.usbserial-31110
```

Can reset the board with:
```
stty -f /dev/cu.usbserial-31110 hup
```

Monitor output connect with Serial program like "SerialTools" on
OSX. Use 115200 baud with bits, parity, stop bit set to 8n1. Set the RTS and DTR both low. Screen
fails to do this. It sets RTS and DTR both high so will not work.

To use python miniterm:
```
pyserial-miniterm -e --parity N --rts 0 --dtr 0 /dev/tty.usbserial-31110 115200
```
Can exit the miniterm with CTRL+]. This will cause board reset when it starts.

# Tools and Dependencies

Set up a mac to be able to build by installing:

* TODO
*  pip3 install pyserial

# Features

## Timing Board

On the V9 hardware 0.09 software have:

* Timing board with TXCO, LTC Sync in, LTC Sync out, LED grid,
  battery power, USB debug, audio in/out

* Display 2ms ticks on 10x5 LED grid
* TODO - update Display 30 fps frame count in binary on 8 LEDs
* 3.3V LTC Sync Out
* PPS out is hold sync button while booting
* Jitter of less than 1 ms
* Jam sync to 3.3V Sync input
* press button to sync all
* Time drift of < 5 ms in 1 hour
* usb port for serial data
* read  calibration from EEPROM
* audio out of beep
* headset monitor of audio out

* Status LEDs: error(red), external synced (green),  not sync (
  blue ), have sync (teal)
* sounds output that works over opus codec
* audio in to detect beep

## GPS Board

On the V9 hardware 0.09 software have:

* GPS board with GPS, OCXO, wall power, 3 x 10Mhz out, LTC sync out, LTC
  sync in, USB serial debug out, Ext 10 Mhz in

* no audio or led display grid
* 10 Mhz sine wave output 1V p2p into 50 ohm, 3 channels
* Sync to GPS PPS
* Show current PPS offset from GPS PPS
* Compare PPS accuracy to GPS over 24 hours
* USB serial display of: Time, GPS Error, SyncIn Error
* Aux input of 10 MHz signal to compare
* PPS jitter of about +/- 20 ns and error less than 10 ns

## Wishlist

* time into browser measure computer time offset
* nice box
* battery with 2h life and USB charger
* AC wall adapter power
* usb upgrade of firmware
* usb power for gps board

* could we add flash video and flash detector for auto detect video
  latency
