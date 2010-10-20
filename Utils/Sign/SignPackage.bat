@echo off
echo Signing %1 as "SIL Software Package"
"%MASTER_INSTALLER%\Utils\SignFile.js" %1 "SIL Software Package"