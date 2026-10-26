@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Tools\launch.ps1" -CouncilPreview
if errorlevel 1 pause
