# use premake to generate a makefile using clang, you can manually also do
# ./vendor/premake/premake5.exe vs2022

premake5 --cc=clang gmake
premake5 export-compile-commands
cp ./compile_commands/debug.json ./compile_commands.json
