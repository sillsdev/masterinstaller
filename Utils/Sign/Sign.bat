@echo off
echo Signing %1 as "SIL Software Installer"
"%MASTER_INSTALLER%\Utils\SignFile.js" %1 "SIL Software Installer"