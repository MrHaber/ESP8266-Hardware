# ESP8266-Hardware
Custom build esp8266 hardware. Using NodeMCU v3 with TFT Module 1.8 and RTC timer module ds3231

This fuction implements realtime display on TFT Module. Using NTPClient or ds3231 clock setting. This time does not changes from NTP time server. If you want you can add google services. 
Please use I2C checker for working with I2C bus. Please check your pinout before using this code.
### Connection: DS3231 to esp8266:
* VCC - 3.3V
* SDA - D6
* SCL - D3
* GND - GND
