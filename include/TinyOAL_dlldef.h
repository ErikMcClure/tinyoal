// Copyright ©2014 Black Sphere Studios
// This file is part of TinyOAL - An OpenAL Audio engine
// For conditions of distribution and use, see copyright notice in TinyOAL.h

#ifndef __TINYOAL_DLLDEF_H__TOAL__
#define __TINYOAL_DLLDEF_H__TOAL__

#include "bss-util/bss_compiler.h"

#ifdef TINYOAL_STATICLIB
#define TINYOAL_DLLEXPORT 
#else
#ifdef TINYOAL_EXPORTS
#define TINYOAL_DLLEXPORT BSS_COMPILER_DLLEXPORT
#else
#define TINYOAL_DLLEXPORT BSS_COMPILER_DLLIMPORT
#endif
#endif

#define TINYOAL_VERSION_MAJOR 1
#define TINYOAL_VERSION_MINOR 0
#define TINYOAL_VERSION_REVISION 1

#endif
