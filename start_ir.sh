#!/bin/bash
BD=23
RST=24
echo $BD > /sys/class/gpio/export # 17 - bd
echo $RST > /sys/class/gpio/export # 18 - reset
echo "out" > /sys/class/gpio/gpio${BD}/direction
echo "out" > /sys/class/gpio/gpio${RST}/direction
echo "1" > /sys/class/gpio/gpio${RST}/value
echo "1" > /sys/class/gpio/gpio${BD}/value
sleep 1.0
echo "0" > /sys/class/gpio/gpio${RST}/value
sleep 0.005
stty -F /dev/ttyAMA0 9600 raw -echo -echoe -echok -crtscts -parenb cs8
echo -en '\x16' > /dev/ttyAMA0
sleep 0.005
echo "0" > /sys/class/gpio/gpio${BD}/value