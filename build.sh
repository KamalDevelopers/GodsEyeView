#!/bin/sh

ninja disk 
ninja format
cd Libraries 
ninja
cd ..
ninja

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

ninja bootloader
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
