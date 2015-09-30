sudo pkill minicom
scons ARDUINO_HOME=/home/gerald/canonical/iot/arduino/arduino-1.5.7 TARGET_OS=arduino TARGET_ARCH=arm SHIELD=ETH arduino_demo_server
../arduino-1.5.7/hardware/tools/gcc-arm-none-eabi-4.8.3-2014q1/bin/arm-none-eabi-objcopy -O binary out/arduino/arm/release/DemoServer out/arduino/arm/release/DemoServer.hex
