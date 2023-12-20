#!/bin/sh

ninja disk 
ninja format
cd Libraries 
echo -e "\033[32;1;4mCompiling libraries\033[0m"
ninja
echo -e "\033[0mDone.\n"
cd ..
echo -e "\033[32;1;4mCompiling kernel\033[0m"
ninja
echo -e "\033[0mDone.\n"


if [ "$1" == "ports" ]
then
    cd Userland/Ports
    ./install.sh
    cd ../../
    ninja disk
fi

if [ "$1" != "nousr" ]
then
    cd Userland/Apps
    ninja -t clean
    ninja
    cd ../..
    cd Userland/Servers
    ninja -t clean
    ninja
    cd ../..
    ninja disk
fi

echo -e "\033[32;1;4mCompiling bootloader\033[0m"
ninja bootloader
echo -e "\033[0mDone.\n"
ninja chain

if [ "$1" != "norun" ]
then
    ninja run
fi

if [ "$1" != "norun" ]
then
    rm bootloader.bin
    rm boot.bin
    rm kernel.bin
    rm drive
    ninja -t clean
    cd Libraries/
    ninja -t clean
fi
