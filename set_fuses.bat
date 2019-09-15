SET fuses=%1
SET baud=%3
SET chip=%2
SET baud=%3
IF "%2"=="" (
 SET chip="atmega328p"
)
IF "%3"=="" (
 SET baud="115200"
)
C:\Users\SHL\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino9\bin\avrdude -CC:\Users\SHL\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino9\etc\avrdude.conf -v -p %chip% -cstk500v1 -PCOM3 -b %baud% -U lfuse:w:0xFF:m	-U hfuse:w:0xDE:m -U efuse:w:0xFC:m
