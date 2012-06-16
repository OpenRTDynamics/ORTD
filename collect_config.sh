#!/bin/sh

LDFLAGS=""
SPACE=" "  ;\
for i in modules/* ; 
do 
  #echo $i
  if [ -f $i/LDFLAGS ] ; 
  then 
    #echo $i
    LDFLAGS=$SPACE$LDFLAGS$SPACE$(cat $i/LDFLAGS)$SPACE
  fi
done 

echo LDFLAGS are: $LDFLAGS

# write to a file
echo $LDFLAGS > tmp/LDFALGS.list