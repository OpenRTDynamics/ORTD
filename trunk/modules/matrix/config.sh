#!/bin/bash

# Create default config
echo " " > LDFLAGS
echo "no" > CONFIG_WITH_blas_AND_gsl



# Check wheter the given Compilder ($1) and Linker ($2) are able to link to the
# libraries

echo "Checking for blas and gsl"

bash checklib.sh $1 $2 "-lblas"
if (( $? == 1 )); then
  echo "NOTE: libblas not found, functionality is reduced"
  exit
fi


bash checklib.sh $1 $2 "-lgsl -lblas"
if (( $? == 1 )); then
  echo "NOTE: libgsl not found, functionality is reduced"
  exit
fi


echo "Found both!"


# Create configuration
echo "-lm -lgsl -lgslcblas " > LDFLAGS
echo "yes" > CONFIG_WITH_blas_AND_gsl




exit 0


