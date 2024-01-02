# blinky

Blinking lights to tell time

# Build and flash

```
make 

st-flash --reset --format ihex write build/blinky.hex
or
stm32flash -w build/blinky.bin -v -b 115200 -i "-rts,-dtr,dtr:rts,-dtr,dtr"  /dev/cu.usbserial-31120

screen /dev/cu.usbserial-21110 115200
exit screen with ^A^\
```


# Features 

On the V7 hardware 0.3 software have:

* Display 240 Hz ticks on 8x5 LED grid 
* Display 30 fps frame count in binary on 8 LEDs
* 3.3V PPS Sync Out 
* Jitter of less than 1 ms
* Jam sync to 3.3V Sync input
* press button to sync all 
* Time drift of < 5 ms in 1 hour 
* usb port for serial data 
* save time error correction, version, serial in EEPROM
* audio out of beep 
* headset monitor of audio out 

# Soon 

* Status LEDs: error(red), external synced ( green ), external not sync (
  yellow ) but PPS 
* sounds output that works over opus codec
* audio in to detect beep 

# Roadmap Features 

* 10 Mhz sine wave output 1V p2p into 50 ohm, 3 channels 
* Sync to GPS PPS 
* Show current PPS offset from GPS PPS
* Compare PPS accuracy to GPS over 24 hours 
* Adjust local oscilator to GPS and store correction 
* serial display of: Time, GPS Error, SyncIn Error



# Wishlist

* Aux Count Error

* time into browser measure computer time offset 

* Heavy board with GPS, OCXO, wall power, 3 x 10Mhz out, LTC sync out, LTC
  sync in, serial debug out, Ext 10 Mhz in, 10 Mhz out,
* Light board with TXCO, LTC Sync in, LTC Sync out, LED grid, 
  battery power, USB charge, audio in/out
* display
* set input time delay from length of GPS cable
* nice box
* battery with 2h life and USB charger
* AC wall adapter power
* do SMPT LTC time codes on sync in / out 

* could we add flash video and flash detector for auto detect video
  latency
  

