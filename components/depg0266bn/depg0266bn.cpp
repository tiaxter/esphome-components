#include "depg0266bn.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include <bitset>
#include <cinttypes>

namespace esphome {
namespace depg0266bn {
static const char *const TAG = "DEPG0266BN";

static const uint8_t lut_partial[] = {
  0x0, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x80, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x40, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0A, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2,
  0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0, 0x0, 0x0,
};

#ifdef USE_ESP32
RTC_DATA_ATTR uint8_t rtc_display_updates = 0;
RTC_DATA_ATTR bool rtc_initialized = false;
RTC_DATA_ATTR uint8_t rtc_old_buffer[152 * 296 / 8u];
#endif

// Code reference:
// - https://github.com/ZinggJM/GxEPD/tree/master/src/GxDEPG0266BN
// - https://github.com/ZinggJM/GxEPD2/blob/master/src/epd/GxEPD2_266_BN.h
// - https://github.com/ZinggJM/GxEPD2/blob/master/src/epd/GxEPD2_266_BN.cpp

void DEPG0266BN::initialize() {
#ifdef USE_ESP32
  if (!rtc_initialized) {
    memset(rtc_old_buffer, 0xFF, this->get_buffer_length_());
    rtc_initialized = true;
  }
#else
  // Old buffer for partial update
  RAMAllocator<uint8_t> allocator;
  this->old_buffer_ = allocator.allocate(this->get_buffer_length_());

  if (this->old_buffer_ == nullptr) {
    ESP_LOGE(TAG, "Could not allocate old buffer for display!");
    return;
  }
#endif
}

void DEPG0266BN::_setRamDataEntryMode(uint8_t em) {
  const uint16_t xPixelsPar = this->get_width_internal() - 1;
  const uint16_t yPixelsPar = this->get_height_internal() - 1;
  this->command(0x11);
  this->data(em);
  this->_SetRamArea(0x00, xPixelsPar / 8, 0x00, 0x00, yPixelsPar % 256,
                    yPixelsPar / 256);    // X-source area,Y-gate area
  this->_SetRamPointer(0x00, 0x00, 0x00); // set ram
}

void DEPG0266BN::_SetRamArea(uint8_t Xstart, uint8_t Xend, uint8_t Ystart,
                             uint8_t Ystart1, uint8_t Yend, uint8_t Yend1) {
  this->command(0x44);
  this->data(Xstart);
  this->data(Xend);
  this->command(0x45);
  this->data(Ystart);
  this->data(Ystart1);
  this->data(Yend);
  this->data(Yend1);
}

void DEPG0266BN::_SetRamPointer(uint8_t addrX, uint8_t addrY, uint8_t addrY1) {
  this->command(0x4e);
  this->data(addrX);
  this->command(0x4f);
  this->data(a8ddrY);
  this->data(addrY1);
}

void DEPG0266BN::init_display_(uint8_t em) {
  // Hardware reset
  this->reset_pin_->digital_write(false);
  delay(10);
  this->reset_pin_->digital_write(true);
  delay(10);

  // Software reset
  this->command(0x12);
  this->wait_until_idle_();

  // Driver output control
  this->command(0x01);
  this->data(0x27);
  this->data(0x01);
  this->data(0x00);

  // BorderWavefrom
  this->command(0x3C);
  this->data(0x05);

  // Display update control
  this->command(0x21);
  this->data(0x00);
  this->data(0x80);

  // Read the built-in temperature sensor
  this->command(0x18);
  this->data(0x80);
  _setRamDataEntryMode(em);
}

void DEPG0266BN::init_full_(uint8_t em) {
  this->init_display_(em);
  this->power_on_();
}

void DEPG0266BN::init_partial_(uint8_t em) {
  this->init_display_(em);

  this->power_on_();
  this->wait_until_idle_();

  this->command(0x32);

  this->start_data_();
  for (uint16_t i = 0; i < sizeof(lut_partial); i++) {
    this->write_byte(lut_partial[i]);
  }
  this->end_data_();
}

void DEPG0266BN::update_partial_() {
  this->command(0x22);
  this->data(0xCC);
  this->command(0x20);
}

void DEPG0266BN::update_full_() {
  this->command(0x22);
  this->data(0xf4); // disable analog (powerOff() here)
  this->command(0x20);
}

void DEPG0266BN::power_off_() {
  if (this->power_is_on_) {
    this->command(0x22);
    this->data(0x83);
    this->command(0x20);
    delay(POWER_OFF_TIME);
    this->power_is_on_ = false;
  }
}

void DEPG0266BN::power_on_() {
  if (!this->power_is_on_) {
    this->command(0x22);
    this->data(0xc0);
    this->command(0x20);
    delay(POWER_ON_TIME);
    this->power_is_on_ = true;
  }
}

void HOT DEPG0266BN::display() {
#ifdef USE_ESP32
  this->at_update_ = rtc_display_updates;
#endif
  uint8_t ram_entry_mode = 0x03;
  bool full_update = this->at_update_ == 0;

  if (full_update) {
    DEPG0266BN::init_full_(ram_entry_mode);
  } else {
    DEPG0266BN::init_partial_(ram_entry_mode);
  }

  // Update previous buffer
  this->command(0x26);
  delay(2);
  this->start_data_();
  for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
#ifdef USE_ESP32
    this->write_byte(rtc_old_buffer[i]);
#else
    this->write_byte(this->old_buffer_[i]);
#endif
  }
  this->end_data_();
  delay(2);

  // Update the current buffer
  this->command(0x24);
  delay(2);
  this->start_data_();
  for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
    this->write_byte(this->buffer_[i]);
#ifdef USE_ESP32
    // Store the current buffer in RTC memory
    rtc_old_buffer[i] = this->buffer_[i];
#else
    // Store the current buffer in the old buffer
    this->old_buffer_[i] = this->buffer_[i];
#endif
  }
  this->end_data_();
  delay(2);

  if (full_update) {
    DEPG0266BN::update_full_();
  } else {
    DEPG0266BN::update_partial_();
  }

  this->wait_until_idle_();

  this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;
#ifdef USE_ESP32
  rtc_display_updates = this->at_update_;
#endif
}

void HOT DEPG0266BN::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || y >= this->get_height_internal() ||
      x < 0 || y < 0)
    return;

  const uint32_t pos = (x + y * this->get_width_controller()) / 8u;
  const uint8_t subpos = x & 0x07;

  if (!color.is_on()) {
    this->buffer_[pos] |= 0x80 >> subpos;
  } else {
    this->buffer_[pos] &= ~(0x80 >> subpos);
  }
}

int DEPG0266BN::get_width_internal() { return 152; }

int DEPG0266BN::get_height_internal() { return 296; }

void DEPG0266BN::set_full_update_every(uint32_t full_update_every) {
  this->full_update_every_ = full_update_every;
}

void DEPG0266BN::dump_config() {
  LOG_DISPLAY("", "DEPG0266BN", this);
  ESP_LOGCONFIG(TAG, "  Model: 2.66in B");
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Busy Pin: ", this->busy_pin_);
  LOG_UPDATE_INTERVAL(this);
}

} // namespace depg0266bn
} // namespace esphome