# RetroZordPaddleSpinner
DIY Paddle and Spinner controller using arduino and rotary encoder.

This is a fork from [A2600 Paddles/Spinners USB Adapter](https://github.com/MiSTer-devel/Retro-Controllers-USB-MiSTer/blob/master/PaddleTwoControllersUSB/) by Alexey Melnikov (sorgelig).

### Original features

- Two devices using a single arduino
- Outputs in a way that MiSTer's sees it as a spinner and paddle combo

### Added features

- Added a second button
- Added [mode] and [config] buttons
- Added button debounce
- Added RetroZord's negCon mode (RZordPsWheel)
- Added mouse mode
- Added oled display
- Added mouse/spinner speed multiplier (1x, 4x, 7x, 10x)
- Added mouse axis inversion

### Modes

| Mode      | Serial ID         | Buttons |  MiSTer notes      |
|-----------|-------------------|---------|--------------------|
| MiSTer-S1 | MiSTer-S1 Spinner | 1       | Spinner and Paddle |
| NeGcon    | RZordPsWheel      | 2       | Wheel and Paddle   |
| Mouse     | RZordSpinnerMouse | 2       |                    |


Mode selection during boot by holding buttons:
- [Mode] high and [Config] high: MiSTer-S1 (2x) (falls to `DEVICEMODE_DEFAULT`)
- [Mode] high and [Config] low: NeGcon (2x)
- [Mode] low and [Config] high: Mouse (2x) as X
- [Mode] low and [Config] low: Mouse (1x), paddle 1 as X, paddle 2 as Y

Changing [Mode] button state during runtime will trigger a hardware reset (after 1 second).<br/>
Can be used to change the output mode without requiring to disconnect/connect the usb cable.

Pressing [Config] mode during runtime enters config mode (shows `Config` on the oled display).<br/>
While in this mode, main button press on each pad will change mouse/spinner multiplier value.<br/>
Secondary button will toggle mouse value inversion.<br/>
Press [Config] again to exit config mode.

Inverted mouse was added to play Tempest on MiSTer's psx core.<br/>
I recommend to set multiplier at 10 for this game.


### Credits

[Original code](https://github.com/MiSTer-devel/Retro-Controllers-USB-MiSTer/blob/master/PaddleTwoControllersUSB/) from sorgelig.

[DigitalIO](https://github.com/greiman/DigitalIO) and [SSD1306Ascii](https://github.com/greiman/SSD1306Ascii) from greiman.<br/>
[SoftWire](https://github.com/felias-fogg/SoftI2CMaster) from felias-fogg.<br/>
[Bugtton](https://github.com/sakabug/Bugtton) from sakabug<br/>

Special thanks to my father helping with the hardware.
