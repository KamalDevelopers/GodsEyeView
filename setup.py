import os
import sys

os.chdir("./src/")
os.system("python setup.py make kernel")
os.system("python setup.py make loader")
os.system("python setup.py link")
os.system("python setup.py clean")
os.system("./out/qemu-system-i386 -kernel kernel.bin")