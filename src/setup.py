import os
import sys
from colorama import *
init()

GPPPARAMS = "-m32 -Ilibraries/LibC -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings"
ASPARAMS  = "--32"

filesC = [
	"./kernel/kernel.cpp",
	"./kernel/multitasking.cpp",
	"./kernel/Hardware/port.cpp",
	"./kernel/Hardware/Drivers/mouse.cpp",
	"./kernel/Hardware/Drivers/keyboard.cpp",
	"./kernel/Hardware/Drivers/vga.cpp",
	"./kernel/GDT/gdt.cpp",
	"./kernel/Hardware/interrupts.cpp",

	"./libraries/LibC/stdio.cpp",
	"./libraries/LibC/stdlib.cpp",
	"./libraries/LibGUI/gui.cpp"
]

filesA = [
	"./kernel/Hardware/interruptstubs.s"
]

h = """
setup.py make kernel - generate kernel object files
setup.py make loader - generate loader object file
setup.py link        - link files and generate kernel.bin
setup.py clean       - remove all generated object files
"""

objectfiles = []

def make_iso():
	os.system("mkdir out/iso")
	os.system("mkdir out/iso/boot")
	os.system("mkdir out/iso/boot/grub")
	os.system("cp out/kernel.bin out/iso/boot/kernel.bin")
	os.system("echo 'set timeout=0'                      > out/iso/boot/grub/grub.cfg")
	os.system("echo 'set default=0'                     >> out/iso/boot/grub/grub.cfg")
	os.system("echo ''                                  >> out/iso/boot/grub/grub.cfg")
	os.system("echo 'menuentry \"GevOS\" {' >> out/iso/boot/grub/grub.cfg")
	os.system("echo '  multiboot /boot/kernel.bin'    >> out/iso/boot/grub/grub.cfg")
	os.system("echo '  boot'                            >> out/iso/boot/grub/grub.cfg")
	os.system("echo '}'                                 >> out/iso/boot/grub/grub.cfg")
	os.chdir("./out")
	os.system("grub-mkrescue --output=kernel.iso iso")
	os.system("rm -rf iso")
	os.system("qemu-system-i386 -cdrom kernel.iso -soundhw pcspk")

def make_kernel():
	for file in filesC:
		os.system("g++ " + GPPPARAMS + " -c " + file + " -o" + file[:-4] + ".o")

	for file in filesA:
		os.system("as " + ASPARAMS + " " + file + " -o" + file[:-2] + ".o")
		
def make_loader():
	os.system("as " + ASPARAMS + " loader.s -o loader.o")

def link():
	for file in filesC:
		objectfiles.append(file[:-4] + ".o")
	for file in filesA:
		objectfiles.append(file[:-2] + ".o")

	if not (os.path.isdir("./out")):
		os.system("mkdir out")

	os.system("rm out/*.iso")
	os.system("rm out/*.bin")
	os.system("ld -melf_i386 -T linker.ld -o ./out/kernel.bin loader.o " + ' '.join(objectfiles))

def clean():
	os.system("rm *.o")
	os.system("rm ./kernel/*.o")
	os.system("rm ./kernel/Hardware/*.o")
	os.system("rm ./kernel/GDT/*.o")
	os.system("rm ./kernel/Hardware/Drivers/*.o")
	os.system("rm ./libraries/LibGUI/*.o")

if len(sys.argv) < 2:
	print(Fore.RED + "Invalid amount of arguments\n" + Style.RESET_ALL + h)
	sys.exit()

if sys.argv[1] == "make":
	if sys.argv[2] == "kernel":
		make_kernel()

	if sys.argv[2] == "loader":
		make_loader()

	if sys.argv[2] == "iso":
		make_iso()

if sys.argv[1] == "link":
	link()

if sys.argv[1] == "clean":
	clean() 

if sys.argv[1] == "--help" or sys.argv[1] == "help" or sys.argv[1] == "-h":
	print(h)