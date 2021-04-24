#!/bin/sh

gcc queso.c -l pthread -o bin/queso_x64 # For 64 bit version
gcc queso.c -l pthread -o bin/queso_x64 -m32 # For 32 bit version
echo "Your binaries are in bin folder"