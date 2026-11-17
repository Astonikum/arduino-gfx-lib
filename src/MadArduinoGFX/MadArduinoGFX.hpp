#pragma once
#include <Arduino_GFX_Library.h>
#include <MadDisplay/panel_presentation.hpp>
#include <stdlib.h>

namespace mad {
// The vendor driver owns the bus and controller initialization; MadDisplay owns rendering.
class ArduinoGFX final : public RGB565Surface {
 public:
  ArduinoGFX(Arduino_GFX& driver,const char* profile) : driver_(driver), spec_(panelSpec(profile)), valid_(profile && strcmp(profile,spec_.id)==0 && spec_.kind==PanelKind::RGB) {}
  ~ArduinoGFX() override { if (ownFrame_) free(frame_); if (ownPrevious_) free(previous_); }
  ArduinoGFX(const ArduinoGFX&)=delete; ArduinoGFX& operator=(const ArduinoGFX&)=delete;
  bool setFrameBuffers(uint16_t* frame,size_t capacity,uint16_t* previous=nullptr,size_t previousCapacity=0) {
    const uint32_t pixels=uint32_t(spec_.width)*spec_.height;
    if (ready_ || frame_ || !frame || capacity<pixels || (previous && previousCapacity<pixels)) return false;
    const uintptr_t a=reinterpret_cast<uintptr_t>(frame), b=reinterpret_cast<uintptr_t>(previous);
    if (previous && (a>b ? a-b : b-a)<pixels*2) return false;
    frame_=frame; previous_=previous; external_=true; return true;
  }
  bool begin(uint32_t frequency=0,bool differential=true) {
    const uint32_t count=uint32_t(spec_.width)*spec_.height;
    if (!frequency) frequency=spec_.maxBusHz()<24000000UL ? spec_.maxBusHz() : 24000000UL;
    if (!valid_ || count>SIZE_MAX/2 || !frequency || frequency>spec_.maxBusHz()) return false;
    if (!frame_) { frame_=static_cast<uint16_t*>(malloc(count*2)); ownFrame_=frame_!=nullptr; }
    if (!frame_) return false;
    if (differential && !previous_ && !external_) { previous_=static_cast<uint16_t*>(malloc(count*2)); ownPrevious_=previous_!=nullptr; }
    if (!driver_.begin(frequency) || driver_.width()!=spec_.width || driver_.height()!=spec_.height) return false;
    differential_=differential && previous_; ready_=setStorage({spec_.width,spec_.height},frame_,count);
    clear(); presenter_.invalidate(); return ready_;
  }
  void present() override {
    if (ready_) presenter_.present(*this,differential_ ? previous_ : nullptr,*this,spec_.wireBits()/8,spec_.windowBytes());
  }
  void invalidate() { presenter_.invalidate(); }
  const TransferStats& transfer() const { return presenter_.stats(); }
  bool differentialUpdates() const { return differential_; }
  Arduino_GFX& driver() { return driver_; }
  void startWrite() {} void endWrite() {}
  void setAddrWindow(uint16_t x,uint16_t y,uint16_t w,uint16_t h) { x_=x; y_=y; w_=w; h_=h; }
  void writePixels(uint16_t* pixels,size_t,bool,bool) { driver_.draw16bitRGBBitmap(x_,y_,pixels,w_,h_); }
 private:
  Arduino_GFX& driver_; const PanelSpec& spec_;
  RGB565Presenter presenter_; uint16_t *frame_=nullptr,*previous_=nullptr;
  uint16_t x_=0,y_=0,w_=0,h_=0;
  bool valid_, ready_=false,ownFrame_=false,ownPrevious_=false,external_=false,differential_=false;
};

// Defaults follow Arduino_GFX's module examples. Supply your own vendor driver for other panel offsets/inversion.
inline Arduino_GFX* createPanelDriver(Arduino_DataBus* bus,const char* profile,int8_t reset=-1) {
  const auto& p=panelSpec(profile); if (!bus || !profile || strcmp(p.id,profile) || p.kind!=PanelKind::RGB) return nullptr;
  if (!strcmp(p.controller,"ST7735") || !strcmp(p.controller,"ST7735S"))
    return new Arduino_ST7735(bus,reset,0,p.width==80,p.width,p.height,p.width==80 ? 26 : 2,p.width==80 ? 1 : p.height==128 ? 3 : 1,p.width==80 ? 26 : 2,p.width==80 ? 1 : p.height==128 ? 3 : 1);
  if (!strcmp(p.controller,"ST7789")) {
    const uint8_t x=p.width==135 ? 52 : p.width==172 ? 34 : p.width==170 ? 35 : 0;
    const uint8_t y=p.width==135 ? 40 : p.height==280 ? 20 : 0;
    return new Arduino_ST7789(bus,reset,0,true,p.width,p.height,x,y,p.width==135 ? 53 : x,p.width==240 && p.height==240 ? 80 : y);
  }
  if (!strcmp(p.controller,"ILI9341")) return new Arduino_ILI9341(bus,reset,p.width==320 ? 1 : 0);
  if (!strcmp(p.controller,"GC9A01")) return new Arduino_GC9A01(bus,reset);
  if (!strcmp(p.controller,"ST7796S")) return new Arduino_ST7796(bus,reset);
  if (!strcmp(p.controller,"ILI9488")) return new Arduino_ILI9488_18bit(bus,reset);
  if (!strcmp(p.controller,"SSD1351")) return new Arduino_SSD1351(bus,reset);
  if (!strcmp(p.controller,"SSD1331")) return new Arduino_SSD1331(bus,reset);
  return nullptr;
}
} // namespace mad
