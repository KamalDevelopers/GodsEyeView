import os
import sys

GCCPATH = '$HOME/opt/cross/bin/i686-elf-g++'
PROGRAM = 'demo'
PATH = ''

def make():
    os.system(GCCPATH + " -c " + PROGRAM + ".cpp -o " + PROGRAM + ".o -ffreestanding -O2 -Wall -Wextra")

def link():
    os.system(GCCPATH + " -T link.ld -o " + PROGRAM + " -ffreestanding -O2 -nostdlib " + PROGRAM + ".o  -lgcc")
    os.rename(PROGRAM, "../../../Base/root/" + PATH + PROGRAM)

def clean():
    os.system("rm *.o")

if sys.argv[1] == 'make':
    make()
if sys.argv[1] == 'link':
    link()
if sys.argv[1] == 'clean':
    clean()
