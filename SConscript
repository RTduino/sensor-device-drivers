from building import *

src = []

if GetDepend('ARDUINO_ADAFRUIT_AHTX0_USING_SENSOR_DEVICE'):
    src += ['sensor_adafruit_ahtx0.cpp']

group = DefineGroup('RTduino', src, depend = ['PKG_USING_RTDUINO', 'PKG_USING_ARDUINO_SENSOR_DEVICE_DRIVERS'])

Return('group')
