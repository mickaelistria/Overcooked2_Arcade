# Overcooked! 2 Arcade Controller

Play Overcooked2! on arcade with real kitchen objects!

## Controller

### Hardware/Material

* Move: an arcade joystick with ball/handle color of player (1.Blue - 2.Red - 3.Green - 4.Yellow) 
* Take/Drop items: a big arcade push button, color of player
* Cut: a wooden board and an unsharpened knife (ideally with handle color of player)
* Wash: a plate (plastic? ideally color of player) and a dishwashing brush or a sponge
* Throw: ?
* Fire extinguisher: ?

### Microcontroller

An HID capable and Arduino compatible microcontroller (eg Raspberry Pico) is connected to all those items and report
action as gamepad events + specific keyboard event (cf `fourGamepads` folder).

Each microcontroller can emulate 2 gamepads/players. So for 4 playes, 2 microcontrollers should be enough.

## Software

To allow Overcooked!2 to be more arcade friendly, deploy the mods from `Mods` folder in it.
