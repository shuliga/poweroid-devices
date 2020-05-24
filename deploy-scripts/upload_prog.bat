@SET chip=%2
@SET baud=%4
@SET port=%3
@IF "%1"=="" (
 @ECHO Target .hex file name missing
 GOTO :END
)
@IF "%2"=="" (
 SET chip="atmega328p"
)
IF "%3"=="" (
 SET port="COM6"
)
@IF "%4"=="" (
 SET baud="115200"
)
@SET avr_path=C:\Users\SHL\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino14
@SET project_path="D:\My Docs\Dev\Poweroid\poweroid-devices"
%avr_path%\bin\avrdude -C%avr_path%\etc\avrdude.conf -v -p %chip% -cstk500v1 -P%port% -b %baud% -Uflash:w:%project_path%\cmake-build-release\%1:i
:END
