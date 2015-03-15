@ECHO OFF
md "..\Packages\TinyOAL"

XCOPY "*.sln" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.cpp" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.c" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.h" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.tcc" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.vcxproj" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.filters" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.txt" "..\Packages\TinyOAL" /C /I /R /Y
XCOPY "*.lib" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.dll" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "alsoftrc.sample" "..\Packages\TinyOAL" /S /C /I /Y
XCOPY "*.csproj" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.cs" "..\Packages\TinyOAL" /S /C /I /R /Y
XCOPY "*.config" "..\Packages\TinyOAL" /S /C /I /R /Y

md "..\Packages\TinyOAL\doc"

XCOPY "doc\*.txt" "..\Packages\TinyOAL\doc" /S /C /I /R /Y
XCOPY "examples\bin\*.exe" "..\Packages\TinyOAL\examples\bin" /C /I /R /Y
XCOPY "bin\*.pdb" "..\Packages\TinyOAL\bin" /C /I /R /Y

del "..\Packages\TinyOAL\examples\bin\*.lib"

md "..\Packages\TinyOAL\examples\media"

XCOPY "examples\media\*.wav" "..\Packages\TinyOAL\examples\media" /C /I /R /Y
XCOPY "examples\media\*.ogg" "..\Packages\TinyOAL\examples\media" /C /I /R /Y
XCOPY "examples\media\*.mp3" "..\Packages\TinyOAL\examples\media" /C /I /R /Y
XCOPY "examples\media\*.flac" "..\Packages\TinyOAL\examples\media" /C /I /R /Y
XCOPY "examples\media\*.txt" "..\Packages\TinyOAL\examples\media" /C /I /R /Y