TinyOAL - An OpenAL Audio engine
----------------------------
Copyright ©2013 Black Sphere Studios

TinyOAL has no dependencies, simply drop the appropriate DLLs into your EXE's directory and link to the *.lib files in /bin. Please see the example source code for a tutorial on how to use the engine. The compiled examples can be previews in /examples/bin/.

Base DLLs:
TinyOAL.dll
soft_oal.dll

Base DLLs x86-64: (These 64-bit DLLs don't have 64-bit OGG dlls to use)
TinyOAL64.dll
soft_oal64.dll

OGG DLLs: (Required only if you are playing OGG files)
ogg.dll
vorbis.dll
vorbisfile.dll

Loop Utility: (Required only for LoopUtility.exe)
tag.dll
zlib1.dll