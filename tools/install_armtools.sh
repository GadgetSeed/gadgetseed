#!/bin/bash

echo "The installation of the GNU Arm Embedded Toolchain will start."

#arcdir="9-2019q4"
arcdir="10-2020q4"
#toolname="gcc-arm-none-eabi-9-2019-q4-major"
toolname="gcc-arm-none-eabi-10-2020-q4-major"

arcfile=$toolname"-x86_64-linux.tar.bz2"
#arcfile=$toolname"-mac.tar.bz2"

installpath="/opt"

echo "Install path :" $installpath

wget https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/$arcdir/$arcfile

sudo tar xvfj $arcfile -C $installpath

echo 'Set PATH for .bash_aliases'
echo 'PATH=$PATH:/opt/'$toolname'/bin' >> ~/.bash_aliases
