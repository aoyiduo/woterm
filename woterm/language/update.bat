echo "init vc env"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
echo "init qt env"
call "C:\Qt\Qt5.12.12\5.12.12\msvc2017\bin\qtenv2.bat"

set path_script=%~dp0
echo %path_script%

cd %path_script%
rem lupdate -recursive %path_script%\..\ -I %path_script%\..\..\kxftp -I %path_script%\..\..\kxvnc -I %path_script%\..\..\kxutil -I %path_script%\..\..\kxterm -ts %path_script%woterm_en.ts
lupdate -recursive %path_script%\..\ %path_script%\..\..\kxftp %path_script%\..\..\kxvnc %path_script%\..\..\kxutil %path_script%\..\..\kxterm -ts %path_script%woterm_zh.ts
pause