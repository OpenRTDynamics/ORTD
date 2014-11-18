#
# Renames the module by renameing all occurances of "TemplateModule_V2" by the name of the module's folder
# This can *only* be called once and you should delete SetModuleName.sh afterwards!
#

# Get the name of the current folder
ModuleName=$(basename `pwd`)

echo "Module will be renamed to" $ModuleName
 

(
cat TemplateModule_V2_blocks.cpp | sed s/TemplateModule_V2/$ModuleName/g  > "$ModuleName"_blocks.cpp

cp Makefile Makefile.tmp
cat Makefile.tmp | sed s/TemplateModule_V2/$ModuleName/g > Makefile
rm Makefile.tmp

cat pluginLoader.cpp | sed s/TemplateModule_V2/$ModuleName/g  > pluginLoader.cpp
) || (
  echo "Something failed!"
)

#cd ..
#mv TemplateModule_V2 $ModuleName




