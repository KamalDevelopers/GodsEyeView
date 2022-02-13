#!/bin/bash

if [ -e dr_wav.h ]
then
    exit
fi

wget "https://raw.githubusercontent.com/mackron/dr_libs/c729134b41cf09542542b5da841ac2f933b36cba/dr_wav.h"
patch -p1 dr_wav.h dr_wav.patch
