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

* Display 5ms ticks on 8x5 LED grid 

* Display tenths of seconds in binary on 8 LEDs

* 3.3V PPS output 



# Roadmap Features 

* Time drift of < 5 ms in 1 hour 

* Jitter of less than 1 ms 

* 10 Mhz sine wave output 1V p2p into 50 ohm, 3 channels 

* Jam sync to 3.3V input 

* Sync to GPS PPS 

* Show current PPS offset from GPS PPS

* Compare PPS accuracy to GPS over 24 hours 

* Adjust local osc to GPS and store correction 



# Wishlist

* 1 board with GPS, OCXO, wall power, 3 x 10Mhz out, LTC sync out, LTC sync in, serial debug out

* 1 board with 10 Mhz in, 10 Mhz out, TXCO, LTC Sync in, LTC Sync out, LED grid, battery power, USB charge, audio in/out


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

* headset moniotr of audio out

