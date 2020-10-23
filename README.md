TinyOAL is a minimalist audio engine using openAL-soft. However, you can also force it to use the normal openAL dll if you really want to.

## Building
TinyOAL uses a submodule to include the bss-util include header files, so be sure to run `git submodule update --init` after cloning the repository. The OpenAL headers, mpg123 headers, libFLAC headers, and vorbis OGG headers must all be installed on your system before you attempt to build.

### Windows
On windows, use [vcpkg](https://github.com/microsoft/vcpkg) and install `openal-soft`, `mpg123`, `libflac` and `libogg` (you'll probably want the `:x64-windows` versions). Integrate vcpkg with Visual Studio with `vcpkg integrate install` if you haven't already, and the solution file should build.

### Linux
You must install openAL or openAL-soft in your package manager, along with `mpg123`, `libflac` and `libogg`. Then simply run `make` in the root project directory to build. It is your responsibility to make sure the compiler can find those include files - if they are in a non-standard directory you may have to modify the makefile so it can find them.

## Features
* volume, pitch, and panning
* loop points
* resource management
* maximum instance counts
* WAV, OGG, MP3, and FLAC decoder support
* file-based streaming or in-memory streaming
* conversion to WAV
* filetype detection
* .net wrapper for managed languages

Resources are loaded using cAudioResource, which is reference counted. These resources are then passed into a cAudio instance that plays them. Provided your resource isn't a streaming file, it can be played multiple times simultaneously. cAudioResource can also spawn managed cAudio instances that automatically start playing and destroy themselves once they've stopped playing, which is useful when you simply need to trigger a sound effect and don't need to micro-manage it.

Note that cAudioResource::Create() grabs a reference to the resource, as does assigning that resource to a cAudio. This means a cAudioResource will always have exactly 1 reference count left over after all instances of that resource have been deleted. This is done on purpose, because 99% of the time you want to keep those resources in memory as long as possible, but if you need to get rid of them, just remember to add a Drop() call after all instances have been destroyed.

Everything uses memory pools to make creating instances as efficient as possible, but consider using FORCETOWAVE for heavily used sound effects that aren't stored as WAV files. Wave formats are highly optimized and are virtually free, both in terms of creating new cAudio instances, and in terms of playing them.

TinyOAL supports all common Ogg vorbis formats, MP3 formats, fixed block size FLAC formats, and the following WAVE formats:

* PCM unsigned 8 bit mono, stereo, quad, rear, 5.1 surround, 6.1 surround, 7.1 surround
* PCM signed 16 bit mono, stereo, quad, rear, 5.1 surround, 6.1 surround, 7.1 surround
* PCM signed 24 bit mono, stereo
* PCM signed 32 bit mono, stereo, quad, rear, 5.1 surround, 6.1 surround, 7.1 surround
* IEEE float 32-bit mono, stereo
* IEEE float 64-bit mono, stereo
* mu-law mono, stereo, quad, 5.1 surround, 6.1 surround, 7.1 surround
* A-law mono, stereo

openAL-soft implements all of these extensions, but traditional OpenAL guarantees only PCM unsigned 8-bit mono/stereo and PCM signed 16 bit mono/stereo. 24-bit and 32-bit integer formats are promoted to floating point, so they require the 32-bit float extension to work.

TinyOAL has no dependencies, simply drop the appropriate DLLs into your EXE's directory and link to the .lib files in /bin. Note, however, that if you force TinyOAL to use the traditional openAL dll, clients may need to install it. You must also provide any DLLs necessary to decode the formats you will be working with (OGG, MP3, or FLAC). Example projects demonstrating basic concepts are found in TinyOAL/examples/, which compile into TinyOAL/examples/bin/.

TinyOAL_net is a Managed C++ wrapper around the TinyOAL dll. Simply add a reference to the TinyOAL_net.dll in your VB.net/C#/F# project and make sure that the corresponding TinyOAL.dll is placed in the same folder as TinyOAL_net.dll. Note that if you choose to use the TinyOAL_net64.dll, you'll need the TinyOAL64.dll, and the same applies to the debug versions.

### Known Issues
* libFLAC.dll is terribly designed, and is thus extremely bad at streaming. If you are doing anything other than playing a large FLAC file without seamless looping, I strongly suggest using FORCETOWAVE.
* The 32-bit integer to floating point does not attempt to do a good job of preserving precision. Since the audio driver usually only has 16 bits of precision, this shouldn't matter, but if you care about it, use a wav file saved as a 32-bit float instead.
* TinyOAL attempts to set up mpg123's seamless looping abilities, but it requires specially crafted MP3 files. You should use OGG files if you need seamless looping.
