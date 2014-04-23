
## Install ORTD static binary
cp ../../../bin/libdyn_generic_exec_static ortd
echo "cd /data/local; chmod 755 ortd ; ls -l ; exit" | adb shell

## Copy & run the sensor example
adb push ortd /data/local
adb push ReadSensors.ipar /mnt/sdcard
adb push ReadSensors.rpar /mnt/sdcard
adb push ReadSensors.sh /mnt/sdcard

echo "cd /mnt/sdcard; /data/local/ortd  --baserate=10000 -s ReadSensors -i 901 -l 0" | adb shell
