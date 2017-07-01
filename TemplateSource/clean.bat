REM 清理项目垃圾文件
del *.sdf *.log *.user *.ipch *.aps /s
del *.exe *.dll /s
del *.suo /s /a h
del *.ilk *.pdb *.lib *.tlog *.mainfest *.res /s
del *.obj *.pch *.codeanalysis *.codeanalysis/s
rmdir /s/q ipch Debug Release x64 .vs
del *.VC.db