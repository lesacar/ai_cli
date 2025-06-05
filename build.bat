@REM use premake to generate a makefile using clang, you can manually also do
@REM ./vendor/premake/premake5.exe vs2022

.\vendor\premake\premake5.exe --cc=clang gmake
.\vendor\premake\premake5.exe export-compile-commands
copy .\compile_commands\debug.json .\compile_commands.json
