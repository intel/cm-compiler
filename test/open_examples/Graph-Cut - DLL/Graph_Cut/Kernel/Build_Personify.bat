@echo off

if "%1" == "" goto HELP


icl /c /Qxcm  personify_genx.cpp /mCM_emit_common_isa  /Qxcm_nonstrict  

move /Y Personify_genx.isa ..\Personify_bin\Personify_genx_%1.isa

del *.obj
del .\Debug\*.tlog
goto :EOF

:HELP
echo.
echo Usages:
echo Build_Personify.bat [MDF version] = mdf30 ^| mdf40 ^| mdf50                
echo.

