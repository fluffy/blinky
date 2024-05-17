
# Design Notes for V7

Sync refers to Sync In

PPS refers to Sync Out

Mon refers to monitor of PPS

## Signals

- ExtIn 10 MHz input on PA0
- RefIn 10 Mhz OCXO input on PA15

- PPS 1 pps output - inverts on on output buffer - PA8 on Timer #1
- Mon 1 pps in ( from PPS ) - PB11 & AUX PA2
- SyncIn 1 pps input - inverts on buffer  - PB3 and aux PC6
- GpsIn 1 pps input - PB10 and aux PA1

 On blink board:
 - AUX\_CLK, AUX\_GPS_PPS , GPS\_RX, GPX\_TX not used
 - CLK on PA15 is 2.048 Mhz signal

 ## Timers


system clock or RefIn drives main 32 bit counter

ExtIn drives aux 32 bit counter

Main and aux count at 10 Mhz and wrap ever second.

Every timer can capture syncMon to provide sync of counters

Main and Aux can capture
 SyncMon, SyncIn, GpsIn

Timer 1 can generate SyncOut and can sync to Main or Aux timer.

Display TImer drives LED and sync to main or aux rollover
* does this need to capture syncMon ???

### PPS Out Timer #1  - hTimePps (aka Sync Out)

* Use timer 1
This timer running at 168 Mhz input clock
* CH1 - PA8 - TimePps\_CH\_SYNC\_OUT
increments at 50 KHz
does 20 ms wide output pulse every 1 second
Resets based on ITR1 from Tim2 Update Event (main)
* 16 bits
* sync off of timer 2
loop ever 1.1 seconds
Input APB2 @ 168 MHz


### Main Timer #2 - hTimeSync (aka SyncIn/main)

* Use timer 2
* max 42 Mhz
* 32 bits
* clocked with ETR from main TCXO at 2.024 MHz or 10 MHz
* period of 1 second ( adjusted in software )
* adjustment store in EEPROM

* ETR: PA15
* CH1: PA0 - NA with ETR
* CH2: PA3  - sync\_in   TimeSync\_CH\_SYNC\_IN
* CH3: PB10  - gps   TimeSync\_CH\_GPS\_PPS
* CH4: PB11  - sync\_mon   TimeSync\_CH\_SYNC\_MON

CH1 is disabled if ETR is in use

* Capture Sync Mon
* Capture PPS In
* Capture GPS PPS

Input APB1 at 84 MHz


### ADC Timer - Timer 3

* count at 1MHz
* 114 count - about 440*20 Hz
* not sync, trigger out to ADC1
Input APB1 at 84 MHz

### Blink Timer - TImer 4

* use timer 4
* trigger reset by main timer every second
* Reset on ITR1 from Tim2
* period 1 ms
* interrupt ( 1 KHz) to drive LEDs
Input APB1 at 84 MHz


### Aux Timer - Timer 5

Assumes that that Aux signal is getting a 10Mhz signal.

* Use 32 bit timer 5
* max 42 Mhz
* ETR:
* CH1: PA0 - AUX\_CLK
* CH2:  PA1 - GPS
* CH3: PA2 - AUX_MON
* Ch4: PA4  - no

CH1 ends up feeding clock.  Capture the Captures both edges of external
so scale by 2 to  couting at 10 MHz. The SyncMon capture time allows this to
be coorelated with the other main clock.

* capture SyncMon
* capture GPS PPS
* capture SyncIn
Input APB1 at 84 MHz

### DAC Timer - Timer 6

* note this does not reset on main
* 1 Mhz count rate
* 8 KHz period ( 1000 count )
* trigger out to DAC2
Input APB1 at 84 MHz

### TImer 8 - used to time LTC

* has SYCN_IN2 on CH1 on PC6
* reset on main
* 50 KHz rate

1.1 seconds , count up on main at 50 KHz
Input APB2 @ 168 MHz


## Serial

USB\_RX/TX on PB6/PB7 on USART1
GPS\_RX1/TX1 on PC10/PC11

GPS is NMEA-0183 set at 4800 baud 8N1
GPS is Garmin "GPS 16x LVS"

## DAC

CH1 is voltage toVCO - PA4
CH2 is audio out - PA5

* DAC2 is DMA stream 6 memory to peripheral on DMA controller #1
* Normal not circular

## ADC

* uses TIM 3

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
* save aux 32 bit timer in AuxSyncMon auxSeconds

On each Main Timer rollover
* reset ms time and offset by syncOutOffset
* increment local seconds

On each Blink Timer
* increment msTime
* set all the LEDS based on msTime and local Seconds


Periodically on main thread
* compute gps offset microseconds (32 bit signed)
* compute syncIn offset microseconds (32 bit signed)
* compute syncMon offset microseconds (32 bit signed)
* compute aux offset microseconds ( 32 bit ) from synMon times on main and aux
* if have recent syncIn, adjust syncOutOffset to match syncIn
* if have recent gpsPPS, adjust syncOutOffset to match gps pps

* save last 100 seconds of syncInOffset, gpsOffset, auxOffset, syncMon offset
* compute average offset over last 100s for GPS, Aux, SyncIn
* compute drift rate in Hz over last 100s for GPS, Aux, SyncIn
* compute stdDev off of drift linear for GPS, Aux, Sync

* adjust period of main to match GPS, Aux, Sync in that priority order

# Charging & Power

Max charges rate is 500 mA up to 4.1 V then drops off with m charges up
to 4.2 V. This charge rate selected to not go over USB limit.  Using 820
mAH battery. Current consumption of about 170 mA when *not* on battery.

The blink board uses about 150 mA when running ( bouncing around 145 to
165 based on how many leds ).

On the ev9 board blink board, the 3.3v regulator, in 25C room, runs
about 40 C at 5V input and 50C at 7V input. Indicating about 33 C/watt
thermal resitance which pretty much matched specs for AMS117.


# Calibrating and Adjusting

On frequency counter. On "internal setup" set timeout off.
Noise Reject on, DC, 1M ohm, manual level 200 mV
History grom from 1 s - 20 ns to 1 s+20 ns, 40 bins

On the 2.048 Mhz VCO, will need to adjust pot to get in range. 30 degree
turn made huge difference.

# ESD

The SN74LVC1G14 provide ESD protection. The USB has pretection with
the TODO chip. The USB power CC1/2 power monitor circuits have no
protection. The AD8397 for audio ciricuits on blink board have no
protection on current design. The ADA4807 on clock board have no
protection. The power input on clock board has no protection.

# Linear Time Codes

The spec for them is at
https://www.itu.int/dms_pubrec/itu-r/rec/br/R-REC-BR.780-2-200504-W!!PDF-E.pdf
and more info at https://en.wikipedia.org/wiki/Linear_timecode


# UI and Modes

Buttons controll the varios modes.

Sync button causes it to sync to an input. If it has GPS then use
that, otherwise if have sync in, use that.

Display button moves between: blink,  audio latency, and off

Mute button moves between: rx only, rx & tx

Status LED is:
RED: Error
Purple: Bad power
Blue: Running but not synced
Green: Synced and running

Will auto sync on first sync if sync buton has never been hit

## Configureation

EEProm stores, serial number and device type, calibration info, config
if LTC or PPS output

## Serial Ouput

Sends @ mark for timing host software

Sends current time HH::MM:SS

All metrics reset to zero on first mon pulse after press of sync button

On clock board, mark pulse is GPS, on blink it is Sync

Measurements:

Current SyncIn Offset in uS
Current GPS Offset in uS
Current Mon Offset in uS
Tick time of last mon pulse
Tick time of last sync pulse
Tick time of last gps pulse

Current Mon ExtOffset in uS
Current GPS ExtOffset in uS
Tick time of last ext mon pulse
Tick time of last ext gps pulse

GPS seconds of last gps pulse
LTC seconds of last sync pulse

Local counter rollover count  ( seconds )
Ext counter rollover count ( seconds )

The current values are copied to prev once a seconds in the mon pulse
interupt then used to compute the rest of the metrics


Metrics:

Current time from varios soruces.

Time erorrs between sources

Rate of change between sources over 1,10,100 seconds



# TODO
