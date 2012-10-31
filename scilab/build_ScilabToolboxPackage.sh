cp -a ld_toolbox/ ortd
rm -rf `find ortd/ -name "*.svn" `
tar -c ortd | gzip -v9 > ORTD_ScilabToolbox_`date +%F`.tgz
