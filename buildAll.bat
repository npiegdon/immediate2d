@echo off
setlocal

where /q cl || (
  echo ERROR: "cl" not found.  Please run this from the MSVC x64 native tools command prompt.
  exit /b 1
)

set CFLAGS=/nologo /W3 /Z7 /EHsc /GS- /Gs999999 /std:c++17 /openmp
set LDFLAGS=/incremental:no /opt:icf /opt:ref

for %%f in (example*.cpp) do (
  call cl -O2 %CFLAGS% %%~nf.cpp /link %LDFLAGS% /subsystem:windows
)
