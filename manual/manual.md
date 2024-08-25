
# Manual for Blink Box

This is the manual for V1.0 software on Rev A hardware.


## Overview

The blink box has a grid of LEDs that blink at a fix rate and are
syncronized to other blink box. The camera of a video conferncing device
can be pointed at one box, thn send that the video conferncing system to
a screen. At the screen end, a photo is take of the screen of anothe
sycnronzed blink box. By looking at the image of the blink box on
screen, and the time that represets, and comparing it to the time
dispalyed by the local box, the overal camera glass to screen glass
latency can be computed.

Once the devices are syncronized, they will stay syncronized for over
10 hours.

## Controls and Display

### Buttons

Mute Button ( black TODO ): The audio beep used in testing can be verry
irrating. This button turn the generation of the audio beep on and of.

Black Button ( white TODO ): This controll what is displed on the LED
grid and cycles acorss three modes. In the first mode nothing is
displayed, in the second the detected audio latency is displayed, and in
the third, the time is displed for video testing.

Sync Button ( red ):


### Status LED:

Red: Error. Look at USB serial to get more information.

Purple: Bad power. The USB port can not deliver enough power to run the
oscillator.

Blue: Not Sycronized. The output time is not syncronized and no
syncronization input is detected.

Teal: Could Sync. The output is not syncronized but an input sync signal
is detced. Pressing the sync button should cause the device to become
suyncronized.

Green: Syncronized. The output has been scyncronzed with the device
connected to the sycn input.


### LED grid

The main green LEDs display one green LED at a time and advace to the
next LED at a rate of 240 Hz. They display 4 across then go to the next
row. This results in the rows advancing at 30 Hz and there are 10 rows.

The red LEDs advance at a rate of 3 Hz and only the frist 3 are used.

To read the time in frames for a 30 fps system, the user would look at
the red LEDs and take if it was 1,2 or 3rd and multipley that by 10 then
add which would give the frame numbers in the second.

Yellow LEDs: The frist LED indicates an error, the second if the device
is syncronized or not, and the thrid if there is an input syncronization
signal detected or not.


## Video Latency Measurement

Two blink boxes are used called primary and secondary. The seconday will
syncronize it's time to the primary. The sync output of the primamry is
connected to sync input of the secondary. This should cause the status
LED of the seconday to changes from blue to teal. At this point the sync
button can be pressed and the secondayr should go to green to indicate
it is syncrozed. The primary will remain blue.

The blank button is pressed once or twice to get into the mode where
video is showing timing data on both boxes.

The video camer of the input is pointed at the primary and relays that
video to screen at the outptu end. The secondayr is set up beside the
output screen and a photo is take that captures both the screen and
secondayr at the same time.

By looking at the photo, it is possible to see how far behnd in time the
image on the screen is from the current time n the secondary unit which
provied the one way glass to glass latency of the system.

## Loss of Syncronization

They syncronization remains valid for about an hour on uncalbrated
system and over 10 hours on a calibrated system.

## Audio Latency Measurement

An headset need to be plugged into the headset jack of the blink
box. Without this, the computer will not detect there is a headset. A
cable is from the computer audio jack compuert jack of the blink
box. For mobile devices, use a ligtenging or USB to 1/8 inch TRRS
adapter. The headsets can be used to check the that audio is working
throught the video conferncing system.

On the primary device, press mute button to toggle on off audio beep and
make sure you can hear it on the headset of the secondary device. You
may need to play with volume levels.

There are three ways to look at the audio latncy. The first is to put
the displayu into audio mode and it will be displayed on the LEDs. TODO
explain reading. The second is plug the USB port into a comptuer and use
it as a serial connection which will print the latency. That is explain
in the uSB seciond. The last is to use the Speaker / Mic output jacks
and feed the mic from the primary and speaker from the secondary ninto a
two channel osciliscope to measure the latency.

## Inputs and Outputs

### USB

The USB port provifdes a serial interface at 115200 baud, 8 bit, no parity, 1 stop bit.

```
pyserial-miniterm -e --parity N --rts 0 --dtr 0  115200
```

The RTS and DTR do need to be low for the serial interface to work and
not all terminal programs supprot this.

The device of /dev/cu.usbserial-31110 would change but the eaest way to
find it is look at what devices are therye before turing the blink box
on then what device are there after it is turned on.



### Sync in / out

The Sync in and out use Linear Time Codes to send and receive time.



### Headset and Computer Audio

### Speaker / Mic



## Mounting

Some ipad mounts and tripods can be used to hold the blink box in a good location.
