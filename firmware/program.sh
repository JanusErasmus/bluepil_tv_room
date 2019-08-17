#!/usr/bin/bash
echo 'Building'
make

retVal=$?
if [ $retVal -ne 0 ]; then
	echo -e '\x1b[31mBuild failed\x1b[0m'
	exit $retVal
else
	echo -e '\x1b[32mBuild OK \x1b[0m'
fi

echo 'Programming'

stm32flash -w $1 -b 115200 -g 0x8000000  /dev/ttyUSB0

retVal=$?
if [ $retVal -ne 0 ]; then
	echo -e '\x1b[31mProgramming failed\x1b[0m'
	exit $retVal
else
	echo -e '\x1b[32mProgram OK\x1b[0m'
fi

minicom -D /dev/ttyUSB0 -c on
