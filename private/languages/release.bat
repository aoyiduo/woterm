echo "init vc env"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
echo "init qt env"
call "C:\Qt\Qt5.12.12\5.12.12\msvc2017\bin\qtenv2.bat"

set path_script=%~dp0
echo %path_script%

cd %path_script%
lrelease %path_script%woterm_en.ts -qm %path_script%woterm_en.qm
lrelease %path_script%woterm_zh.ts -qm %path_script%woterm_zh.qm
pause