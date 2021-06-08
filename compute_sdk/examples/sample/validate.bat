::========================== begin_copyright_notice ============================
::
:: Copyright (C) 2020-2021 Intel Corporation
::
:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files (the "Software"),
:: to deal in the Software without restriction, including without limitation
:: the rights to use, copy, modify, merge, publish, distribute, sublicense,
:: and/or sell copies of the Software, and to permit persons to whom the
:: Software is furnished to do so, subject to the following conditions:
::
:: The above copyright notice and this permission notice (including the next
:: paragraph) shall be included in all copies or substantial portions of the
:: Software.
::
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
:: FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
:: IN THE SOFTWARE.
::
:: SPDX-License-Identifier: MIT
::=========================== end_copyright_notice =============================

@echo off

if "%CSDK_IGC%"=="" goto SETUP
if "%VSCMD_ARG_HOST_ARCH%"=="" goto MSVC

echo INFO: creating binaries...
cmake %CSDK_IGC%\examples\sample
cmake --build . --target install

echo INFO: binaries created in 'bin' directory
cd bin

vector.shim 2>shim.run
findstr PASSED shim.run >nul 2>nul
IF "%ERRORLEVEL%"=="1" ((echo ERROR: Run failed - see shim.run output) && GOTO :EOF)

echo INFO: Checking SHIM Layer
vector.shim 2>shim.run
findstr PASSED shim.run >nul 2>nul
IF "%ERRORLEVEL%"=="1" ((echo ERROR: Run failed - see shim.run output) && GOTO :EOF)

echo INFO: Checking running on GEN9 HW
if exist igdrcl.config del igdrcl.config
vector.skl 2>skl.run
findstr PASSED skl.run >nul 2>nul
IF "%ERRORLEVEL%"=="1" ((echo ERROR: Run failed - see skl.run output) && GOTO :EOF)



echo INFO: Checking SHIM(L0) Layer
vector.l0.shim 2>shim.l0.run
findstr PASSED shim.l0.run >nul 2>nul
IF "%ERRORLEVEL%"=="1" ((echo ERROR: Run failed - see shim.l0.run output) && GOTO :EOF)

echo INFO: Checking running on GEN9(L0) HW
if exist igdrcl.config del igdrcl.config
vector.l0.skl 2>skl.l0.run
findstr PASSED skl.l0.run >nul 2>nul
IF "%ERRORLEVEL%"=="1" ((echo ERROR: Run failed - see skl.l0.run output) && GOTO :EOF)



cd ..

GOTO :EOF

:MSVC
echo.
@echo ERROR: script must be run from within 'x64 Native Tools Command Prompt for VS 201x'
GOTO :EOF

:SETUP
echo.
@echo ERROR: SDK_IGC environment variable must be set (via setenv.bat)
GOTO :EOF
