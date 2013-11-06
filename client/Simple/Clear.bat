@echo off
title Clear
color 3
for /r . %%B in (*.log *.obj *.manifest *.res *.lastbuildstate *.pdb *.rc *.tlog *.idb *.exp *.ilk *.sdf *.rar *.exe *.sdf *.ipch *.a *.so) do (
	echo "%%B"
	del "%%B"
)
pause