#!/bin/python3

import math
import os
import sys

if len(sys.argv) < 3:
    print("align.py [file] [alignment]")
    print("padds 0's at end of file.")
    exit()

file_stats = os.stat(sys.argv[1])
size = file_stats.st_size
sects = math.floor(size / int(sys.argv[2]))
overflow = size - sects * int(sys.argv[2])
extra = int(sys.argv[2]) - overflow

with open(sys.argv[1], "ab") as f:
    f.write(("\0"*extra).encode())
