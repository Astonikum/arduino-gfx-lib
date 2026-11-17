#include <MadDisplay.h>
#include <MadArduinoGFX.h>
// ESP32 default hardware SPI: SCK 18, MOSI 23; change control pins for your wiring.
constexpr const char* profile="st7789-240x240";
Arduino_HWSPI bus(2,5);
Arduino_GFX* driver=mad::createPanelDriver(&bus,profile,4);
mad::ArduinoGFX adapter(*driver,profile);
mad::Display<mad::ArduinoGFX> display(adapter);
mad::VerticalLayout root;
mad::Page page(root,mad::Color::Black);
mad::Label title("MAD DISPLAY",mad::LabelStyle(mad::Color::White,1));
void setup() {
  if (!display.begin()) while (true) { delay(100); }
  root.add(title); display.setFps(20);
  display.openPage(display.registerPage(page));
}
void loop() { display.render(micros()); }
