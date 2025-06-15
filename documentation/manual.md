
# Manual for Blink Box

This is the manual for V1.0 software on Rev A hardware.


## Overview

The blink box has a grid of LEDs that blink at a fix rate and are
synchronized to other blink box. The camera of a video conferencing device
can be pointed at one box, the send that the video conferencing system to
a screen. At the screen end, a photo is take of the screen of another
synchronized blink box. By looking at the image of the blink box on
screen, and the time that represents, and comparing it to the time
displayed by the local box, the overall camera glass to screen glass
latency can be computed.

Once the devices are synchronized, they will stay synchronized for over
10 hours.

## Controls and Display

### Buttons

Mute Button ( black  ): The audio beep used in testing can be very
irritating. This button turn the generation of the audio beep on and off.

Blank Button ( white  ): This control what is displayed on the LED
grid and cycles across three modes. In the first mode nothing is
displayed, in the second the detected audio latency is displayed, and in
the third, the time is displayed for video testing.

Sync Button ( red ):


### Status LED:

Red: Error. Look at USB serial to get more information.

Purple: Bad power. The USB port can not deliver enough power to run the
oscillator.

Blue: Not Synchronized. The output time is not synchronized and no
synchronization input is detected.

Teal: Could Sync. The output is not synchronized but an input sync signal
is detected. Pressing the sync button should cause the device to become
synchronized.

Green: Synchronized. The output has been synchronized with the device
connected to the sync input.


### LED grid

The main green LEDs display one green LED at a time and advance to the
next LED at a rate of 240 Hz. They display 4 across then go to the next
row. This results in the rows advancing at 30 Hz and there are 10 rows.

The red LEDs advance at a rate of 3 Hz and only the first 3 are used.

To read the time in frames for a 30 fps system, the user would look at
the red LEDs and take if it was 1,2 or 3rd and multiple that by 10 then
add which would give the frame numbers in the second.

Yellow LEDs: The first LED indicates an error, the second if the device
is synchronized or not, and the third if there is an input synchronization
signal detected or not.


## Video Latency Measurement

Two blink boxes are used called primary and secondary. The secondary will
synchronize it's time to the primary. The sync output of the primary is
connected to sync input of the secondary. This should cause the status
LED of the secondary to changes from blue to teal. At this point the sync
button can be pressed and the secondary should go to green to indicate
it is syhncrozed. The primary will remain blue.

The blank button is pressed once or twice to get into the mode where
video is showing timing data on both boxes.

The video camera of the input is pointed at the primary and relays that
video to screen at the output end. The secondary is set up beside the
output screen and a photo is take that captures both the screen and
secondary at the same time.

By looking at the photo, it is possible to see how far behind in time the
image on the screen is from the current time n the secondary unit which
provided the one way glass to glass latency of the system.

## Loss of Synchronization

They synchronization remains valid for about an hour on uncalibrated
system and over 10 hours on a calibrated system.

## Audio Latency Measurement

An headset need to be plugged into the headset jack of the blink
box. Without this, the computer will not detect there is a headset. A
cable is from the computer audio jack computer jack of the blink
box. For mobile devices, use a lightning or USB to 1/8 inch TRRS
adapter. The headsets can be used to check the that audio is working
thought the video conferencing system.

On the primary device, press mute button to toggle on off audio beep and
make sure you can hear it on the headset of the secondary device. You
may need to play with volume levels.

There are three ways to look at the audio latency. The first is to put
the display into audio mode and it will be displayed on the LEDs. TODO
explain reading. The second is plug the USB port into a computer and use
it as a serial connection which will print the latency. That is explain
in the uSB second. The last is to use the Speaker / Mic output jacks
and feed the mic from the primary and speaker from the secondary into a
two channel oscilloscope to measure the latency.

## Inputs and Outputs

### USB

The USB port provides a serial interface at 115200 baud, 8 bit, no
parity, 1 stop bit.

```
pyserial-miniterm -e --parity N --rts 0 --dtr 0  115200
```

The RTS and DTR do need to be low for the serial interface to work and
not all terminal programs support this.

The device of /dev/cu.usbserial-31110 would change but the easiest way to
find it is look at what devices are there before turning the blink box
on then what device are there after it is turned on.


### Sync in / out

The Sync in and out use Linear Time Codes to send and receive time.


### Headset and Computer Audio

These allow the blink box to be in the middle of the audio connection
and listening to audio going to the headset and sound coming from the mic
as well as inject synthetic audio beeps.


### Speaker / Mic

This allows an tap of what is going in mic or out speaker at amplified
levels that are appropriate for recording or sending to an
oscilloscope. The PicoScope 2204A is very nice for this.


## Mounting

Some iPad mounts and tripods can be used to hold the blink box in a good location.
