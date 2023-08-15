# blinky

Blinking lights to tell time

# Build and flash

```
make 
st-flash --reset --format ihex write build/blinky.hex
screen /dev/cu.usbserial-21110 115200
exit screen with ^A^\
```


# Features 

On the V4 hardware 0.2 software have:

* Display 240 Hz ticks on 8x5 LED grid 
* Display 30 fps frame count in binary on 8 LEDs
* 3.3V PPS Sync Out 
* Jitter of less than 1 ms
* Jam sync to 3.3V Sync input
* press button to sync all 


# Roadmap Features 

* Time drift of < 5 ms in 1 hour 
* 10 Mhz sine wave output 1V p2p into 50 ohm, 3 channels 
* Sync to GPS PPS 
* Show current PPS offset from GPS PPS
* Compare PPS accuracy to GPS over 24 hours 
* Adjust local osc to GPS and store correction 
* Status LEDs: error(red), external synced ( green ), exernal not sync (
  yellow ) but PPS 
* serial display of: Time, GPS Error, SyncIn Error, Aux Count Error


# Wishlist

* Heavy board with GPS, OCXO, wall power, 3 x 10Mhz out, LTC sync out, LTC
  sync in, serial debug out, Ext 10 Mhz in, 10 Mhz out,
* Light board with TXCO, LTC Sync in, LTC Sync out, LED grid, 
  battery power, USB charge, audio in/out
* save time error correction, version, serial in EEPROM
* display
* buttons for menu/select/up/down
* set input time delay from length of GPS cable
* nice box
* battery with 2h life and USB charger
* AC wall adapter power
* sounds output that works over opus codec ( FSK ???, 2400 Hz )
* sounds input
* RTC clock and battery 
* do SMPT LTC time codes on sync in / out 
* audio out of beep
* audio in to detect beep
* headset monitor of audio out
