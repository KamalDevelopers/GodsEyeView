#!/bin/sh

git clone "https://github.com/magetron/c-interpreter"
cp patch.patch ./c-interpreter/
cd c-interpreter
git apply patch.patch
ninja
cp ./pcc ../../../../Base/root/bin
cd ..
rm -f -r c-interpreter
