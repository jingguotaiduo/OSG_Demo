@echo off
if not exist *.lib (
echo This directory contains no lib files.
) else (
   echo ********This directory contains the following lib files********
   echo.
   dir /b *.lib
)