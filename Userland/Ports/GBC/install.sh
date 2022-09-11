#!/bin/sh

git clone "https://github.com/koenk/gbc"
cp patch.patch ./gbc/
cd gbc
git apply patch.patch
make
cp ./gbc ../../../../Base/root/bin
cd ..
rm -f -r gbc
