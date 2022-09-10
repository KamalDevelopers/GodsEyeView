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

mkdir .out/iso
mkdir .out/iso/boot
mkdir .out/iso/boot/grub
cp .out/kernel.bin .out/iso/boot/kernel.bin
echo 'set timeout=0'                     >> .out/iso/boot/grub/grub.cfg
echo 'set default=0'                     >> .out/iso/boot/grub/grub.cfg
echo ''                                  >> .out/iso/boot/grub/grub.cfg
echo 'menuentry "gevos" {'               >> .out/iso/boot/grub/grub.cfg
echo '  multiboot /boot/kernel.bin'      >> .out/iso/boot/grub/grub.cfg
echo '  boot'                            >> .out/iso/boot/grub/grub.cfg
echo '}'                                 >> .out/iso/boot/grub/grub.cfg
cd ./.out

grub-mkrescue --output=kernel.iso iso
rm -rf iso

cd ../

if [ "$1" != "norun" ]
then
    ninja run
fi

if [ "$1" != "norun" ]
then
    ninja -t clean
    cd Libraries/
    ninja -t clean
fi
