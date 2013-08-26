#!/bin/bash
#
# A build script to pack the sources of Openrtdynamics into an self-extracting archieve
# 
#
# e.g.  $ bash ./trunk/AutoBuild/LINUX_source/build.sh this trunk OpenRTDynamics_source
#



# Please enter the path to the android NDK toolchain "bin" subdirectory
# ANDROID_TOOLCHAIN_BIN=$HOME/bin/AndroidArmToolchain/bin/


# The name of the created package

#  either "svn" or "this"
SOURCE=$1

########################################
########################################


if [ "$SOURCE" = "svn" ]
then

  folder=$2 #OpenRTDynamics_source


  rm -rf $folder
  svn checkout svn://svn.code.sf.net/p/openrtdynamics/code/trunk $folder





  # update remove this!
  #cp ../../BuildScripts/InstallShellArchieve.sh $folder/BuildScripts


  cd $folder

  make superclean
  #svn up

  # Remove module EDF as it does not compile for Android
  # rm -rf modules/EDF

  # # Prepare Cross compiler
  # export PATH=$ANDROID_TOOLCHAIN_BIN:$PATH
  # export CC=arm-linux-androideabi-gcc   # or export CC=clang
  # export CXX=arm-linux-androideabi-g++

  # configure target
  echo "LINUX" > target.conf

  cd ..

  tar -c $folder | gzip -v9 > $folder.tgz

  # cat $folder/bin/ortd_static | gzip -v9 > ortd_static.gz

  # create a selft installable shell archieve
  chmod +x $folder/BuildScripts/InstallShellArchieve.sh
  sh $folder/thirdparty/makeself-2.1.5/makeself.sh --notemp $folder ORTDInstall_source.sh "OpenRTDynamics Android" ./BuildScripts/InstallShellArchieve.sh
  #sh $folder/thirdparty/makeself-2.1.5/makeself.sh          $folder ORTDInstall_android.sh "OpenRTDynamics Android" ./BuildScripts/InstallShellArchieve.sh

  #sh $folder/thirdparty/makeself-2.1.5/makeself.sh $folder ORTDInstall_android.sh "OpenRTDynamics Android" ./BuildScripts/InstallShellArchieve.sh



  # Upload the files to the ortd website
  # scp ortd_static.gz              christianausb@frs.sourceforge.net:/home/project-web/openrtdynamics/htdocs/download/Android/current
  # scp $folder.tgz  christianausb@frs.sourceforge.net:/home/project-web/openrtdynamics/htdocs/download/Android/current

  #rm -rf $folder

  exit

fi



if [ "$SOURCE" = "this" ]
then

  tmpdir=`mktemp -d`
  ArchName=$3
  folder="$tmpdir/$ArchName"

  cp -av $2 $folder

#   folder=$2
  dstdir=`pwd`

 
  

  echo "Building source package for $folder ---> $destdir"

  pushd .

  cd "$folder"

# pwd
# exit

  # remove the folders created by subversion
  # this does not follow symlinks
#  find -name "*.svn" -print0 | xargs -0 rm -rf

  make superclean

  # configure target
  echo "LINUX" > target.conf

  # # Compile
  # make
  # make cleanBuildFiles
  popd


  # create a selft installable shell archieve
  chmod +x $folder/BuildScripts/InstallShellArchieve.sh
  sh $folder/thirdparty/makeself-2.1.5/makeself.sh --notemp $folder "$ArchName.sh" "OpenRTDynamics Source" ./BuildScripts/InstallShellArchieve.sh

  echo "Delete temp folder $tmpdir"
  rm -rf "$tmpdir/$ArchName"
  rmdir $tmpdir

  exit
fi






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


