@echo off
title SRM IST Campus Route Planner - Web Server
color 0b

echo ==================================================================
echo   SRM INSTITUTE OF SCIENCE AND TECHNOLOGY, DELHI-NCR CAMPUS
echo         Interactive Campus Route Planner & Navigator
echo ==================================================================
echo.
echo  Starting local web server on port 3000...
echo.

start "" "http://localhost:3000"
node server.js

pause
