# blinky

Blinking lights to tell time. And it beeps.

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

If this fails, try the following reset then try to flash again. This
will not work on a brand new uniitalized board, the board needs tobe
programmed forfirst time with st-flash.

Can reset the board with:
```
stty -f /dev/cu.usbserial-31110 hup
```

Monitor output connect with Serial program. Use 115200 baud with 8 bits,
no parity, 1 stop bit. Set the RTS and DTR both low. Screen fails to do
this. It sets RTS and DTR both high it will not work.

To use python miniterm (replace usbserial-31110 with number on your machine):
```
pyserial-miniterm -e --parity N --rts 0 --dtr 0 /dev/cu.usbserial-31110 115200
```
Can exit the miniterm with CTRL+]. This will cause board reset when it starts.

# Tools and Dependencies

Set up a mac to be able to build by installing:

* TODO
* pip3 install pyserial

# Features

## Blink Timing Board

On the Rev A hardware 1.0 software have:

* Timing board with TXCO, LTC Sync in, LTC Sync out, LED grid,
  battery power, USB debug, audio in/out

* Display 120 fps tick on 10x4 grid
* 3.3V LTC Sync Out
* PPS out is hold sync button while booting
* Jitter of less than 1 ms
* Jam sync to 3.3V Sync input
* Time drift of < 5 ms in 1 hour
* USB port for serial data
* read  calibration from EEPROM
* audio out of beep
* headset monitor of audio out
* usb upgrade of firmware
* Status LEDs: error(red), external synced (green),  not sync (
  blue ), have sync (teal)
* audio input to detect beep
* battery with 2h life and USB charger

## Clock and GPS Board

On the Rev A hardware 0.2 software have:

* GPS board with GPS, OCXO
* LTC sync out, LTC sync in
* Does not ahve audio or led display grid
* 10 Mhz sine wave output 1V p2p into 50 ohm, 3 channels
* Sync to GPS PPS
* Show current PPS offset from GPS PPS
* Compare PPS accuracy to GPS over 24 hours
* read  calibration from EEPROM
* USB serial display of: Time, GPS Error, SyncIn Error
* Aux input of 10 MHz signal to compare
* PPS jitter of about +/- 20 ns and error less than 10 ns
* USB upgrade of firmware
* USB 3A power for gps board
* USB output serial data for computer time sync

## Light GPS Board

On the Rev A hardware 0.2 software have:

* GPS board with GPS
* LTC sync out
* Does not ahve audio or led display grid
* Sync to GPS PPS
* USB upgrade of firmware
* USB power
* USB output serial data for computer time sync


## Wishlist

* time into browser measure computer time offset
* software upgrade from browser
