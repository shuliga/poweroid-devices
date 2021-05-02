@SET target=%1
@SET chip=%2
@SET port=%3
@SET baud=%4
@IF "%1"=="" (
 @ECHO Target .hex file name missing
 @ECHO Parameters: ^<filename.hex^> ^<chip^> ^<port^> ^<baud^>
 @EXIT /b 1
)
@IF "%2"=="" (
 SET chip="atmega328p"
)
@IF "%3"=="" (
 SET port="COM6"
)
@IF "%4"=="" (
 SET baud="115200"
)
@IF "%AVR_PATH%"=="" (
    SET AVR_PATH=C:\Users\SHL\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino14
)
@SET project_path=.
@EXIT /b 0
