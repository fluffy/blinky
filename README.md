# blinky

Blinking lights to tell time

# Build and flash

```
make 
st-flash --reset --format ihex write build/blinky.hex
```

# Features 

# Roadmap Features 

* Display 5ms ticks on 5x4 LED grid 

* Display tenths of seconds in BCD on 8 LEDs

* 10 Mhz sine wave output 1V p2p into 50 ohm, 3 channels 

* 3.3V PPS output 

* Jam sync to 3.3V input 

* Sync to GPS PPS 

* Show current PPS offset from GPS PPS

* Compare PPS accurace to GPS over 24 hours 

* Adjust local osc to GPS and store correction 

* Time drift of < 5 ms in 1 hour 

* Jitter of less than 1 ms 

# Wishlist 

* display 

* buttons for menu/select/up/down

* set input delay from length of GPS cable 

* nice box 

* battery with 2h life and USB charge 

* AC wall adapter power 

* sounds output that works over opus codec ( FSK ???, 2400 Hz ) 

* sounds input 

* RTC clock and battery 

* do SMPT LTC time codes on sync in / out 

