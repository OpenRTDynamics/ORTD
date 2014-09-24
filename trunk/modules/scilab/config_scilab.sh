#!/bin/sh

if [ -f ../../scilabdir.conf ] 
then 
  { # try
    scilabexec=$(readlink -m  `cat ../../scilabdir.conf`/bin/scilab)
  } || { # catch
    scilabexec=$(echo `cat ../../scilabdir.conf`/bin/scilab)
  }
  echo scilab exec path is $scilabexec 
  echo '#define SCILAB_EXEC ''"'$scilabexec'"'  > scilabconf.h 
else 
  
  if which scilab
  then
    echo "Found a command named >scilab< in your system" 
    scilabexec=$(which scilab)
    echo scilab exec path is $scilabexec 
    #echo '#define SCILAB_EXEC ''"'$scilabexec'"'  
    echo '#define SCILAB_EXEC ''"'$scilabexec'"'  > scilabconf.h 
  else
    echo "#define SCILAB_EXEC "'"' "NOT FOUND DURING INSTALLATION"'"' > scilabconf.h 
  fi
fi