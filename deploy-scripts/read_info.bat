call "%~dp0env.cmd" %1 %2 %3 %4
@if %ERRORLEVEL% EQU 0 (
    %AVR_PATH%\bin\avrdude -C%AVR_PATH%\etc\avrdude.conf -v -p %chip% -cstk500v1 -P%port% -b %baud%
)
