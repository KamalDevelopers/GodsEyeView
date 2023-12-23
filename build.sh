#!/bin/sh

if [ "$1" == "help" ] || [ "$1" == "-h" ] || [ "$1" == "--help" ]
then
    echo "build.sh [option]"
    echo "options:"
    echo "      default     - build and run"
    echo "      fullscreen  - enable 1920x1080 resolution"
    echo "      nousr       - don't compile userland"
    echo "      ports       - compile all ports"
    echo "      norun       - build but don't run"
    echo "      help        - display usage"
    exit
fi

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

if [ "$1" == "fullscreen" ]
then
    sed -i -e 's/.... ; config vesa width/1920 ; config vesa width/g' ./Bootloader/vesa.asm
    sed -i -e 's/.... ; config vesa height/1080 ; config vesa height/g' ./Bootloader/vesa.asm
fi

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
    cd ..
fi

if [ "$1" == "fullscreen" ]
then
    sed -i -e 's/.... ; config vesa width/1440 ; config vesa width/g' ./Bootloader/vesa.asm
    sed -i -e 's/.... ; config vesa height/900  ; config vesa height/g' ./Bootloader/vesa.asm
fi
