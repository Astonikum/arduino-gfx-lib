# MadArduinoGFX

MadDisplay framebuffer and differential presentation on top of **GFX Library for
Arduino 1.6.9**. Supports ST7735/ST7735S, ST7789, ILI9341, GC9A01, ST7796,
ILI9488 (SPI RGB666), SSD1331 and SSD1351. Rendering stays in MadDisplay; the
vendor library handles initialization, offsets, inversion and physical bus writes.

See `examples/color_ui/color_ui.ino` and [display catalog](https://github.com/Astonikum/display-studio/blob/main/DISPLAYS.md) for
all profile IDs. Build with `pio run` in this directory. Configure wiring in
`Arduino_HWSPI`, or supply another Arduino_GFX bus and driver instance.

```cpp
Arduino_HWSPI bus(2,5); // DC, CS; default board SPI pins
Arduino_GFX* driver = mad::createPanelDriver(&bus,"gc9a01-240x240",4);
mad::ArduinoGFX panel(*driver,"gc9a01-240x240");
mad::Display<mad::ArduinoGFX> display(panel);
// setup: display.begin(); register/open pages; loop: display.render(micros());
```

`begin(frequency=0,differential=true)` chooses a conservative bus clock, validates
driver dimensions and allocates an RGB565 frame. An optional history frame enables
row differential updates. If history allocation fails, full transfers remain usable;
if the current frame fails, begin returns false. ILI9488 uses Arduino_ILI9488_18bit:
two bytes per stored pixel, three bytes on SPI. Small AVR boards cannot hold large
color framebuffers.

`setFrameBuffers(frame,pixelCapacity,history,historyCapacity)` attaches external
storage before begin, including ESP32 PSRAM allocated with `heap_caps_malloc`.
Buffers must not overlap and must outlive the panel. `invalidate()` forces a full
transfer after an external reset. Changing driver rotation after begin is unsupported:
choose a matching profile/driver before begin (legacy landscape ILI9341 is included).

Factory offsets follow the upstream module examples. AliExpress module revisions
may need a different Arduino_ST7735/ST7789 constructor, inversion or RGB/BGR
setting. Supply that driver directly to the same adapter. Backlight GPIO/PWM and
touch-controller input belong to application wiring, not the framebuffer transport.

Upstream: https://github.com/moononournation/Arduino_GFX

## Installation

Repository: https://github.com/Astonikum/arduino-gfx-lib

PlatformIO (`platformio.ini`):

```ini
lib_deps =
  https://github.com/Astonikum/arduino-gfx-lib.git#v0.2.1
```

PlatformIO resolves the core, adapter support and vendor dependencies automatically.

Arduino IDE: download **Code → Download ZIP**, then use **Sketch → Include Library → Add .ZIP Library**.
Install these GitHub ZIP libraries first:

- [MadDisplay](https://github.com/Astonikum/display-lib)
- [MadDisplayAdapters](https://github.com/Astonikum/adapter-common-lib)

Install from Arduino Library Manager: GFX Library for Arduino.

These packages are distributed through GitHub; they are not yet indexed in the Arduino or PlatformIO registries. Arduino IDE does not install a library directly from a Git URL.

Requires C++14 on the host. Embedded examples select their board and its Arduino toolchain in `platformio.ini`. RAM requirements are documented above; a matching controller does not guarantee a board has enough memory.
