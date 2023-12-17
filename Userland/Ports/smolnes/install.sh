#!/bin/sh

git clone "https://github.com/binji/smolnes"
cp patch.patch ./smolnes/
cd smolnes
git apply patch.patch
ninja
ninja move
cd ..
rm -f -r smolnes
