#!/bin/bash

# this is beeing run from the main directory of ortd

echo "This is the installer for ORTD"
ls -l 

export SOURCE_DIR=`pwd`

# detect target type

TARGET=`cat target.conf`

echo Target is $TARGET


if [ "$TARGET" == "LINUX" ]; then
  echo "Generic Linux target"


fi

if [ "$TARGET" == "ANDROID_ARM" ]; then
#if [ "$TARGET" == "LINUX" ]; then
  echo "Generic Linux target"


  # install static version of ortd into ~/bin
  (
    cd
    mkdir bin
    cd bin
    rm -f ortd_static ortd 
    rm -f libdyn_generic_exec_static 
    rm -f libdyn_generic_exec

    cp "$SOURCE_DIR/bin/ortd_static" .
    chmod +x ortd_static

    ln -s ortd_static ortd
    ln -s ortd_static libdyn_generic_exec
    ln -s ortd_static libdyn_generic_exec_static
  )

fi


