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
    ninja
    cd ../..
    ninja disk
fi

if [ "$#" -eq  "0" ]
then
    ninja run
else
    if [ "$1" = "iso" ]
    then
        mkdir .out/iso
        mkdir .out/iso/boot
        mkdir .out/iso/boot/grub
        cp .out/kernel.bin .out/iso/boot/kernel.bin
        echo 'set timeout=0'                     >> .out/iso/boot/grub/grub.cfg
        echo 'set default=0'                     >> .out/iso/boot/grub/grub.cfg
        echo ''                                  >> .out/iso/boot/grub/grub.cfg
        echo \'menuentry "GevOS" {\' >> .out/iso/boot/grub/grub.cfg
        echo '  multiboot /boot/kernel.bin'      >> .out/iso/boot/grub/grub.cfg
        echo '  boot'                            >> .out/iso/boot/grub/grub.cfg
        echo '}'                                 >> .out/iso/boot/grub/grub.cfg
        cd ./.out

        grub-mkrescue --output=kernel.iso iso
        rm -rf iso

        qemu-system-x86_64 -cdrom kernel.iso -boot d -soundhw pcspk -serial mon:stdio -drive format=raw,file=../hdd.tar
        cd ../
    else
        if [ "$1" != "norun" ]
        then
            ninja run
        fi
    fi
fi

if [ "$1" != "norun" ]
then
    ninja -t clean
    cd Libraries/
    ninja -t clean
fi
