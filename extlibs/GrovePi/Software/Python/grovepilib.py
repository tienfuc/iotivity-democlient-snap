#!/usr/bin/env python

import math
import decimal
import time
import grovepi
import grove_rgb_lcd

# LED configs
led_red = 4
led_green = 5
led_blue = 6

# DHT11 sensor
dht11_sensor_port = 7     # Connect the DHt sensor to port 7

# Connect the Grove Light Sensor to analog port A0
light_sensor = 0

# Connect the Grove Sound Sensor to analog port A1
# SIG,NC,VCC,GND
sound_sensor = 1

# Connect the Grove Buzzer to digital port D8
# SIG,NC,VCC,GND
buzzer = 8

# Connect the Grove Button to digital port D3
# SIG,NC,VCC,GND
button = 3





def CtoF( tempc ):
        "This converts celcius to fahrenheit"
        tempf = round((tempc * 1.8) + 32, 2);
        return tempf;

def FtoC( tempf ):
        "This converts fahrenheit to celcius"
        tempc = round((tempf - 32) / 1.8, 2)
        return tempc;

def sensor_read_temp():
        temp = 0.01
        hum = 0.01
        [ temp,hum ] = grovepi.dht(dht11_sensor_port,0)
        #print temp
        return temp
                
def sensor_read_humidity():
        temp = 0.01
        hum = 0.01
        [ temp,hum ] = grovepi.dht(dht11_sensor_port,0)
        #print hum
        return hum

def sensor_read_light():
        grovepi.pinMode(light_sensor,"INPUT")
        sensor_value = grovepi.analogRead(light_sensor)
        return sensor_value

def sensor_read_sound():
	grovepi.pinMode(sound_sensor,"INPUT")
        sensor_value = grovepi.analogRead(sound_sensor)
        return sensor_value

def led_read_red():
        grovepi.pinMode(led_red,"INPUT")
        #time.sleep(1)
        status = grovepi.digitalRead(led_red)
        return status

def led_read_green():
        grovepi.pinMode(led_green,"INPUT")
        #time.sleep(1)
        status = grovepi.digitalRead(led_green)
        return status

def led_read_blue():
        grovepi.pinMode(led_blue,"INPUT")
        #time.sleep(1)
        status = grovepi.digitalRead(led_blue)
        return status

def led_write_red(value):
        grovepi.pinMode(led_red,"OUTPUT")
        #time.sleep(1)
        grovepi.digitalWrite(led_red, value)
	return 0

def led_write_green(value):
        grovepi.pinMode(led_green,"OUTPUT")
        #time.sleep(1)
        grovepi.digitalWrite(led_green, value)
	return 0

def led_write_blue(value):
        grovepi.pinMode(led_blue,"OUTPUT")
        #time.sleep(1)
        grovepi.digitalWrite(led_blue, value)
	return 0

def lcd_write_str(value):
	#print "Write string: "
	#print value
	grove_rgb_lcd.setRGB(0, 255, 0)
	grove_rgb_lcd.setText(value)
	return 0

def buzzer_write(value):
	grovepi.pinMode(buzzer,"OUTPUT")

        grovepi.digitalWrite(buzzer,1)
        time.sleep(value)
        grovepi.digitalWrite(buzzer,0)

def button_read():
        grovepi.pinMode(button,"INPUT")
        status = grovepi.digitalRead(button)
        return status
