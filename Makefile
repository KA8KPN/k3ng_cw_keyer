BOARD_TAG    = uno
ARDUINO_LIBS = LiquidCrystal_I2C EEPROM Wire K3NG_PS2Keyboard
USER_LIB_PATH = ${PWD}/libraries
MONITOR_PORT = /dev/ttyACM0

include /usr/share/arduino/Arduino.mk

