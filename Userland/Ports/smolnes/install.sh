#!/bin/sh

git clone "https://github.com/binji/smolnes"
cp patch.patch ./smolnes/
cd smolnes
git checkout a4100a2
git apply patch.patch

ninja
ninja move
cd ..
rm -f -r smolnes
