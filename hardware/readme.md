# Notes on updating hardware

If the schematic or boards in changed such that the software can tell
the difference, the rev need to go up a letter. Other changes can get a
sub number on the rev letter.


## Schematics

Don't update sybols  / footprint from libraries. It over writes stuff.

When done, do a plot to generate schematic in production subdir.

## PCB

When done, do an export of step to generate a model in production
subdir.

## JLCPCB

Make sure to have them check part placement becuase it will be
wrong. Even what they have in the preivewi tool does not allways match
what they have in productions.
