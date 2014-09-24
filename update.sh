#!/bin/sh

svn upgrade
svn up
make clean
make
make scilabhelp

echo
echo "***********************************************************************"
echo "Note: Please keep in mind to re-compile all the plug-ins you are using,"
echo "      as the ABI (Application Binary Interface) might have changed!"
echo "***********************************************************************"
echo
echo "Now do make install or make homeinstall to install the new version"
echo

