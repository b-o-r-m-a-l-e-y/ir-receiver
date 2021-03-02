#!/bin/bash
# Arguments:
#  ${1} - filename to transfer
#  ${2} optional - baudrate (9600 by default or 115200)
device=/dev/ttyAMA0
TMP_FILE_NAME=tmpfile_ir_tx.bin
GPIO_BD=23
GPIO_RST=24

# TOIM4232 configuration
if ! [[ -e /sys/class/gpio/gpio${GPIO_BD} ]] || ! [[ -e /sys/class/gpio/gpio${GPIO_BD} ]]; then
    echo "${GPIO_BD}" > /sys/class/gpio/export
    echo "${GPIO_RST}" > /sys/class/gpio/export
fi
echo "out" > /sys/class/gpio/gpio${GPIO_BD}/direction
echo "out" > /sys/class/gpio/gpio${GPIO_RST}/direction
echo "1" > /sys/class/gpio/gpio${GPIO_BD}/value
echo "1" > /sys/class/gpio/gpio${GPIO_RST}/value
sleep 1.0
echo "0" > /sys/class/gpio/gpio${GPIO_RST}/value
sleep 0.005
if [ ${2} -eq 115200 ]; then
    stty -F $device 115200 raw -echo -echoe -echok -crtscts -parenb cs8
    echo -en '\x10' > $device
else
    stty -F $device 9600 raw -echo -echoe -echok -crtscts -parenb cs8
    echo -en '\x16' > $device
fi
sleep 0.005
echo "0" > /sys/class/gpio/gpio${GPIO_BD}/value
stty -F $device 9600 raw -echo -echoe -echok -crtscts -parenb cs8
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


touch $TMP_FILE_NAME
echo -ne $PREAMBLE | xxd -r -p > $TMP_FILE_NAME
echo -ne $FILESIZE | xxd -r -p >> $TMP_FILE_NAME
cat ${1} >> $TMP_FILE_NAME
echo -ne $CRC | xxd -r -p >> $TMP_FILE_NAME

hexdump -v -e '/1 "%02X"' $TMP_FILE_NAME
echo
cat $TMP_FILE_NAME > $device
