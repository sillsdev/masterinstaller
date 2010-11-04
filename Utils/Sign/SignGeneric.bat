@echo off
echo Signing %1 as %2
"%MASTER_INSTALLER%\Utils\SignFile.js" %1 %2