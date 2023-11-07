#!/bin/python3
import os

""" stage 1 """
os.system("nasm Entry/entry.asm -f bin -o entry_boot.bin")


""" stage 2 """
os.system("i686-elf-gcc -O3 -fno-asynchronous-unwind-tables -s -c -o bootloader.o bootloader.c")
os.system("objconv -v0 -fnasm bootloader.o c.compiled.asm")

with open("c.compiled.asm") as f:
    assembly_gen = f.read()

assembly_gen.replace("align=1 exec", "").replace("align=1 noexec", "")
parts = assembly_gen.split("\n")

i = 0
while i < len(parts):
    if "global " in parts[i]:
        del parts[i]
        i -= 1
    if "extern" in parts[i]:
        del parts[i]
        i -= 1
    if "SECTION" in parts[i]:
        del parts[i]
        i -= 1
    i += 1

parts = [i for i in parts if i]
assembly_gen = '\n'.join(parts)
with open("c.compiled.asm", "w") as f:
    f.write("\n" + assembly_gen + "\n")

os.system("cat bootloader.asm c.compiled.asm end.asm > fin.compiled.asm")
os.system("nasm fin.compiled.asm -f bin -o boot.bin")
os.system("cat entry_boot.bin boot.bin > bootloader.bin")
os.system("cp ./bootloader.bin ../bootloader.bin")
