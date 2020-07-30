
@ECHO OFF
set "b=%cd%"
set source=..\tuya_ble_sdk_Demo_Project.cydsn\CortexM4\ARM_GCC_541\Debug
set h=%time:~0,2%
set h=%h: =0%
set timestamp=%date:~2,2%%date:~5,2%%date:~8,2%%h%


echo %cd%

echo Copy application.hex file ...
copy  %source%\tuya_ble_sdk_Demo_Project.hex  .\
copy  %source%\tuya_ble_sdk_Demo_Project_signed.hex  .\

pause