import os
import sys
py = sys.executable

for (dirpath, dirnames, filenames) in os.walk("./"):
    if dirpath != "./":
        os.chdir(dirpath)
        print("Building:", dirpath)
        os.system(py + " build.py make")
        os.system(py + " build.py link")
        os.system(py + " build.py clean")
        os.chdir("../")
