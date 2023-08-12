# Design Notes for  V4

Tim1 - 1 KHz tick, 1 second period 
* ch1: ext clk in on board 1 connected to TXCO at 2.048 MHz 

Tim2 - 1 Mhz tick, 1 second loop
* Ch1: sync in 
* Ch4: sync_mon in 

Tim3 - 10 KHz tick, 1 second loop 
* ch2: sync out 

Tim7 - 1Mhz tick, 1 ms loop 

Tim8 - 
* ch1: GPS PPS 

Tim5 - not used - 32 bit 
* could ext clock perhaps from PA0, or PA1 (LEDM3 , LEDM1 ) 

MCO: conflict PA8 ( ex clk in), and PC9 (row1) 

# Design Notes for V5

Sync refers to Sync In
Out refers to Sync Out

## Signalls 
 
 ExtIn 10 MHz input 
 RefIn 10 Mhz OCXO input
 ExtOut 10 MHz output from MCO 
 
 SyncOut 1 pps output - inverts on buffer 
 SyncMon 1 pps in ( from SyncOut )  
 SyncIn 1 pps input - inverts on buffer 
 GpsIn 1 pps input 
 
 
 ## Timers 
 
system clock or RefIn drives main 32 bit coutner with overlofow to 16
 bit 
 
ExtIn drives aux 32 bit coutner with 16 bit overflow. 
 
Main and aux count at 10 Mhz and wrap ever second. 
 
Every timer can capture syncMon to provide sync of counters. Probaly
don't need for the secondary counter for main and aux
 
Main and Aux can capture 
 SyncMon, SyncIn, GpsIn 
 
Main and Aux can genrate SyncOut 

Display TImer drives LED and sync to main or aux rollover 
* does this need to capture syncMon ???

### Main Timer:
* Use timer X
* 32 bits
* clocked with ETR from main TCXO
* use 20 Mhz 0.5 ppm JLCPCB C516500
* period of 1 second ( adjusted in software )
* adjustment store in EEPROM

* Capture Sync Mon
* Capture Sync In
* Capture GPS PPS
* Genrate Sync Out

### MainSeconds Time
* User Timer X, 16 bits
* clocked from Main Timer
* counts seconds, rollover 18 hours 
* Capture Sync Mon

### Blink Timer
* triggerreset by main timer every second
* period abotu 1 ms 
* interupt ( less than 1KHz) to drive LEDs


### Aux Timer
* Use 32 bit timer X
* clocked with ETR from extermal 10 MHz
* captures something to link this to Main timer
* capture SyncMon
* capture GPS PPS
* capture SyncIn

### AuxSeconds  Time 2
* clock from Aux timer
* 16 bit
* gives totall counter of 48 bits for external 10Mhz
* rollover around 10 months 
* Capture Sync Mon

# Interupt Handling

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

* save last 100 seconds of syncInOffset, gpsOffset, auxOffset, syncMon offse
* compute average offset over last 100s for GPS, Aux, SyncIn
* compute drift rate in Hz over last 100s for GPS, Aux, SyncIn
* compute stdDev off of drift linear for GPS, Aux, Sync 

* adjust period of main to match GPS, Aux, Sync in that priority order 

# TODO
* figure out how to base everything off Aux if that is there instead of Main
* figure out how to base everything off of GPS if that is there instead of main

