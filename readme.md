# squirrel
## What is squirrel? 
Box that detects and annoys squirrels (or any other object)

### Purpose
Protect peach trees

### Contributors

### Software Dependencies


### BOM
  - 1X: Adafruit Feather M0 Proto (part 2772)
  - 1X: Adafruit Feather MusicWing w/3A Amplifier (part 3436)
  - 1X: Adafruit VL53L0X ToF sensor (part 3317)

### Pinouts


### Information Sources
  - Adafruit Feather M0 Proto: https://learn.adafruit.com/adafruit-feather-m0-basic-proto
  - HC-SR04 (ultrasonic sensor): https://docs.google.com/document/d/1Y-yZnNhMYy7rwhAgyL_pfa39RsB-x2qR4vP8saG73rE/edit
  - Adafruit MusicWing: https://learn.adafruit.com/adafruit-music-maker-featherwing/
  - Adafruit VL53L0X: https://learn.adafruit.com/adafruit-vl53l0x-micro-lidar-distance-sensor-breakout

### Issues

### Questions

### Learnings
- 070720 - You want to validate the HC-SR04 input over multiple samples (depending on sampling rate) as I see multiple spurious signals at high sample rate
- 070720 - As usual, check voltage requirement between sensor and MCU. For 5v HC-SR04 and 3v MCU, 10K resistor inline to MCU pin and 10K resistor on same pin to GND
- 071120 - You can't read the battery level using A7 on Feather M0 with Music Maker due to pin 9 conflict, but you could make your own 100k-100k voltage divider between BAT and GND, and read that with a different analog pin.

### Feature Requests
  - 070720 - Add vibration code to further annoy squirrel
  - 070920 - Add RTC clock to solution
  - 070920 - open a second logfile and dump DEBUG to it (via new #define DEBUG_LOGFILE)?
 
### Revisions
- 070720
  - merged code from clock.ino and feather_player.ino Adafruit example
- 070820
  - added support for VL53L0X sensor
- 070920
  - added logging support to SD card already on MusicWing