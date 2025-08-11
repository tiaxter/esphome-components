#pragma once

#include "esphome/components/waveshare_epaper/waveshare_epaper.h"
#include "esphome/core/component.h"

namespace esphome {
namespace depg0266bn {
class DEPG0266BN : public esphome::waveshare_epaper::WaveshareEPaper {
  static constexpr uint16_t POWER_ON_TIME = 80;
  static constexpr uint16_t POWER_OFF_TIME = 80;
  static constexpr uint16_t FULL_REFRESH_TIME = 1200;
  static constexpr uint16_t PARTIAL_REFRESH_TIME = 300;

public:
  void initialize() override;
  void display() override;
  void dump_config() override;

  void deep_sleep() override {
    this->power_off_();
    if (this->reset_pin_ != nullptr) {
      this->command(0x10);
      this->data(0x01);
    }
  }

  void init_display_(uint8_t em);
  void init_full_(uint8_t em);
  void init_partial_(uint8_t em);
  void update_partial_();
  void update_full_();
  void write_lut_(const uint8_t *lut, uint8_t size);
  void _SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart,
                   uint8_t Ystart1, uint8_t Yend, uint8_t Yend1);
  void _SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1);
  void _setRamDataEntryMode(uint8_t em);
  void power_off_();
  void power_on_();
  void set_full_update_every(uint32_t full_update_every);

protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;
  // Reference https://github.com/ZinggJM/GxEPD/blob/a40fbc48a0f39de6c1a3d7365b9ad4d3d0365687/src/GxDEPG0266BN/GxDEPG0266BN.cpp#L420
  uint32_t idle_timeout_() override { return 10000000; };

private:
  uint32_t full_update_every_{30};
  uint32_t at_update_{0};
  bool deep_sleep_between_updates_{false};
  bool power_is_on_{false};
  bool is_deep_sleep_{false};
  uint8_t *old_buffer_{nullptr};
};
} // namespace depg0266bn
} // namespace esphome