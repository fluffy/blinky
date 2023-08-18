# Design Notes for  V4

Tim1 - 1 KHz tick, 1-second period 
* ch1: ext clk in onboard 1 connected to TXCO at 2.048 MHz 

Tim2 - 1 Mhz tick, 1 second loop, reset by Tim1 
* Ch1: sync in 
* Ch4: sync_mon in 

Tim3 - 10 KHz tick, 1 second loop , reset by Tim1 
* ch2: sync out 

No slave on tim7 so use Tim4 - drives the LEDs 
Tim4 - 24 KHz tick, 240 Hz loop , reset by Tim1 

Tim8 - Osc freq /250 tick, 1 second loop, reset by Tim1 
* ch1: GPS PPS 

Tim5 - not used - 32 bit 
* could ext clock perhaps from PA0, or PA1 (LEDM3 , LEDM1 ) 

MCO: conflict PA8 ( ex clk in), and PC9 (row1) 

# Design Notes for V5

Sync refers to Sync In

PPS refers to Sync Out

Mon refers to monitor of PPS

## Signals 
 
- ExtIn 10 MHz input 
- RefIn 10 Mhz OCXO input
- ExtOut 10 MHz output from MCO 
 
- SyncOut 1 pps output - inverts on buffer 
- SyncMon 1 pps in ( from SyncOut )  
- SyncIn 1 pps input - inverts on buffer 
- GpsIn 1 pps input 
 
 
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

### Main Timer:
* Use timer 2
* max 42 Mhz 
* 32 bits
* clocked with ETR from main TCXO
* use 20 Mhz 0.5 ppm JLCPCB C516500
* period of 1 second ( adjusted in software )
* adjustment store in EEPROM

* ETR: PA15 or  PA0, PA5, PA15
* Ch1: PA0 or PA0, PA5 - NA with ETR 
* Ch2: PA3 or PA1, PB3 - sync_in
* ch3: PB10 or PA2, PB10 - gps 
* ch4: PB11 or PA3,  PB11 - mon 

CH1 is disabled if ETR is in use 

* Capture Sync Mon
* Capture Sync In
* Capture GPS PPS
* Genrate Sync Out

### Sync Out Timer 
* Use timer 1
* max 84Mhz 
* reset on 1 second from main and tick rate 50KHz
* 16 bits 
* sync off of tim 2 or 5 
* CH on PA8 or  PA8,9,10,11

### Sync In Timer 
* User Tim8 
* max 84Mhz
* CH on PC6,7,8,9
* can sync off tim 2 or 5 

### Blink Timer
* trigger reset by main timer every second
* period about1 ms 
* interrupt ( less than 1KHz) to drive LEDs


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


# Interrupt Handling

On each Main Sync Mon
- save sync out local seconds and microseconds

On each Main GPS PPS
- increment gps seconds and save lastGpsTime in local seconds, microseconds

on each Main SyncIn
- save syncIn local seconds and microsecond

on each Main SyncOut
- if off, turn on, load syncOutOffset+100ms
- in on, turn off, load syncOutOffset

on each Aux SyncMon
* save aux 32+16 bit timer in AuxSyncMon auxSeconds, auxMicroseconds


On each Main Timer rollover
* reset ms time and offset by syncOutOffset
* increment local seconds 

On each Blink Timer
* increment msTime
* set all the LEDS based on msTime and local Sconds


Periodically on main thread
* compute gps offset microseconds (32 bit signed)
* compute syncIn offset microsoeconds (16 bit signed)
* compute syncMon offset microseconds (16 bit signed)
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

