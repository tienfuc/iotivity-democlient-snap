sudo stty -F /dev/ttyACM0 speed 1200 cs8 -cstopb -parenb
sudo /home/gerald/canonical/iot/arduino/arduino-1.5.7/hardware/tools/bossac -i -d --port=ttyACM0 -U false -e -w -v -b /home/gerald/canonical/iot/arduino/iotivity/out/arduino/arm/release/DemoServer.hex -R
