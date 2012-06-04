#!/bin/sh

if [ -f ../../scilabdir.conf ] 
then 
  scilabexec=$(readlink -m  `cat ../../scilabdir.conf`/bin/scilab)
  echo scilab exec path is $scilabexec 
  echo '#define SCILAB_EXEC ''"'$scilabexec'"'  > scilabconf.h 
else 
  echo "#define SCILAB_EXEC "'"' "NOT FOUND DURING INSTALLATION"'"' > scilabconf.h 
fi