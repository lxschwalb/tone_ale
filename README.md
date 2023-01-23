# Tone Ale
rp2040 based guitar pedal

## Upload new effect
You don't need to be able to develop or install special software to upload new effects. All you need is a usb port, cable, and something conductive (a piece of tin foil will to).

To upload new effects, connect the pedal to a computer with a usb cable, touch the back of the pcb with a piece of tinfoil and drag it to the side as shown below. Now the pedal shows up as a mass storage device. Just drag and drop the uf2 file of the corresponding effect and you're all set to go.
TODO add picture showing how to drag the tin foil

## Compile effects
Perhaps you want to alter some effects before uploading it onto the pedal. For example you might want to increase the gain of the fuzz. This would require recompiling the effect after the changes to the code have been made. Follow the steps below to recompile:

First set up the Pi Pico SDK according to [this guide](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html#sdk-setup)

Open terminal in the main project folder

Go to the build directory with `cd /build`

Run `cmake ..`

Run `make`

This will compile each effect and place it in "/build/<effect_name>/<effect_name>.uf2". This uf2 file can then directly be uploaded onto the pedal

## Develop new effects
TODO explain this

## Hardware setup
TODO explain what to solder where for people building their own pedal
