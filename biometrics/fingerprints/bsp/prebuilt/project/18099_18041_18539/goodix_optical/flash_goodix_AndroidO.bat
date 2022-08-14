adb wait-for-device
adb remount
adb shell setenforce 0
adb shell rm /vendor/firmware/goodixfp* 
adb push goodixfp.b00 /vendor/firmware/goodixfp.b00
adb push goodixfp.b01 /vendor/firmware/goodixfp.b01
adb push goodixfp.b02 /vendor/firmware/goodixfp.b02
adb push goodixfp.b03 /vendor/firmware/goodixfp.b03
adb push goodixfp.b04 /vendor/firmware/goodixfp.b04
adb push goodixfp.b05 /vendor/firmware/goodixfp.b05
adb push goodixfp.b06 /vendor/firmware/goodixfp.b06
adb push goodixfp.b07 /vendor/firmware/goodixfp.b07
adb push goodixfp.mdt /vendor/firmware/goodixfp.mdt
adb shell setenforce 1
adb shell sync
pause


