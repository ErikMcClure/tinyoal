# TinyOAL Changelog

## 1.1.0
- Visual Studio 2017 update, no longer supports older VS versions due to use of modern C++
- Changed logging behavior to use a replaceable function pointer
- Removed all prefixes and changed private functions to camelCase
- Updated bss::Singleton usage
- Removed most static variables in favor of just using TinyOAL singleton

## 1.0.4
- Split filetype out from flags
- Codecs can now be dynamically registered at runtime
- Added M4A codec example (windows only)
- Fixed error when destroying the engine while audio samples were still playing
- Standardized integer types to C++11
- Changed namespace to be all lowercase

## 1.0.3
- Updated bss-util headers
- Fixed divide-by-zero errors when loading incompatible wav files
- Changed OGG layer to look for libvorbisfile and libvorbisfile64 and added 64-bit DLLs for ogg, vorbis, and vorbisfile

## 1.0.2
- Updated bss-util headers
- Re-organized intermediate output folders

## 1.0.1
- Updated utility library headers
- cAudioResource::ToSample is now ToSamples, added a ToSeconds counterpart.
- cAudioResource now has a GetTotalSamples and a GetLength for figuring out the total length of a resource
- Fixed some settings in the managed project

## 1.0.0
- Completely rewrote the engine's internals; only changes to the external interface will be listed.
- cAudioResource::Create is now used to load resources, and PlaySound is just a wrapper around cAudioResource::Play()
- TinyOAL.h was removed; just include cTinyOAL.h now
- cAudio can only be constructed from a cAudioResource now.
- Skipping now works properly while the sound is playing
- Sources are now properly requested and released on Play() and Stop()
- TINYOAL_LOADINTOMEMORY is now TINYOAL_COPYINTOMEMORY
- TINYOAL_AUTOPLAY has been replaced by simply specifying TINYOAL_ISPLAYING upon construction
- TINYOAL_NODOUBLEPLAY has been removed. Play() will never restart the stream because it's inconsistent with Pause()
- cAudioResource now stores a loop point that is copied to all cAudio's upon construction.
- cAudioResource also has a loop point argument in its Create() functions, but this will be overriden if the file being loaded has a loop point.
- You can now manually specify the file format by passing in a file format parameter into the cAudioResource flags.
- You can set a cAudioResource to have a maximum number of instances. Attempting to play more than _maxactive instances of a sound will result in the one that's been playing the longest to be destroyed and replaced with the new instance.
- cAudioResource::ToSample is no longer virtual and takes only a double argument now.
- Added MP3 file support (cTinyOAL supports mpg123's seamless looping, but it doesn't always work. See 04Mp3File for details)
- Added FLAC file support (can't do seamless looping despite being lossless because the codec is bullshit)
- Added TINYOAL_FORCETOWAVE
- You can now override all dynamic DLL paths when creating the engine
- You can now change the target audio device being used by openAL
- TinyOAL_net is now stable and supports all new features
- Added 04Mp3File (a working version, anyway), 05FlacFile, and 06Managed examples, plus a 00Testbed for stress-testing purposes.
- The engine now includes an example of an openAL config file (alsoftrc.sample) and a function that will copy any config file you specify to the magical location that openAL reads it's config files from, depending on platform.

## 0.9.92
- Threw out useless errlog.h header
- Standardized logging and put useful log messages everywhere
- Utilizes bss_compiler.h
- TinyOAL.rc included for windows version info
- Replaced proprietary openAL with open-source softOAL implementation
- Threw out stupid linked list implementation
- Renamed ManagedLoad to PlaySound
- Updated examples
- Renamed SetLoopPoint to SetLoopPointSeconds and SetLoopPointSamples to SetLoopPoint
- Now compiles to 64-bit using the 64-bit softOAL library. Unfortunately, we don't have a matching 64-bit ogg.dll so only 01 WavFile works in 64-bit.

## 0.9.91
- Updated to use bss-util v0.3.82
- Removed lame attempt at using LAME
- All MP3 components must be forced on by using __INCLUDE_MP3. This is, of course, useless, becuase they don't work right now.
- Evil #pragma once usage replaced by include gaurds

## 0.9.9
- Fixed invalid resource deletion
- Fixed incorrect Play() source
- Renamed AUDIO_flags to TINYOAL_flags
- Removed Permaloads
- Introduced cAudioRefs for reference management as a replacement to permaloading
- Introduced a ManagedCreate function that allows you to create an audio source and have it be automatically deleted after it finishes playing
- Various utility updates

## 0.9.85
- Added changelog
- Added version number to source
- Added several OGG function calls
- Implemented a faster Reset for OGG using ov_seek_raw()
- Added time to sample conversion for all formats
- Implemented skipping
- Implemented loop points
- OGG format now looks for LOOPSTART and automatically sets the loop point to that on load
- Added looping utility to insert LOOPSTART into an OGG file and added its associated dependencies

## 0.9.8
- Minor bugfixees

## 0.9.5
- Initial Public Release