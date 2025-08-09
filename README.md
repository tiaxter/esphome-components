# ESPHome Custom Components
This repository contains custom components for LilyGo T5 2.66" board. For more information, please refer to the [ESPHome display documentation](https://esphome.io/#display-components)

## Usage
Basic display definition example:
```yaml
external_components:
  - source: github://tiaxter/esphome-custom-components
    components: [ depg0266bn ]

display:
  - platform: depg0266bn
    cs_pin: 5
    dc_pin: 19
    busy_pin: 34
    reset_pin: 4
    reset_duration: 1200ms
    update_interval: never
    full_update_every: 5
```