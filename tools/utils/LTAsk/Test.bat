
@echo off

debug\LTAsk "I am asking you a question!  (y/n)"
if not errorlevel 1 echo "You said No!"
if errorlevel 1 echo "You said Yes!"
