import time
import os
import sys

WRITEDISK = True 
GCCPATH = '$HOME/opt/cross/bin/i686-elf-g++'
GPPPARAMS = '-ILibraries -fno-builtin -fno-use-cxa-atexit -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings'
QEMUPARAMS = '-cdrom kernel.iso -boot d -soundhw pcspk -serial mon:stdio -drive format=raw,file=../hdd.tar'
ARCH = 'i386'
ASPARAMS = '--32'

filesC = [
    './Kernel/kernel.cpp',
    './Kernel/multitasking.cpp',
    './Kernel/syscalls.cpp',
    './Kernel/tty.cpp',
    './Kernel/Hardware/port.cpp',
    './Kernel/Hardware/Drivers/mouse.cpp',
    './Kernel/Hardware/Drivers/keyboard.cpp',
    './Kernel/Hardware/Drivers/vga.cpp',
    './Kernel/Hardware/Drivers/cmos.cpp',
    './Kernel/Hardware/Drivers/ata.cpp',
    './Kernel/Hardware/Drivers/driver.cpp',
    './Kernel/Hardware/Drivers/amd79.cpp',
    './Kernel/Hardware/Drivers/pcspk.cpp',
    './Kernel/Hardware/interrupts.cpp',
    './Kernel/Hardware/pci.cpp',
    './Kernel/Mem/paging.cpp',
    './Kernel/Mem/mm.cpp',
    './Kernel/GDT/gdt.cpp',
    './Kernel/Net/arp.cpp',
    './Kernel/Net/etherframe.cpp',
    './Kernel/Net/ipv4.cpp',
    './Kernel/Exec/loader.cpp',
    './Kernel/Exec/elf.cpp',
    './Kernel/Filesystem/tar.cpp',
    './Kernel/Filesystem/fat.cpp',
    './Kernel/Filesystem/part.cpp',
    './Kernel/Filesystem/vfs.cpp',
    './Libraries/LibC/string.cpp',
    './Libraries/LibC/stdio.cpp',
    './Libraries/LibC/stdlib.cpp',
    './Libraries/LibC/liballoc.cpp',
    './Libraries/LibGUI/gui.cpp',
]

filesA = ['./Kernel/Hardware/interruptstubs.s']
objectfiles = []

def make_iso():
    os.system('mkdir .out/iso')
    os.system('mkdir .out/iso/boot')
    os.system('mkdir .out/iso/boot/grub')
    os.system('cp .out/kernel.bin .out/iso/boot/kernel.bin')
    # os.system("echo 'set vbemode=1024x768x32'                      > .out/iso/boot/grub/grub.cfg")
    os.system("echo 'set timeout=0'                      > .out/iso/boot/grub/grub.cfg")
    os.system("echo 'set default=0'                     >> .out/iso/boot/grub/grub.cfg")
    os.system("echo ''                                  >> .out/iso/boot/grub/grub.cfg")
    os.system('echo \'menuentry "GevOS" {\' >> .out/iso/boot/grub/grub.cfg')
    os.system("echo '  multiboot /boot/kernel.bin'      >> .out/iso/boot/grub/grub.cfg")
    os.system("echo '  boot'                            >> .out/iso/boot/grub/grub.cfg")
    os.system("echo '}'                                 >> .out/iso/boot/grub/grub.cfg")
    os.chdir('./.out')
    os.system('grub-mkrescue --output=kernel.iso iso')
    os.system('rm -rf iso')

    os.chdir('../Base')
    if WRITEDISK == True or not os.path.exists("../hdd.tar"):
        os.system('tar cf ../hdd.tar root/')
        os.system('mv hdd.tar ../')
        time.sleep(0.1)
    os.chdir('../.out')

    os.system('qemu-system-x86_64 ' + QEMUPARAMS)

def make_kernel():
    for file in filesC:
        os.system(GCCPATH + ' ' + GPPPARAMS + ' -c ' + file + ' -o ' + file[:-4] + '.o')
    for file in filesA:
        os.system('as ' + ASPARAMS + ' ' + file + ' -o' + file[:-2] + '.o')

def make_loader():
    os.system('as ' + ASPARAMS + ' Arch/' + ARCH + '/Boot/boot.S -o loader.o')

def link():
    for file in filesC:
        objectfiles.append(file[:-4] + '.o')
    for file in filesA:
        objectfiles.append(file[:-2] + '.o')

    if not os.path.isdir('./.out'):
        os.system('mkdir .out > /dev/null 2>&1')
    os.system('rm .out/*.iso > /dev/null 2>&1')
    os.system('rm .out/*.bin > /dev/null 2>&1')
    os.system('ld -melf_i386 -T linker.ld -o ./.out/kernel.bin loader.o ' + ' '.join(objectfiles))

def clean():
    os.system('rm *.o > /dev/null')
    ofolders = []
    for file in filesC:
        p = '/'.join(file.split('/')[0:-1])
        if p not in ofolders:
            ofolders.append(p)

    for file in filesA:
        p = '/'.join(file.split('/')[0:-1])
        if p not in ofolders:
            ofolders.append(p)

    for file in ofolders:
        os.system('rm ' + './' + file + '/*.o > /dev/null')
    if WRITEDISK == True:
        os.system('rm -rf ../hdd.tar')

def all():
    os.system("find ./ -iname *.hpp -o -iname *.cpp | xargs clang-format -i")
    os.system("python setup.py make kernel")
    os.system("python setup.py make loader")
    os.system("python setup.py link")
    os.system("python setup.py make iso")
    os.system("python setup.py clean")
    os.chdir("./.out/")

if len(sys.argv) < 2:
    all()
    exit()

if sys.argv[1] == 'make':
    if sys.argv[2] == 'kernel':
        make_kernel()
    if sys.argv[2] == 'loader':
        make_loader()
    if sys.argv[2] == 'iso':
        make_iso()
if sys.argv[1] == 'link':
    link()
if sys.argv[1] == 'clean':
    clean()
