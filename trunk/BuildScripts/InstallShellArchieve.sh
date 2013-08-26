#!/bin/sh

# this is beeing run from the main directory of ortd

echo "This is the installer for ORTD"
ls -l 

SOURCE_DIR=`pwd`

# detect target type

TARGET=`cat target.conf`

echo Target is $TARGET


if [ "$TARGET" = "LINUX" ]
then
  echo "Generic Linux target"
  echo "Compiling from source ..."
  echo 
  sleep 1
#  echo "Doing nothing"

  (  
    cd "$SOURCE_DIR/"
    make clean
    make install_toolbox
    make config
    make
    make demo
  ) || (
    echo "There was an error..."
    echo "Make sure you have the commands gcc, g++, make, perl, bash"
    echo "On Ubuntu: sudo apt-get install g++ make build-essential"
    exit -1
  )

  # compilation was ok
  (
    echo "Compilation was successfull"
    echo "Results were placed into $SOURCE_DIR"
    echo

    if zenity --question --text "Shall I install the libraries into the system? Root access is required herefore."
    then
      echo
      echo "----------------------------------------------------------------"
      echo "Root access is required to install the binaries into your system"
      echo "enter your sudo password"
      echo "----------------------------------------------------------------"
      echo
      cd "$SOURCE_DIR"
      sudo make install
    fi

    if zenity --question --text "Shall I install the libraries into your home directory within ~/bin ?"
    then
      cd "$SOURCE_DIR"
      sudo make homeinstall
    fi

    echo
    echo
    echo "****************************************************"
    echo
    echo "Compilation was successfull"
    echo "Results were placed into $SOURCE_DIR"
    echo
    echo "Run "
    echo
    echo "  make demo"
    echo
    echo "from this folder to see a demo"
    echo
    echo "For more information have a look at the README file placing in this folder."
    echo
  )



fi

if [ "$TARGET" = "ANDROID_ARM" ]; then
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


