@echo off
color 3
title RunAllServer.bat

start 1_AdaptServer.bat
start 2_HallServer.bat
ping -n 3 127.0.0.1

start 3_GateWayServer.bat
start 4_GameServer.bat
ping -n 3 127.0.0.1

start 5_HttpServer.bat
