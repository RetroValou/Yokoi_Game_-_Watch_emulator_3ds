# Virtual Input

Contains the input configuration for Game & Watch systems and other compatible electronic games. Designed to be global and reusable outside of the 3DS.


## Multiplexing of input

Game & Watch systems have a very limited number of input lines (most of the time 4 standard inputs + 2 special inputs). However, the actual number of physical buttons on some Game & Watch models largely exceeds those 6 inputs.
This is made possible thanks to multiplexing.

Buttons are wired in a grid between two types of pins: an output pin and an input pin.
The output pins (usually 4 of them) can be activated by the CPU. Depending on which output pin is currently active, only a specific subset of buttons can work, since they are powered by that output line.

This mechanism makes it possible to determine which button is pressed using two values:

the output pin that is currently activated

the input pin that is being read

In theory, this allows up to 4 × 4 = 16 buttons using only 8 pins.

The input configuration is defined as follows:
cpu->input_set(output_pin, input_pin, state)

However, some Game & Watch systems do not require multiplexing and have their buttons directly wired to input pins (the button is always powered). This mainly applies to the earliest Game & Watch models with only two action buttons (such as Ball, Vermin, and Fire).

Later Game & Watch models always use multiplexing — probably because Nintendo found it simpler to generalize the approach, even for systems that technically didn’t require it. For example, Game & Watch Fire (Wide Screen) uses multiplexing, unlike the original Game & Watch Fire.