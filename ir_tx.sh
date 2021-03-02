#!/bin/bash
# Arguments:
#  ${1} - filename to transfer
#  ${2} optional argument - baudrate (9600 or 115200)
#  Baudrate will be configured as 9600 if not stated
# Note: 
#  xxd must be installed. To be able to config, script
#  should be run with sudo. GPIOs operations could only
#  be performed as sudo.
device=/dev/ttyAMA0
TMP_FILE_NAME=tmpfile_ir_tx.bin
# GPIOs to which TOIM4232 BD and RSP pins connected
GPIO_BD=23
GPIO_RST=24

# TOIM4232 configuration
if ! [[ -e /sys/class/gpio/gpio${GPIO_BD} ]] || ! [[ -e /sys/class/gpio/gpio${GPIO_BD} ]]; then
    echo "${GPIO_BD}" > /sys/class/gpio/export
    echo "${GPIO_RST}" > /sys/class/gpio/export
fi
echo "out" > /sys/class/gpio/gpio${GPIO_BD}/direction
echo "out" > /sys/class/gpio/gpio${GPIO_RST}/direction
# Reset TOIM4232 and go into program mode
echo "1" > /sys/class/gpio/gpio${GPIO_BD}/value
echo "1" > /sys/class/gpio/gpio${GPIO_RST}/value
sleep 1.0
# End reset condition and boot
echo "0" > /sys/class/gpio/gpio${GPIO_RST}/value
sleep 0.005
# Set serial tty with desired baudrate (9600 by default)
# and send program word to TOIM4232 
if [[ ${2} == 115200 ]]; then
    echo "Baudrate set as 115200"
    stty -F $device 9600 raw -echo -echoe -echok -crtscts -parenb cs8
    echo -en '\x10' > $device
    stty -F $device 115200 raw -echo -echoe -echok -crtscts -parenb cs8
else
    echo "Baudrate set as 9600"
    stty -F $device 9600 raw -echo -echoe -echok -crtscts -parenb cs8
    echo -en '\x16' > $device
fi
sleep 0.005
# Exit from programming mode
echo "0" > /sys/class/gpio/gpio${GPIO_BD}/value
sleep 0.005

PREAMBLE=DEADBEEF

FILESIZE=$(printf "%x\n" `stat -c "%s" ${1}`)
# Manually add zero HEX in begin to align by bytes
if [ `expr ${#FILESIZE} % 2` -ne 0 ]; then
    FILESIZE=0${FILESIZE}
fi
# Expand file size to 4 bytes (max size 4096 Mb)
if [ `expr ${#FILESIZE} % 2` -lt 8 ]; then
    while [ ${#FILESIZE} -lt 8 ]
    do
        FILESIZE=0${FILESIZE}
    done
else
    echo "File is too big to transfer"
    exit $?
fi

CRC=$(md5sum -b ${1})   # md5 + filename
CRC=${CRC::32}          # md5
echo $CRC

# Create temporary file with preamble, filesize, data and CRC,
# it will be transmitted
touch $TMP_FILE_NAME
echo -ne $PREAMBLE | xxd -r -p > $TMP_FILE_NAME
echo -ne $FILESIZE | xxd -r -p >> $TMP_FILE_NAME
cat ${1} >> $TMP_FILE_NAME
echo -ne $CRC | xxd -r -p >> $TMP_FILE_NAME

hexdump -v -e '/1 "%02X"' $TMP_FILE_NAME
echo
cat $TMP_FILE_NAME > $device
#rm $TMP_FILE_NAME
