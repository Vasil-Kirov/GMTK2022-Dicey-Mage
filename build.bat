@ECHO OFF
CLS

REM ********************************************************
REM * Use "build release" to build an optimized executable *
REM ********************************************************


IF [%1] == [release] (
	ECHO -------- RELEASE --------
	SET CompilerFlags=-O2 -W4 -WX -DRELEASE_MODE -GS-
) ELSE (
	SET CompilerFlags=-Zi -Od -W3 -DDEBUG_MODE
)

REM **********************************************
REM * Change the name of the outputed executable *
REM **********************************************
SET Name=main


SET CompilerFlags=%CompilerFlags% -nologo -fp:fast -wd4100 -wd4201 
SET Libraries=kernel32.lib user32.lib Gdi32.lib OpenGL32.lib ..\libs\*.lib
SET Includes=-I ..\includes

PUSHD build
CL ..\code\main.c -Fe:%Name% %CompilerFlags% %Includes% /link %Libraries% /subsystem:windows /ENTRY:mainCRTStartup
POPD


