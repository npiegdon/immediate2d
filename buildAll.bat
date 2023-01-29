@echo off
setlocal

where /q cl || (
  echo ERROR: "cl" not found.  Please run this from the MSVC x64 native tools command prompt.
  exit /b 1
)

set CFLAGS=/nologo /W3 /EHsc /GS- /Gs999999 /std:c++17 /openmp
set LDFLAGS=/incremental:no /opt:icf /opt:ref /subsystem:windows

for %%f in (example*.cpp) do (
  if exist %%~nf.rc (

    :: If there's an accompanying resource file, compile it, too.
    call rc.exe /nologo %%~nf.rc
    call cl.exe -O2 %CFLAGS% %%~nf.cpp %%~nf.res /link %LDFLAGS%

  ) else (

    :: No resource file, just compile the code.
    call cl.exe -O2 %CFLAGS% %%~nf.cpp /link %LDFLAGS%

  )
)
