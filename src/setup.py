import os
import sys
from colorama import *
init()

GPPPARAMS = "-m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings"
ASPARAMS  = "--32"
PATH      = "./kernel/"
h = """
setup.py make kernel - generate kernel object files
setup.py make loader - generate loader object file
setup.py link        - link files and generate kernel.bin
setup.py clean       - remove all generated object files
"""

files = os.listdir(PATH)
headerfiles_names = []
headerfiles = []
cppfiles_names = []
cppfiles = []

for file in files:
	filename, file_extension = os.path.splitext(PATH + file)
	if file_extension == ".cpp":
		cppfiles.append(PATH + file)
		cppfiles_names.append(filename)

	if file_extension == ".h":
		headerfiles.append(PATH + file)
		headerfiles_names.append(filename)

def make_kernel():
	for cppfile in cppfiles_names:
		os.system("g++ " + GPPPARAMS + " -c " + cppfile + ".cpp -o" + cppfile + ".o")

def make_loader():
	os.system("as " + ASPARAMS + " loader.s -o loader.o")

def link():
	if not (os.path.isdir("./out")):
		os.system("mkdir out")

	os.system("ld -melf_i386 -T linker.ld -o ./out/kernel.bin loader.o ./kernel/kernel.o")

def clean():
	os.system("rm *.o")
	os.system("rm ./kernel/*.o")

if len(sys.argv) < 2:
	print(Fore.RED + "Invalid amount of arguments\n" + Style.RESET_ALL + h)
	sys.exit()

if sys.argv[1] == "make":
	if sys.argv[2] == "kernel":
		make_kernel()

	if sys.argv[2] == "loader":
		make_loader()

if sys.argv[1] == "link":
	link()

if sys.argv[1] == "clean":
	clean() 

if sys.argv[1] == "--help" or sys.argv[1] == "help" or sys.argv[1] == "-h":
	print(h)