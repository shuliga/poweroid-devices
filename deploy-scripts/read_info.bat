@SET chip=%1
@SET port=%2
@SET baud=%3
@IF "%1"=="" (
 SET chip="atmega328p"
)
@IF "%2"=="" (
 SET port=COM6
)
@IF "%3"=="" (
 SET baud=115200
)
@IF "%avr_path%"=="" (
 SET avr_path=C:\Users\SHL\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino14
)
%avr_path%\bin\avrdude -C%avr_path%\etc\avrdude.conf -v -p %chip% -cstk500v1 -P%port% -b %baud%
