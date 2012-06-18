#!/bin/sh

LDFLAGS=""
MACROS=""
SPACE=" "  ;\
for i in modules/* ; 
do 
  #echo $i
  if [ -f $i/LDFLAGS ] ; 
  then 
    #echo $i
    LDFLAGS=$SPACE$LDFLAGS$SPACE$(cat $i/LDFLAGS)$SPACE
  fi


  if [ -f $i/MACROS ] ; 
  then 
    #echo $i
    MACROS=$SPACE$MACROS$SPACE$(cat $i/MACROS)$SPACE
  fi

done 

echo LDFLAGS are: $LDFLAGS
echo MACROS are: $MACROS

# write to a file
echo $LDFLAGS > tmp/LDFALGS.list
echo $MACROS > tmp/MACROS.list
