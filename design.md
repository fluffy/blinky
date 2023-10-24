
# Design Notes for V6

Sync refers to Sync In

PPS refers to Sync Out

Mon refers to monitor of PPS

## Signals 
 
- ExtIn 10 MHz input 
- RefIn 10 Mhz OCXO input
- ExtOut 10 MHz output from MCO 
 
- SyncOut 1 pps output - inverts on on output buffer - PA8 on Timer #1 
- SyncMon 1 pps in ( from SyncOut ) - 
- SyncIn 1 pps input - inverts on buffer 
- GpsIn 1 pps input 
 
 On blink board 
 - AUX_CLK, AUX_GPS_PPS , GPS_RX, GPX_TX not used 
 
 - CLK on PA15 is 2.048 Mhz signal 
 
 ## Timers 
 

system clock or RefIn drives main 32 bit counter with overflow to 16
 bit 
 
ExtIn drives aux 32 bit counter with 16 bit overflow. 
 
Main and aux count at 10 Mhz and wrap ever second. 
 
Every timer can capture syncMon to provide sync of counters. Probably
don't need for the secondary counter for main and aux
 
Main and Aux can capture 
 SyncMon, SyncIn, GpsIn 
 
Main and Aux can generate SyncOut 

Display TImer drives LED and sync to main or aux rollover 
* does this need to capture syncMon ???

### PPS Out Timer #1  - hTimePps 

* Use timer 1
This time runnint at 168 Mhz input clock
* CH1 - PA8 - TimePps_CH_SYNC_OUT 
increments at 10KHz 
does 20 ms wide output pulse every 1 second 
Resets based on ITR1 from Tim2 Update Event (main) 
TODO * tick rate to 50KHz
* 16 bits 
* sync off of timer 2
* CH on PA8  - SYNC_OUT 


### Main Timer #2 - hTimeSync 

* Use timer 2
* max 42 Mhz 
* 32 bits
* clocked with ETR from main TCXO at 2.024 MHz 
* period of 1 second ( adjusted in software )
* adjustment store in EEPROM

* ETR: PA15 
* CH1: PA0 - NA with ETR 
* CH2: PA3  - sync_in   TimeSync_CH_SYNC_IN 
* CH3: PB10  - gps   TimeSync_CH_GPS_PPS  
* CH4: PB11  - sync_mon   TimeSync_CH_SYNC_MON 

CH1 is disabled if ETR is in use 

* Capture Sync Mon
* Capture PPS In
* Capture GPS PPS


### Blink Timer

* use timer 4 
* trigger reset by main timer every second
* Reset on ITR1 from Tim2 
* period 1 ms 
* interrupt ( 1 KHz) to drive LEDs


### Aux Timer

* Use 32 bit timer 5
* max 42 Mhz
* ETR:
* CH1: PA0 - AUX_CLK
* CH2:  PA1 - GPS 
* CH3: PA2 - UUX_MON
* Ch4: PA4  - NO 

* clocked with ETR from external 10 MHz
* captures something to link this to Main timer
* capture SyncMon
* capture GPS PPS
* capture SyncIn

### TImer 8 ???

1.5 sc , count up on main at 10 KHz


# Interrupt Handling

On each Main Sync Mon
- save sync out local seconds and microseconds

On each Main GPS PPS
- increment gps seconds and save lastGpsTime in local seconds, microseconds

on each Main SyncIn
- save syncIn local seconds and microsecond

on each Main SyncOut
- if off, turn on, load syncOutOffset+10ms * (seconds%10)
- in on, turn off, load syncOutOffset

on each Aux SyncMon
* save aux 32+16 bit timer in AuxSyncMon auxSeconds, auxMicroseconds

On each Main Timer rollover
* reset ms time and offset by syncOutOffset
* increment local seconds 

On each Blink Timer
* increment msTime
* set all the LEDS based on msTime and local Seconds


Periodically on main thread
* compute gps offset microseconds (32 bit signed)
* compute syncIn offset microsoeconds (32 bit signed)
* compute syncMon offset microseconds (32 bit signed)
* compute aux offset microseconds ( 32 bit ) from synMon times on main and aux
* if have recent syncIn, adjust syncOutOffset to match syncIn
* if have recent gpsPPS, adjust syncOutOffset to match gps pps

* save last 100 seconds of syncInOffset, gpsOffset, auxOffset, syncMon offset
* compute average offset over last 100s for GPS, Aux, SyncIn
* compute drift rate in Hz over last 100s for GPS, Aux, SyncIn
* compute stdDev off of drift linear for GPS, Aux, Sync 

* adjust period of main to match GPS, Aux, Sync in that priority order 

# TODO

* figure out how to base everything off Aux if that is there instead of Main
* figure out how to base everything off of GPS if that is there instead of main


### Adjusting
On Freq counter. INtrurment setup timeout off.
Noise Reject on, DC, 1M ohm, manual level 200 mV
History grom from 1 s - 20 ns to 1 s+20 ns, 40 bins

Seeing data fall from -6 to +12 ns.
Say 10 ppb ??? => 36 ms / hour ???