@IF "%1"=="" (
 @ECHO Target flags file name missing
 GOTO :END
)
@SET flag_file=%1
@SET chip=%2
@SET port=%3
@SET baud=%4
@IF "%2"=="" (
 SET chip="atmega328p"
)
@IF "%3"=="" (
 SET port="COM6"
)
@IF "%4"=="" (
 SET baud="115200"
)
@IF "%avr_path%"=="" (
 SET avr_path=C:\Users\SHL\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino14
)
%avr_path%\bin\avrdude -C%avr_path%\etc\avrdude.conf -v -p %chip% -cstk500v1 -P%port% -b %baud% -U eeprom:w:%flag_file%:i
:END