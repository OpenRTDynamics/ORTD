#!/bin/bash

echo "main() {}" > tmpTest237823.c
$1 -c tmpTest237823.c -o tmpTest237823.o 2> /dev/null
#$2 $3 tmpTest237823.o -o tmpTest237823  || ( echo "not ok" ;  exit 1 )

# Try to link
$2 $3 tmpTest237823.o -o tmpTest237823 2> /dev/null
if (( $? == 0 )); then
  #echo "found <" $3 ">"
  
  rm -f tmpTest237823*
  exit 0
else
 #echo "not ok" 

 rm -f tmpTest237823*
 exit 1 
fi



