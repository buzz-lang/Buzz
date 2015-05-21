#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 script.bzz"
    exit 1
fi

#
# File names
#
BZZ=$1
BASM=$(echo $BZZ | rev | cut -d. -f 2- | rev).basm
BO=$(echo $BZZ | rev | cut -d. -f 2- | rev).bo

#
# Tools
#
BUZZC=../../build/buzzc
BUZZASM=../../build/buzzasm

#
# Compilation
#
$BUZZC $BZZ $BASM && $BUZZASM $BASM $BO

