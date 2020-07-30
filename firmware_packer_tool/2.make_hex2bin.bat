
@ECHO OFF
set "b=%cd%"
set source=..\tuya_ble_sdk_demo.cydsn\CortexM4\ARM_GCC_541\Debug
set h=%time:~0,2%
set h=%h: =0%
set timestamp=%date:~2,2%%date:~5,2%%date:~8,2%%h%


echo %cd%

echo hex2bin ...
.\hex2bin -k 1 .\tuya_ble_sdk_Demo_Project_signed.hex


echo Del temp file ...
del %b%\tuya_ble_sdk_Demo_Project_signed.hex


pause