@echo off
title Clear
color 3
for /r . %%B in (*.log) do (
	echo "%%B"
	del "%%B"
)
pause