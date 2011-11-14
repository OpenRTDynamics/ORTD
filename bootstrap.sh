#!/bin/sh

zenity --info --text "Please choose a directory, where you like to place ORTD. This directory has to be writeable by you. A subdirectory will be created."

if ortdinstdir=`zenity --file-selection --directory`
then
  echo "ok"
else
  echo "Installation aborted"
  exit
fi


#if zenity --question --text "Shall I install a relase (stable) version? Otherwise the latest version from svn will be used."
if false
then
  cd $ortdinstdir
  #echo "not implemented"

  #wget "http://sourceforge.net/projects/openrtdynamics/files/latest/download?source=files" -O ortd.tgz
  wget http://sourceforge.net/projects/openrtdynamics/files/ortd-0.99g.tgz/download -O ortd.tgz
  tar xfvz ortd.tgz
  rm ortd.tgz
  cd v0.99g

else
  cd $ortdinstdir

  echo "Installing ORTD into $ortdinstdir ..."

  if which svn
  then
    echo "found svn command"
  else
    zenity --info --text "Subversion is not installed, which is required to check out the latest version. Under Ubuntu you can install it with: sudo apt-get install subversion"
    exit
  fi

  svn co https://openrtdynamics.svn.sourceforge.net/svnroot/openrtdynamics/trunk openrtdynamics/trunk
  cd openrtdynamics/trunk

fi

# Build
make

if zenity --question --text "Shall I install the libraries into the system? Root access is required."
then
  echo
  echo "----------------------------------------------------------------"
  echo "Root access is required to install the binaries into your system"
  echo "enter your sudo password"
  echo "----------------------------------------------------------------"
  echo
  sudo make install
fi

make install_toolbox

