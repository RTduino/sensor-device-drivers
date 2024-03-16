from building import *
import os

src = []

if GetDepend('ARDUINO_ADAFRUIT_AHTX0_USING_SENSOR_DEVICE'):
    src += [os.path.join('Adafruit-AHTx0', 'sensor_adafruit_ahtx0.cpp')]

if GetDepend('PKG_USING_ARDUINO_ADAFRUIT_SHT31'):
    src += [os.path.join('Adafruit-SHT31', 'sensor_adafruit_sht31.cpp')]

group = DefineGroup('RTduino-libraries', src, depend = ['PKG_USING_RTDUINO', 'PKG_USING_ARDUINO_SENSOR_DEVICE_DRIVERS'])

Return('group')
