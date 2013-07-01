#!/bin/bash
#
# A build script to automatically build Openrtdynamics
# for android
#
#
#



# Please enter the path to the android NDK toolchain "bin" subdirectory
ANDROID_TOOLCHAIN_BIN=$HOME/bin/AndroidArmToolchain/bin/


# The name of the created package
folder=OpenRTDynamics_android

########################################
########################################


rm -rf $folder

svn checkout svn://svn.code.sf.net/p/openrtdynamics/code/trunk $folder
cd $folder

make clean
#svn up

# Remove module EDF as it does not compile for Android
rm -rf modules/EDF

# Prepare Cross compiler
export PATH=$ANDROID_TOOLCHAIN_BIN:$PATH
export CC=arm-linux-androideabi-gcc   # or export CC=clang
export CXX=arm-linux-androideabi-g++

# configure target
echo "ANDROID_ARM" > target.conf

# Compile
#bash ../prepare.sh
make
make cleanBuildFiles
cd ..

mkdir bin
mkdir lib

cp $folder/bin/ortd bin/
cp $folder/bin/ortd_static bin/
cp $folder/libortd.so lib/

tar -c $folder | gzip -v9 > $folder.tgz

cat $folder/bin/ortd_static | gzip -v9 > ortd_static.gz

# create a selft installable shell archieve
chmod +x trunk/BuildScripts/InstallShellArchieve.sh
sh $folder/thirdparty/makeself-2.1.5/makeself.sh $folder ORTDInstall.sh OpenRTDynamics ./BuildScripts/InstallShellArchieve.sh



# Upload the files to the ortd website
scp ortd_static.gz              christianausb@frs.sourceforge.net:/home/project-web/openrtdynamics/htdocs/download/Android/current
scp $folder.tgz  christianausb@frs.sourceforge.net:/home/project-web/openrtdynamics/htdocs/download/Android/current

rm -rf $folder

exit


# Download the files with
#
# wget http://openrtdynamics.sf.net/download/Android/current/ortd_static.gz
# wget http://openrtdynamics.sf.net/download/Android/current/OpenRTDynamics_android.tgz
#
#

## Just install the executable
cd
mkdir bin
cd bin
rm -f ortd_static ortd 
rm -f libdyn_generic_exec_static 
rm -f libdyn_generic_exec

wget http://openrtdynamics.sf.net/download/Android/current/ortd_static.gz
gzip -d ortd_static.gz
chmod +x ortd_static

ln -s ortd_static ortd
ln -s ortd_static libdyn_generic_exec
ln -s ortd_static libdyn_generic_exec_static


## Install the whole compiled framework including the examples
cd
wget http://openrtdynamics.sf.net/download/Android/current/OpenRTDynamics_android.tgz
tar xfvz OpenRTDynamics_android.tgz
rm OpenRTDynamics_android.tgz


