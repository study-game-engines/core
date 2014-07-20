@ECHO OFF


SET /p PRJ_NAME=Name of the new game project: 

SET PRJ_CUR=%CD%\template
cd ../..
SET PRJ_TARGET=%CD%\%PRJ_NAME%\


SET /p PRJ_CONFIRM=Create folder ^<%PRJ_TARGET%^> [y/n]: 
IF NOT %PRJ_CONFIRM:~0,1% == y EXIT


ECHO Copying...


xcopy "%PRJ_CUR%" "%PRJ_TARGET%" /E /Q


ECHO Renaming...


SET TOOL_REPLACE="%PRJ_CUR%\..\..\tools\replace_text.bat"
SET TOOL_NAME=CoreApp

SET PATH_CODEBLOCKS=%PRJ_TARGET%\projects\codeblocks
SET PATH_NETBEANS=%PRJ_TARGET%\projects\netbeans\nbproject
SET PATH_VISUAL=%PRJ_TARGET%\projects\visualstudio


cd "%PATH_CODEBLOCKS%"
CALL %TOOL_REPLACE% %TOOL_NAME% %PRJ_NAME% %TOOL_NAME%.cbp > %PRJ_NAME%.cbp
CALL DEL %TOOL_NAME%.cbp


cd "%PATH_NETBEANS%"
CALL RENAME configurations.xml configurations_temp.xml
CALL %TOOL_REPLACE% %TOOL_NAME% %PRJ_NAME% configurations_temp.xml > configurations.xml
CALL DEL configurations_temp.xml

CALL RENAME project.xml project_temp.xml
CALL %TOOL_REPLACE% %TOOL_NAME% %PRJ_NAME% project_temp.xml > project.xml
CALL DEL project_temp.xml


cd "%PATH_VISUAL%\%TOOL_NAME%"
CALL RENAME %TOOL_NAME%.filters %PRJ_NAME%.filters
CALL RENAME %TOOL_NAME%.vcxproj %PRJ_NAME%.vcxproj
CALL RENAME %TOOL_NAME%.vcxproj.user %PRJ_NAME%.vcxproj.user

cd "%PATH_VISUAL%"
CALL RENAME %TOOL_NAME% %PRJ_NAME%
CALL %TOOL_REPLACE% %TOOL_NAME% %PRJ_NAME% %TOOL_NAME%.sln > %PRJ_NAME%.sln
CALL DEL %TOOL_NAME%.sln


cd "%PRJ_TARGET%"
CALL RENAME .gitignore .gitignore_temp
CALL %TOOL_REPLACE% %TOOL_NAME% %PRJ_NAME% .gitignore_temp > .gitignore
CALL DEL .gitignore_temp

CALL RENAME .hgignore .hgignore_temp
CALL %TOOL_REPLACE% %TOOL_NAME% %PRJ_NAME% .hgignore_temp > .hgignore
CALL DEL .hgignore_temp

cd "executable"
CALL RENAME %TOOL_NAME%_linux.sh %PRJ_NAME%_linux.sh
CALL RENAME %TOOL_NAME%_windows.exe %PRJ_NAME%_windows.exe


ECHO ^<%PRJ_TARGET%^> created
%SystemRoot%\explorer.exe "%PRJ_TARGET%"

PAUSE